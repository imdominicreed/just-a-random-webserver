#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <unordered_map>

#include "db_handler.h"
#include "http/request.h"
#include "socket_handler.h"

constexpr int kPort = 8080;
constexpr int kBacklogMax = 3;

int create_socket() {
  int server_fd;
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("cannot create socket_fd");
    return -1;  // TODO: Better return code.
  }
  return server_fd;
}

int main() {
  int socket_fd = create_socket();
  domino::sqlite::Database db_handler;

  struct sockaddr_in address = {0};
  // memset((char *)&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(kPort);

  if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("cannot bind socket_fd");
    return -1;  // TODO: Better return code.
  }

  if (listen(socket_fd, kBacklogMax) < 0) {
    perror("cannot listen to request");
    return -1;
  }
  domino::handler::SocketHandler socket_handler;
  while (true) {
    printf("\n+++++++ Waiting for new connection ++++++++\n\n");
    socket_handler.waitForRequestSocket(socket_fd, address, sizeof(address));
    domino::http::Request request = socket_handler.parseSocketRequest();
    printf("------------------Hello message sent-------------------\n");
    socket_handler.closeSocket();
  }

  return 0;
}
