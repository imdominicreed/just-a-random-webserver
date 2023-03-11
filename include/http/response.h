#ifndef HTTP_RESPONSE_H_
#define HTTP_RESPONSE_H_

#include <sstream>
#include <string_view>

#include "db_handler.h"
#include "file_handler.h"
#include "http/status.h"
#include "tcp/socket_writer.h"

namespace domino {
namespace http {
const size_t kBufferSize = 1024;
class Response {
 public:
  Response(int socket_fd)
      : status_(Status::kOk), _socket_writer(tcp::SocketWriter(socket_fd)) {
    body_ << "";
  }

  Response(const Response& original) {
    status_ = original.status_;
    body_ << original.body_.rdbuf();
    _socket_writer = tcp::SocketWriter(original._socket_writer);
  }

  void SetStatus(Status status) { status_ = status; }

  void SetBody(const std::string& value) {
    body_.clear();
    body_ << value;
  }

  void Send() {
    std::string buffer = this->ToBuffer();
    const char* buffer_as_char_pointer = buffer.c_str();
    _socket_writer.WriteBuffer(buffer_as_char_pointer,
                               strlen(buffer_as_char_pointer));
    _socket_writer.Close();
  }

  /**
   * Write a given HTTP status' response to the socket. Closes said socket
   * after writing.
   */
  void SendStaticResponse(Status status) {
    SetStatus(status);
    Send();
  };

  std::string ToBuffer() const {
    std::stringstream ss;

    std::streambuf* copy = body_.rdbuf();
    ss << "HTTP/1.1 " << status_ << " " << StatusToString(status_) << "\r\n";
    ss << "Content-Length: " << copy->pubseekoff(0, body_.end) << "\r\n";
    ss << "\r\n";
    ss << body_.str();

    return ss.str();
  }

  void SendFileHeader(size_t n) {
    std::stringstream ss;
    ss << "HTTP/1.1 " << status_ << " " << StatusToString(status_) << "\r\n";
    ss << "Content-Length: " << n << "\r\n";
    ss << "\r\n";

    std::string buffer = ss.str();
    const char* buffer_as_char_pointer = buffer.c_str();
    std::cout << buffer_as_char_pointer[0] << std::endl;
    _socket_writer.WriteBuffer(buffer_as_char_pointer,
                               strlen(buffer_as_char_pointer));
  }

  size_t GetBodySize() const {
    // the below is from this answer:
    // https://stackoverflow.com/a/27429040
    std::streambuf* copy = body_.rdbuf();
    return copy->pubseekoff(0, body_.end);
  }

  void SendFile(std::string request_file_name) {
    std::optional<sqlite::Row> file_row =
        _db_handler.getFile(request_file_name);

    if (!file_row.has_value()) {
      SetStatus(Status::kBadRequest);
      SetBody("File not found!");
      Send();
    } else {
      _file_handler.selectFile(file_row.value().file_name);

      SetStatus(Status::kOk);
      SendFileHeader(_file_handler.getFileSize());

      char buffer[kBufferSize];

      while (!_file_handler.isEof()) {
        size_t size = _file_handler.readFileBuffer(buffer, kBufferSize);
        _socket_writer.WriteBuffer(buffer, size);
      }
      _socket_writer.Close();
    }
  }

 private:
  Status status_;
  std::stringstream body_;
  tcp::SocketWriter _socket_writer;
  handler::FileHandler _file_handler;
  sqlite::Database _db_handler;
};
}  // namespace http
}  // namespace domino
#endif  // HTTP_RESPONSE_H_
