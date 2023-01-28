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
};
}  // namespace http
}  // namespace domino
#endif  // HTTP_RESPONSE_H_
