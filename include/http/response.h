#ifndef HTTP_RESPONSE_H_
#define HTTP_RESPONSE_H_

#include <sstream>
#include <string_view>

#include "http/status.h"

namespace domino {
namespace http {
class Response {
 public:
  Response() : status_(Status::kOk) {}
  void SetStatus(Status status) { status_ = status; }

  std::string ToBuffer() {
    std::stringstream ss;

    ss << "HTTP/1.1 " << status_ << " " << StatusToString(status_) << "\r\n";

    ss << "Content-Length: " << GetBodyLength() << "\r\n";
    ss << "\r\n";
    ss << body_.str();

    return ss.str();
  }

 private:
  Status status_;
  std::stringstream body_;
  size_t GetBodyLength() {
    body_.seekp(0, std::ios::end);
    return body_.tellp();
  }
};
}  // namespace http
}  // namespace domino
#endif  // HTTP_RESPONSE_H_
