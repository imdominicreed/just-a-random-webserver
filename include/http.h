#include <string>
#include <unordered_map>

#ifndef HTTP_H_
#define HTTP_H_
namespace domino {
namespace http {

enum Method { kGet, kPost };

struct http_request {
  Method method;
  std::string endpoint;
  std::unordered_map<std::string, std::string> args;
};

}  // namespace http
}  // namespace domino
#endif  // HTTP_H_
