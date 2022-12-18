#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

#include "http/request.h"
#include "socket_handler.h"

namespace domino {
namespace http {
constexpr int kEnable = 1;
constexpr int kMaximumPendingConnections = 3;
class Server {
 public:
  Server(int port) : port(port) {}

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
  }

  void Start() {
    if (listen(this->socket_file_descriptor, kMaximumPendingConnections) < 0) {
      perror("cannot listen to request");
      return;
    }
    domino::handler::SocketHandler socket_handler;
    while (true) {
      printf("\n+++++++ Waiting for new connection ++++++++\n\n");
      socket_handler.waitForRequestSocket(this->socket_file_descriptor, address,
                                          sizeof(address));
      Request request = socket_handler.parseSocketRequest();
      printf("------------------Hello message sent-------------------\n");
      socket_handler.closeSocket();
    }
  }

 private:
  int port;
  int socket_file_descriptor;
  struct sockaddr_in address;
};
}  // namespace http
}  // namespace domino

#endif  // HTTP_SERVER_H_
