#ifndef HTTP_STATUS_H_
#define HTTP_STATUS_H_

#include <iostream>
#include <string>

namespace domino {
namespace http {
enum class Status {
  kOk = 200,
  kBadRequest = 400,
  kNotFound = 404,
  kInternalServerError = 500,
};

std::ostream& operator<<(std::ostream& out, const Status& value) {
  out << std::to_string(static_cast<int>(value));
  return out;
}

inline std::string_view StatusToString(const Status& status) {
  switch (status) {
    case Status::kOk:
      return "OK";
    case Status::kBadRequest:
      return "BAD REQUEST";
    case Status::kNotFound:
      return "NOT FOUND";
    case Status::kInternalServerError:
      return "INTERNAL SERVER ERROR";
    default:
      return "-";
  }
}
}  // namespace http
}  // namespace domino
#endif  // HTTP_STATUS_H_
