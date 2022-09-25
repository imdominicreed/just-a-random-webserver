#include <iostream>
#include <sstream>
#include <string>

#include "../include/db_handler.h"

using namespace domino;

std::string kTestDb = "bin/test_db.db";

int main() {
  DbHandler db(kTestDb);
  while (true) {
    try {
      std::string line;
      std::getline(std::cin, line);

      std::stringstream stream(line);
      std::string function;
      stream >> function;
      if ("markFileDeleted" == function) {
        std::string file_name;
        stream >> file_name;
        if (db.markFileDeleted(file_name))
          std::cout << "Mark Deleted!" << std::endl;
        else
          std::cout << "No Mark Deletetion!" << std::endl;

      } else if ("insertFile" == function) {
        std::string file_name, id;
        stream >> id >> file_name;
        if (db.insertFile(id, file_name))
          std::cout << "Inserted!" << std::endl;
        else
          std::cout << "Not inserted!" << std::endl;

      } else if ("getFile" == function) {
        std::string file_name;
        stream >> file_name;
        auto opt = db.getFile(file_name);
        if (opt.has_value()) {
          std::cout << opt.value().toString() << std::endl;
        } else
          std::cout << "File not Found!" << std::endl;

      } else if ("getDeleteRows" == function) {
        auto rows = db.getDeleteRows();
        std::cout << "Printing rows!" << std::endl;
        for (auto r : rows) std::cout << r.toString() << std::endl;
      } else if ("deleteRow" == function) {
        std::string file_name;
        stream >> file_name;
        if (db.deleteRow(file_name))
          std::cout << "Deleted!" << std::endl;
        else
          std::cout << "No Deletetion!" << std::endl;
      } else if ("quit" == function)
        return 0;
    } catch (const std::exception& e) {
      std::cout << e.what();
    }
  }
}