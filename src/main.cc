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

constexpr int kEnable = 1;
constexpr int kPort = 8080;
constexpr int kBacklogMax = 3;

int create_socket() {
  int server_fd;
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("cannot create socket_fd");
    return -1;  // TODO: Better return code.
  }

  // force the OS to bind to the port even if it is in use, for more info see
  // https://stackoverflow.com/a/24208409
  // https://stackoverflow.com/a/24194999
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &kEnable, sizeof(int)) <
      0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    return -1;
  }
  return server_fd;
}

void signal_handler(int signal_number) {
  std::cout << "Interrupt signal (" << signal_number << ") received.\n";
  exit(signal_number);
}

int main() {
  signal(SIGINT, signal_handler);

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
