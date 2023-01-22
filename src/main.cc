#include <stdlib.h>

#include <csignal>
#include <iostream>
#include <string>

#include "http/server.h"

constexpr int kPort = 8080;

void signal_handler(int signal_number) {
  std::cout << "Interrupt signal (" << signal_number << ") received.\n";
  exit(signal_number);
}

// ideal usage: /square?value=23
// any invalid characters will have the below method return 0.
// if the value sent is separated by non numerical characters, the
// first segment will be squared until a non numerical character is found
std::function<void(domino::http::Request, domino::http::Response)>
    SquareHandler = [](const domino::http::Request& request,
                       domino::http::Response response) {
      std::unordered_map<std::string, std::string>::const_iterator maybe_value =
          request.query.find("value");
      if (maybe_value == request.query.end()) {
        response.SetStatus(domino::http::Status::kBadRequest);
        response.SetBody(
            "a value must be specified in the query string to square");
        response.Send();
      } else {
        int value_as_int = strtol(maybe_value->second.c_str(), NULL, 10);
        response.SetBody(std::to_string(value_as_int * value_as_int));
        response.Send();
      }
    };

int main() {
  signal(SIGINT, signal_handler);

  domino::http::Server server(kPort);
  server.Register(domino::http::Method::kPost, "/square", SquareHandler);
  server.Start();

  return 0;
}
