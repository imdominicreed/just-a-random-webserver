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

  void parseArgs(std::string &endpoint,
                 std::unordered_map<std::string, std::string> &args) {
    // Find Start of Args
    size_t start = endpoint.find("?");
    // If npos then no args to read
    if (start == std::string::npos) {
      return;
    }

    // Parses indivdual args
    size_t index = start;
    size_t next_index;
    while (index != std::string::npos) {
      index++;
      next_index = endpoint.find("&", index);
      std::string arg = endpoint.substr(index, next_index - index);
      int equal_index = arg.find("=");
      std::string key = arg.substr(0, equal_index);

      std::string value;
      // an index of -1 causes to substr to return the entire string
      // to avoid this, we assign the value to an empty string
      if (equal_index == -1) {
        value = "";
      } else {
        value = arg.substr(equal_index + 1);
      }
      printf("key:%s value: %s\n", key.c_str(), value.c_str());
      args[key] = value;
      index = next_index;
    }

    // Erases Args from endpoint
    endpoint.erase(start);
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

  void respondOK() { writeBuffer((char *)kOkResponse, strlen(kOkResponse)); }

  void writeBuffer(char *buffer, size_t n_bytes) {
    if ((send(socket_fd, buffer, n_bytes, 0)) == -1) {
      throw std::runtime_error("Error Sending Bytes!");
    }
  }
  size_t readBuffer(char *buffer, size_t n_bytes) {
    return read(socket_fd, buffer, kBufferSize);
  }

  void closeSocket() { close(socket_fd); }

  domino::http::Request parseSocketRequest() {
    domino::http::Request request{};

    char buffer[kBufferSize];
    size_t amount_read = readBuffer(buffer, kBufferSize);
    std::string http_string(buffer, amount_read);

    size_t http_index = 0;
    std::string method = substringToken(http_index, http_string, " ");
    if (method != "GET" && method != "POST") {
      throw std::invalid_argument(
          "Invalid HTTP Request: Only support GET and POST!");
    }

    std::string endpoint_and_args =
        substringToken(http_index, http_string, " ");
    std::unordered_map<std::string, std::string> args;
    parseArgs(endpoint_and_args, args);
    request.endpoint = endpoint_and_args;
    request.args = args;

    if (method == "GET") {
      request.method = http::kGet;
      this->respondOK();
      return request;
    }
    request.method = http::kPost;

    substringToken(
        http_index, http_string,
        "Content-Type: ");  // Technically headers are case-insenstive but
                            // practice is that they are supposed to be
                            // capatilized can be fixed later!
    request.args["content_type"] =
        substringToken(http_index, http_string, "\n");

    http_index = 0;
    substringToken(http_index, http_string, "Content-Length: ");
    std::string str_size = substringToken(http_index, http_string, "\n");
    unsigned long long size = strtoul(str_size.c_str(), NULL, 10);
    request.args["file_name"] = readBinaryData(size, http_string);

    return request;
  }
};
}  // namespace handler
}  // namespace domino

#endif  // SOCKET_HANDLER_H_
