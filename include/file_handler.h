#include <chrono>
#include <fstream>
#include <string>

#ifndef FILE_HANDLER_H_
#define FILE_HANDLER_H_

namespace domino {

namespace handler {
constexpr char kFileFormat[] = "data_%llu.data";

class FileHandler {
 private:
  std::fstream file_stream;
  static bool file_exists(std::string file_name) {
    std::ifstream file;
    file.open(file_name);
    bool exist = (bool)file;
    file.close();
    return exist;
  }

 public:
  static std::string createFile() {
    std::string file_name;
    do {
      char buffer_file_name[50] = {0};
      uint64_t epochs_ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count();
      sprintf(buffer_file_name, kFileFormat, epochs_ms);
      file_name = std::string(buffer_file_name);
    } while (file_exists(file_name));
    std::ofstream file(file_name);
    file.close();
    return file_name;
  };
  static bool deleteFile(std::string file_name);

  bool selectFile(std::string file_name) {
    if (!file_exists(file_name)) {
      return false;
    }
    this->file_stream = std::fstream();
    this->file_stream.open(file_name,
                           std::ios::out | std::ios::in | std::ios::binary);
    return true;
  }

  void writeFileBuffer(char* buffer, size_t n_bytes) {
    this->file_stream.write(buffer, n_bytes);
  }

  size_t readFileBuffer(char* buffer, size_t n_bytes) {
    this->file_stream.read(buffer, n_bytes);
    if (!this->file_stream) {
      return this->file_stream.gcount();
    }
    return n_bytes;
  }

  unsigned long long getFileSize() {
    file_stream.seekg(0, file_stream.end);
    unsigned long long fsize = file_stream.tellg();
    file_stream.seekg(0, file_stream.beg);
    return fsize;
  }

  bool isEof() { return this->file_stream.peek() == EOF; }

  void close() { file_stream.close(); }
};
}  // namespace handler
}  // namespace domino

#endif  // FILE_HANDLER_H_
