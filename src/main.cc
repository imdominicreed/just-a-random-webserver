#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

constexpr int kBufferSize = 1024;
constexpr int kPort = 8083;
constexpr int kBacklogMax = 3;
int socket_fd;
constexpr char kExampleFile[] =
    "/Users/domino/Documents/Projects/E3/goodimage.jpeg";
constexpr char kFormatHeader[] =
    "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n";

int create_socket() {
  int server_fd;
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("cannot create socket_fd");
    return -1;  // TODO: Better return code.
  }
  return server_fd;
}

void send_file(int socket) {
  FILE *fp = fopen(kExampleFile, "r");
  char file_data[kBufferSize] = {0};
  struct stat st;
  fstat(fileno(fp), &st);
  int total_size = st.st_size;
  printf("total:%d\n", total_size);
  char header[1024] = {0};
  sprintf(header, kFormatHeader, total_size, "image/jpeg");
  send(socket, header, strlen(header), 0);
  int send_bytes;
  int sum = 0;
  while ((send_bytes = fread(file_data, 1, sizeof(file_data), fp)) > 0) {
    int sent;
    if ((sent = send(socket, file_data, send_bytes, 0)) == -1) {
      perror("[-]Error in sending file.\n");
      exit(1);
    }
    printf("entry:\n%s", file_data);
    sum += send_bytes;
    bzero(file_data, kBufferSize);
  }
  printf("sum:%d\n", sum);
}

int main() {
  socket_fd = create_socket();

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

  // char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length:
  // 12\n\nHello world!";
  while (true) {
    printf("\n+++++++ Waiting for new connection ++++++++\n\n");
    int new_socket;
    int addr_len = sizeof(address);
    if ((new_socket = accept(socket_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addr_len)) < 0) {
      perror("failed to accept");
      exit(EXIT_FAILURE);
    }
    char buffer[30000] = {0};
    read(new_socket, buffer, 30000);
    printf("%s\n", buffer);
    send_file(new_socket);
    printf("------------------Hello message sent-------------------\n");
    close(new_socket);
  }

  return 0;
}