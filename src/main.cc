#include <csignal>
#include <iostream>

#include "http/server.h"

constexpr int kPort = 8080;

void signal_handler(int signal_number) {
  std::cout << "Interrupt signal (" << signal_number << ") received.\n";
  exit(signal_number);
}

int main() {
  signal(SIGINT, signal_handler);

  domino::http::Server server(kPort);
  server.Start();

  return 0;
}
