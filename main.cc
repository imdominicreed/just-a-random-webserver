#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <unordered_map>

#include "http.h"
using namespace domino;
constexpr int kBufferSize = 1024;
constexpr int kPort = 8080;
constexpr int kBacklogMax = 3;
int socket_fd;
constexpr char kExampleFile[] =
    "/Users/domino/Documents/Projects/E3/goodimage.jpeg";
constexpr char kFormatHeader[] =
    "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n";

constexpr char kFileFormat[] = "data_%llu.data";

constexpr char kResponse[] = "HTTP/1.1 201 Created\r\n\r\n";

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

unsigned long long get_epochs_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned long long millisecondsSinceEpoch =
      (unsigned long long)(tv.tv_sec) * 1000 +
      (unsigned long long)(tv.tv_usec) / 1000;
  return millisecondsSinceEpoch;
}

void parse_args(std::string &endpoint,
                std::unordered_map<std::string, std::string> &args) {
  size_t start = endpoint.find("?");
  if (start == std::string::npos) {
    return;
  }
  size_t index = start + 1;
  size_t next_index;
  while ((next_index = endpoint.find("&", index)) != std::string::npos) {
    std::string arg = endpoint.substr(index, next_index - index);
    int equal_index = arg.find("=");
    std::string key = arg.substr(0, equal_index);
    std::string value = arg.substr(equal_index + 1);
    args[key] = value;
    index = next_index + 1;
  }
  std::string arg = endpoint.substr(index);
  int equal_index = arg.find("=");
  std::string key = arg.substr(0, equal_index);
  std::string value = arg.substr(equal_index + 1);
  args[key] = value;
  endpoint.erase(start);
}

std::string substring_to_token(size_t &http_index, std::string &http_string,
                               std::string delimiter, int request_socket) {
  size_t end = http_string.find(delimiter, http_index);

  if (end == std::string::npos) {
    char buffer[kBufferSize] = {0};
    int amount_read = read(request_socket, buffer, kBufferSize);
    std::string tmp(buffer, amount_read);
    std::string begin = http_string.substr(http_index);
    http_string = tmp;
    http_index = 0;
    return begin +
           substring_to_token(http_index, tmp, delimiter, request_socket);
  }
  std::string ret = http_string.substr(http_index, end - http_index);
  http_index = end + delimiter.length();
  return ret;
}

std::string read_post_data(int request_socket,
                           unsigned long long content_size) {
  send(request_socket, kResponse, strlen(kResponse), 0);
  char buffer[kBufferSize] = {0};
  unsigned long long sum = 0;
  char file_name[50] = {};
  sprintf(file_name, kFileFormat, get_epochs_ms());
  FILE *f = fopen(file_name, "w");
  while (sum < content_size) {
    sum += read(request_socket, buffer, kBufferSize - 1);
    fputs(buffer, f);
    bzero(buffer, kBufferSize);
  }
  fflush(f);
  send(request_socket, "HTTP/1.1 200 OK\r\n\r\n", 25, 0);
  return std::string(file_name);
}

http::http_request parse_socket_request(int request_socket) {
  std::unordered_map<std::string, std::string> args;
  char buffer[kBufferSize] = {0};
  int amount_read = read(request_socket, buffer, kBufferSize);
  std::string http_string(buffer, amount_read);
  printf("%s\n", http_string.c_str());
  size_t http_index = 0;
  std::string method =
      substring_to_token(http_index, http_string, " ", request_socket);
  if (method != "GET" && method != "POST") {
    throw std::invalid_argument(
        "Invalid HTTP Request: Only support GET and POST!");
  }
  std::string endpoint_and_args =
      substring_to_token(http_index, http_string, " ", request_socket);
  parse_args(endpoint_and_args, args);
  http::http_request request{};
  request.endpoint = endpoint_and_args;
  request.args = args;
  if (method == "GET") {
    request.method = http::kGet;
    return request;
  }
  http_index = 0;
  substring_to_token(http_index, http_string, "Content-Type: ", request_socket);
  request.args["content_type"] =
      substring_to_token(http_index, http_string, "\n", request_socket);
  http_index = 0;
  substring_to_token(http_index, http_string,
                     "Content-Length: ", request_socket);

  std::string str_size =
      substring_to_token(http_index, http_string, "\n", request_socket);

  unsigned long long size = strtoul(str_size.c_str(), NULL, 10);
  request.method = http::kPost;
  request.args["file_name"] = read_post_data(request_socket, size);
  return request;
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
    parse_socket_request(new_socket);
    printf("------------------Hello message sent-------------------\n");
    close(new_socket);
  }

  return 0;
}
