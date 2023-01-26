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

constexpr int kEnable = 1;

constexpr int kMaximumPendingConnections = 3;

class SocketHandler {
 public:
  SocketHandler(int port) : port(port) {}

  void Initialize() {
    this->address = {0};

    this->address.sin_family = AF_INET;
    // we default sin_addr.s_addr to INADDR_ANY as we don't forsee the
    // need to specify a network interface to add that logic right now
    this->address.sin_addr.s_addr = htonl(INADDR_ANY);
    this->address.sin_port = htons(this->port);

    if ((this->socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("cannot create socket file descriptor");
      return;  // TODO: Better return code.
    }

    // force the OS to bind to the port even if it is in use, for more info see
    // https://stackoverflow.com/a/24208409
    // https://stackoverflow.com/a/24194999
    if (setsockopt(this->socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR,
                   &kEnable, sizeof(int)) < 0) {
      perror("setsockopt(SO_REUSEADDR) failed");
      return;
    }

    if (bind(this->socket_file_descriptor, (struct sockaddr *)&this->address,
             sizeof(this->address)) < 0) {
      perror("unable to bind to socket file descriptor");
    }

    if (listen(this->socket_file_descriptor, kMaximumPendingConnections) < 0) {
      perror("cannot listen to request");
      return;
    }
  }

 private:
  int port;
  struct sockaddr_in address;
  int socket_file_descriptor;

  std::string readBinaryData(int socket_fd, unsigned long long content_size,
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
      size_t data_read = readBuffer(socket_fd, buffer, kBufferSize);
      file_handler.writeFileBuffer(buffer, data_read);
      sum += data_read;
    }

    // Response OK
    respondOK(socket_fd);

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
  int ListenForRequest() {
    int addr_len = sizeof(address);
    int socket_fd = 0;
    if ((socket_fd = accept(socket_file_descriptor, (struct sockaddr *)&address,
                            (socklen_t *)&addr_len)) < 0) {
      throw std::runtime_error("Failed Listening to socket!");
    }
    return socket_fd;
  }

  void sendHeader(int socket_fd, unsigned long long content_length,
                  std::string content_type) {
    char header[1024] = {0};
    sprintf(header, kFormatHeader, content_length, content_type.c_str());
    writeBuffer(socket_fd, header, strlen(header));
  }

  void sendFile(int socket_fd, std::string file_path,
                std::string content_type) {
    FileHandler handler;
    handler.selectFile(file_path);
    sendHeader(socket_fd, handler.getFileSize(), content_type);
    char buffer[kBufferSize];
    while (!handler.isEof()) {
      size_t bytes_read = handler.readFileBuffer(buffer, kBufferSize);
      writeBuffer(socket_fd, buffer, bytes_read);
    }
    handler.close();
  }

  void respondOK(int socket_fd) {
    http::Response response;
    response.SetStatus(http::Status::kOk);
    std::string body = response.ToBuffer();
    writeBuffer(socket_fd, body.c_str(), strlen(body.c_str()));
  }

  void writeBuffer(int socket_fd, const char *buffer, size_t n_bytes) {
    if ((send(socket_fd, buffer, n_bytes, 0)) == -1) {
      throw std::runtime_error("Error Sending Bytes!");
    }
  }
  size_t readBuffer(int socket_fd, char *buffer, size_t n_bytes) {
    return read(socket_fd, buffer, kBufferSize);
  }

  void closeSocket(int socket_fd) { close(socket_fd); }

  domino::http::Request ParseRequestFromSocketDescriptor(int socket_fd) {
    char buffer[kBufferSize];
    size_t amount_read = readBuffer(socket_fd, buffer, kBufferSize);
    std::string http_string(buffer, amount_read);
    domino::http::Request request(http_string);
    request.Initialize();

    if (request.method == http::Method::kUnsupported) {
      throw std::invalid_argument(
          "Invalid HTTP Request: Only support GET and POST!");
    }
    this->respondOK(socket_fd);
    return request;
  }
};
}  // namespace handler
}  // namespace domino

#endif  // SOCKET_HANDLER_H_
