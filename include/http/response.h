#ifndef HTTP_RESPONSE_H_
#define HTTP_RESPONSE_H_

#include <sstream>
#include <string_view>

#include "http/status.h"
#include "tcp/socket_writer.h"

namespace domino {
namespace http {
class Response {
 public:
  Response(int socket_fd)
      : status_(Status::kOk), _socket_writer(tcp::SocketWriter(socket_fd)) {}

  void SetStatus(Status status) { status_ = status; }

  /**
   * Write a given HTTP status' response to the socket. Closes said socket
   * after writing.
   */
  void SendStaticResponse(Status status) {
    SetStatus(status);
    const char* body = this->ToBuffer().c_str();
    _socket_writer.WriteBuffer(body, strlen(body));
    _socket_writer.Close();
  };

  std::string ToBuffer() const {
    std::stringstream ss;

    std::streambuf* copy = body_.rdbuf();
    ss << "HTTP/1.1 " << status_ << " " << StatusToString(status_) << "\r\n";
    ss << "Content-Length: " << copy->pubseekoff(0, body_.end) << "\r\n";
    ss << "\r\n";
    ss << copy;

    return ss.str();
  }

  size_t GetBodySize() const {
    // the below is from this answer:
    // https://stackoverflow.com/a/27429040
    std::streambuf* copy = body_.rdbuf();
    return copy->pubseekoff(0, body_.end);
  }

 private:
  Status status_;
  std::stringstream body_;
  tcp::SocketWriter _socket_writer;
};
}  // namespace http
}  // namespace domino
#endif  // HTTP_RESPONSE_H_
