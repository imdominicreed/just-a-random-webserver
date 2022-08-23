#include <string>
#include <unordered_map>

enum http_type { get, post };

struct http_request {
  http_type type;
  std::string url;
};

struct http_get : http_request {
  std::unordered_map<std::string, std::string> args;
};

struct http_post : http_request {
  int file_descriptor;
  std::string content_type;
};
