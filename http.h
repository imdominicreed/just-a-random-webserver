#include <string>
#include <unordered_map>

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
