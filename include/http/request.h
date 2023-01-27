#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include <string>
#include <string_view>
#include <unordered_map>

namespace domino {
namespace http {

constexpr char kSlash = '/';

enum class Method { kGet, kPost, kUnsupported };

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
  Request(std::string_view raw_http_request)
      : raw_http_request(std::string(raw_http_request)) {}
  void Initialize() {
    ParseMethodAndEndpointFromHttpRequest();
    ParseHeadersFromHttpRequest();
  }
  Method method;
  std::string endpoint;
  std::unordered_map<std::string, std::string> query;
  std::unordered_map<std::string, std::string> header;

 private:
  // return an index instead of making a new string variable?
  inline std::string GetStringUpToDelimiter(std::string_view given,
                                            std::string_view delimiter,
                                            const size_t &start) {
    size_t end = given.find(delimiter, start);
    return std::string(given.substr(start, end - start));
  }

  inline std::string_view RemoveTrailingSlashFromNonEmptyEndpoint(
      std::string_view endpoint) {
    // remove any trailing slashes on the parsed endpoint, i.e.
    // `/path/to/endpoint/` becomes `/path/to/endpoint`. we also
    // skip the logic if the endpoint is only a single char since it
    // would be a single slash.
    if (endpoint.size() > 1 && endpoint[endpoint.size() - 1] == kSlash) {
      return endpoint.substr(0, endpoint.length() - 1);
    }
    return endpoint;
  }

  // the endpoint parameter looks like /path/to/something or
  // path/to/something?parameter=1&another=hello
  void ParseEndpointAndQueryParameters(std::string_view endpoint) {
    std::unordered_map<std::string, std::string> args_map;
    // Find Start of Args
    size_t start = endpoint.find("?");
    // If npos then no args to read
    if (start == std::string::npos) {
      this->endpoint = RemoveTrailingSlashFromNonEmptyEndpoint(endpoint);
      return;
    }
    this->endpoint =
        RemoveTrailingSlashFromNonEmptyEndpoint(endpoint.substr(0, start));

    // Parses indivdual args
    size_t index = start;
    size_t next_index;
    while (index != std::string::npos) {
      index++;
      next_index = endpoint.find("&", index);
      std::string arg = std::string(endpoint.substr(index, next_index - index));
      int equal_index = arg.find("=");
      std::string key = arg.substr(0, equal_index);

      std::string value;
      // an index of -1 causes to substr to return the entire string
      // to avoid this, we assign the value to an empty string
      if (equal_index == -1) {
        value = "";
      } else {
        value = arg.substr(equal_index + 1);
      }
      args_map[key] = value;
      index = next_index;
    }
    query = args_map;
  }

  void ParseMethodAndEndpointFromHttpRequest() {
    /**
     * the first line of an HTTP request looks like:
     * GET /hello.htm HTTP/1.1
     * this function parses the above string and extracts:
     * 1. the http request method
     * 2. the endpoint
     * 3. any query parameters stored in the endpoint
     *
     * these values are stored to the domino::http::Request parameter sent in
     * as a pointer.
     *
     * for example, GET /path/to/something?value=1&another=hi HTTP/1.1 would
     * extract:
     * 1. the method as domino::http::Method::kGet
     * 2. the endpoint as /path/to/something
     * 3. the query parameters as
     * {
     *    "value": "1",
     *    "another": "hi"
     * }
     */
    size_t http_index = 0;
    std::string method_as_string =
        GetStringUpToDelimiter(raw_http_request, " ", http_index);
    http_index += method_as_string.length() + 1;
    std::string endpoint_and_args =
        GetStringUpToDelimiter(raw_http_request, " ", http_index);
    ParseEndpointAndQueryParameters(endpoint_and_args);
    method = StringToMethod(method_as_string);
  }

  void ParseHeadersFromHttpRequest() {
    size_t http_index = 0;
    std::string first_line =
        GetStringUpToDelimiter(raw_http_request, "\r\n", http_index);
    // we add 2 to account for us finding the \r\n characters
    http_index += first_line.length() + 2;
    std::unordered_map<std::string, std::string> header_map;
    std::string next_line;
    while (http_index < raw_http_request.length()) {
      next_line = GetStringUpToDelimiter(raw_http_request, "\r\n", http_index);
      std::string key = GetStringUpToDelimiter(next_line, ":", 0);
      if (key.length()) {
        std::string value =
            GetStringUpToDelimiter(next_line, "\r\n", key.length() + 2);
        header_map[key] = value;
      }
      http_index += next_line.length() + 2;
    }
    header = header_map;
  }
  std::string raw_http_request;
};

}  // namespace http
}  // namespace domino
#endif  // HTTP_REQUEST_H_
