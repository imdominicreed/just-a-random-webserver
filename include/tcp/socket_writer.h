#ifndef SOCKET_WRITER_H_
#define SOCKET_WRITER_H_

#include <netinet/in.h>
#include <unistd.h>

#include <string>

#include "http/response.h"

namespace domino {
namespace tcp {
class SocketWriter {
 public:
  SocketWriter(int socket_fd) : socket_fd(socket_fd) {}

  // ideally make the below const ref
  void SendHttpResponse(const http::Response &response) {
    std::string body = response.ToBuffer();
    WriteBuffer(body.c_str(), strlen(body.c_str()));
  }

  bool Close() {
    // for more information on close()'s return values
    // see https://linux.die.net/man/3/close
    return close(socket_fd) == 0;
  }

 private:
  void WriteBuffer(const char *buffer, size_t n_bytes) {
    if ((send(socket_fd, buffer, n_bytes, 0)) == -1) {
      throw std::runtime_error("Error Sending Bytes!");
    }
  }
  int socket_fd;
};
}  // namespace tcp
}  // namespace domino

#endif  // SOCKET_WRITER_H_
