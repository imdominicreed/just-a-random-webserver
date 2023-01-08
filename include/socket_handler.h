#ifndef SOCKET_HANDLER_H_
#define SOCKET_HANDLER_H_

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <exception>
#include <unordered_map>

#include "file_handler.h"
#include "http/request.h"
#include "http/response.h"

namespace domino {
namespace handler {

constexpr char kFormatHeader[] =
    "HTTP/1.1 200 OK\r\nContent-Length: %llu\r\nContent-Type: %s\r\n\r\n";

constexpr int kBufferSize = 8192;  // Headers cannot exceed 8KB if so then bug

constexpr char kOkResponse[] = "HTTP/1.1 200 OK\r\n\r\n";

class SocketHandler {
 private:
  int socket_fd;

  std::string readBinaryData(unsigned long long content_size,
                             std::string in_buffer) {
    // Creates New File for Post Data
    FileHandler file_handler;
    std::string file_name = file_handler.createFile();
    file_handler.selectFile(file_name);

    // Any remaining data in previous buffer gets written out;
    size_t index = 0;
    substringToken(index, in_buffer, "\r\n\r\n");
    unsigned long long sum = in_buffer.size() - index;
    in_buffer = in_buffer.substr(index);
    file_handler.writeFileBuffer((char *)in_buffer.c_str(), sum);

    // Writes file from socket using buffer
    char buffer[kBufferSize];
    while (sum < content_size) {
      size_t data_read = readBuffer(buffer, kBufferSize);
      file_handler.writeFileBuffer(buffer, data_read);
      sum += data_read;
    }

    // Response OK
    respondOK();

    return std::string(file_name);
  }

  std::string substringToken(size_t &http_index, std::string &http_string,
                             std::string delimiter) {
    size_t end = http_string.find(delimiter, http_index);
    std::string ret = http_string.substr(http_index, end - http_index);
    http_index = end + delimiter.length();
    return ret;
  }

 public:
  int waitForRequestSocket(int listen_socket, struct sockaddr_in address,
                           socklen_t addr_len) {
    if ((socket_fd = accept(listen_socket, (struct sockaddr *)&address,
                            (socklen_t *)&addr_len)) < 0) {
      throw std::runtime_error("Failed Listening to socket!");
    }
    return socket_fd;
  }

  void sendHeader(unsigned long long content_length, std::string content_type) {
    char header[1024] = {0};
    sprintf(header, kFormatHeader, content_length, content_type.c_str());
    writeBuffer(header, strlen(header));
  }

  void sendFile(std::string file_path, std::string content_type) {
    FileHandler handler;
    handler.selectFile(file_path);
    sendHeader(handler.getFileSize(), content_type);
    char buffer[kBufferSize];
    while (!handler.isEof()) {
      size_t bytes_read = handler.readFileBuffer(buffer, kBufferSize);
      writeBuffer(buffer, bytes_read);
    }
    handler.close();
  }

  void respondOK() {
    http::Response response;
    response.SetStatus(http::Status::kOk);
    std::string body = response.ToBuffer();
    writeBuffer(body.c_str(), strlen(body.c_str()));
  }

  void writeBuffer(const char *buffer, size_t n_bytes) {
    if ((send(socket_fd, buffer, n_bytes, 0)) == -1) {
      throw std::runtime_error("Error Sending Bytes!");
    }
  }
  size_t readBuffer(char *buffer, size_t n_bytes) {
    return read(socket_fd, buffer, kBufferSize);
  }

  void closeSocket() { close(socket_fd); }

  domino::http::Request parseSocketRequest() {
    char buffer[kBufferSize];
    size_t amount_read = readBuffer(buffer, kBufferSize);
    std::string http_string(buffer, amount_read);
    domino::http::Request request(http_string);
    request.Initialize();

    if (request.method == http::Method::kUnsupported) {
      throw std::invalid_argument(
          "Invalid HTTP Request: Only support GET and POST!");
    }
    this->respondOK();
    return request;
  }
};
}  // namespace handler
}  // namespace domino

#endif  // SOCKET_HANDLER_H_
