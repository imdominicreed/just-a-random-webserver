#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

#include "http/request.h"
#include "socket_handler.h"

namespace domino {
namespace http {
class Server {
 public:
  Server(int port) : socket_handler(handler::SocketHandler(port)) {}

  void Start() {
    socket_handler.Initialize();
    while (true) {
      printf("\n+++++++ Waiting for new connection ++++++++\n\n");
      int request_fd = socket_handler.ListenForRequest();
      Request request =
          socket_handler.ParseRequestFromSocketDescriptor(request_fd);
      printf("------------------Hello message sent-------------------\n");
      socket_handler.closeSocket(request_fd);
    }
  }

 private:
  handler::SocketHandler socket_handler;
};
}  // namespace http
}  // namespace domino

#endif  // HTTP_SERVER_H_
