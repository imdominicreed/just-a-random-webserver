#include <string>
#include <unordered_map>

enum http_type { get, post };

struct http_request {
  http_type type;
};

struct http_get : http_request {
  std::string url;
  std::unordered_map<std::string, std::string> args;
};

struct http_post : http_request {
  std::string url;
  int fd;
  std::string content_type;
};
