#ifndef SOCKET_READER_H_
#define SOCKET_READER_H_

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
namespace tcp {

constexpr char kFormatHeader[] =
    "HTTP/1.1 200 OK\r\nContent-Length: %llu\r\nContent-Type: %s\r\n\r\n";

constexpr int kBufferSize = 8192;  // Headers cannot exceed 8KB if so then bug

constexpr char kOkResponse[] = "HTTP/1.1 200 OK\r\n\r\n";

constexpr int kEnable = 1;

constexpr int kMaximumPendingConnections = 3;

class SocketReader {
 public:
  SocketReader(int port) : port(port) {}

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

  size_t ParseHttpHeader(int socket_fd, char *buffer) {
    return read(socket_fd, buffer, kBufferSize);
  }

  domino::http::Request ParseRequestFromSocketDescriptor(int socket_fd) {
    char buffer[kBufferSize];
    size_t amount_read = ParseHttpHeader(socket_fd, buffer);
    std::string http_string(buffer, amount_read);
    domino::http::Request request(http_string);
    request.Initialize();

    if (request.method == http::Method::kUnsupported) {
      throw std::invalid_argument(
          "Invalid HTTP Request: Only support GET and POST!");
    }
    return request;
  }
};
}  // namespace tcp
}  // namespace domino

#endif  // SOCKET_READER_H_
