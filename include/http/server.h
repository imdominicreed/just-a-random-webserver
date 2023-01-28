#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

#include <iostream>
#include <unordered_map>

#include "http/request.h"
#include "tcp/socket_reader.h"
#include "tcp/socket_writer.h"

namespace domino {
namespace http {

typedef std::function<void(Request, Response)> EndpointHandler;
typedef std::unordered_map<std::string, EndpointHandler> MethodToEndpointMap;

class Server {
 public:
  Server(int port) : socket_reader(tcp::SocketReader(port)) {}

  void Register(Method method, std::string endpoint, EndpointHandler handler) {
    endpoint_map[method][endpoint] = handler;
  }

  void Start() {
    socket_reader.Initialize();
    while (true) {
      printf("\n+++++++ Waiting for new connection ++++++++\n\n");
      int request_fd = socket_reader.ListenForRequest();
      Request request =
          socket_reader.ParseRequestFromSocketDescriptor(request_fd);
      tcp::SocketWriter socket_writer(request_fd);
      Response response;

      // check if the HTTP method we recieved has any endpoints available
      // ense return 404
      std::unordered_map<
          Method, std::unordered_map<std::string, EndpointHandler>>::iterator
          method_to_endpoint_map = endpoint_map.find(request.method);
      if (method_to_endpoint_map == endpoint_map.end()) {
        response.SetStatus(Status::kNotFound);
        socket_writer.SendHttpResponse(response);
        socket_writer.Close();
        continue;
      }

      // check if the endpoint we recieved for a given HTTP method
      // has any handlers available, else return 404
      std::unordered_map<std::string, EndpointHandler>::iterator
          endpoint_handler =
              method_to_endpoint_map->second.find(request.endpoint);
      if (endpoint_handler == method_to_endpoint_map->second.end()) {
        response.SetStatus(Status::kNotFound);
        socket_writer.SendHttpResponse(response);
        socket_writer.Close();
        continue;
      }

      printf("------------------Hello message sent-------------------\n");
      socket_writer.Close();
    }
  }

 private:
  tcp::SocketReader socket_reader;
  std::unordered_map<Method, std::unordered_map<std::string, EndpointHandler>>
      endpoint_map;
};
}  // namespace http
}  // namespace domino

#endif  // HTTP_SERVER_H_
