#include <string>
#include <unordered_map>

#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_
namespace domino {
namespace http {

enum Method { kGet, kPost };

class Request {
 public:
  Method method;
  std::string endpoint;
  std::unordered_map<std::string, std::string> args;
};

}  // namespace http
}  // namespace domino
#endif  // HTTP_REQUEST_H_
