#include <string>
#include <unordered_map>

namespace domino {
namespace http {

enum Method { kGet, kPost };

struct http_request {
  Method method;
  std::string endpoint;
};

struct http_get : http_request {
  std::unordered_map<std::string, std::string> args;
};

struct http_post : http_request {
  int file_descriptor;
  std::string content_type;
};

}  // namespace http
}  // namespace domino
