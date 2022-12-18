#include <string>
#include <string_view>
#include <unordered_map>

#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_
namespace domino {
namespace http {

enum Method { kGet, kPost, kUnsupported };

std::unordered_map<std::string, Method> string_to_method_map{};

inline Method StringToMethod(std::string_view method_as_string) {
  if (method_as_string == "GET") {
    return Method::kGet;
  } else if (method_as_string == "POST") {
    return Method::kPost;
  } else {
    return Method::kUnsupported;
  }
}

class Request {
 public:
  Method method;
  std::string endpoint;
  std::unordered_map<std::string, std::string> args;
};

}  // namespace http
}  // namespace domino
#endif  // HTTP_REQUEST_H_
