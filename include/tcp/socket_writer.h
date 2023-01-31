#ifndef SOCKET_WRITER_H_
#define SOCKET_WRITER_H_

#include <netinet/in.h>
#include <unistd.h>

#include <string>

namespace domino {
namespace tcp {
class SocketWriter {
 public:
  SocketWriter(int socket_fd) : socket_fd(socket_fd) {}

  bool Close() {
    // for more information on close()'s return values
    // see https://linux.die.net/man/3/close
    return close(socket_fd) == 0;
  }

  void WriteBuffer(const char *buffer, size_t n_bytes) {
    if ((send(socket_fd, buffer, n_bytes, 0)) == -1) {
      throw std::runtime_error("Error Sending Bytes!");
    }
  }

 private:
  int socket_fd;
};
}  // namespace tcp
}  // namespace domino

#endif  // SOCKET_WRITER_H_
