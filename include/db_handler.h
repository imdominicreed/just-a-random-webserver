#include <sqlite3.h>

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#ifndef DB_HANDLER_H_
#define DB_HANDLER_H_

namespace domino {
namespace sqlite {
const char *kDbName = "bin/file_metadata.db";
const char *kCreateFileTable =
    "CREATE TABLE IF NOT EXISTS file_metadata("
    "id text primary_key not null, "
    "file_name text not null unique, "
    "deleted boolean not null"
    ");";

const char *kQueryFile = "SELECT * FROM file_metadata WHERE file_name=\"%s\";";
const char *kInsertRow =
    "INSERT  or IGNORE  INTO file_metadata (id, file_name, deleted) VALUES "
    "(\"%s\",\"%s\",false);";
const char *kMarkDelete =
    "UPDATE file_metadata SET deleted = true WHERE file_name=\"%s\";";
const char *kQueryDelete = "SELECT * FROM file_metadata WHERE deleted = true;";
const char *kDeleteRow = "DELETE FROM file_metadata WHERE file_name=\"%s\";";

class Row {
 public:
  std::string id;
  std::string file_name;
  bool deleted;

  std::string toString() {
    return "id: " + id + " file name: " + file_name +
           " deleted: " + (deleted ? " True" : "False");
  }
};

class Database {
 private:
  sqlite3 *db;

  void checkRC(int rc) {
    if (rc != SQLITE_OK) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      throw std::runtime_error("Cannot open DB!");
    }
  }

  void execActionSqlStmt(const char *sql_stmt) {
    int rc = sqlite3_exec(db, sql_stmt, NULL, 0, NULL);
    checkRC(rc);
  }

  std::vector<Row> execQuerySqlStmt(const char *sql_stmt) {
    std::vector<Row> rows;

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, NULL);
    checkRC(rc);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      const char *id = (const char *)sqlite3_column_text(stmt, 0);
      const char *file_name = (const char *)sqlite3_column_text(stmt, 1);
      int deleted = sqlite3_column_int(stmt, 2);
      rows.push_back({std::string(id), std::string(file_name), deleted == 1});
    }

    sqlite3_finalize(stmt);

    return rows;
  }

  void createMainTable() { execActionSqlStmt(kCreateFileTable); }

 public:
  Database() {
    int rc = sqlite3_open(kDbName, &db);
    checkRC(rc);

    createMainTable();
  }

  Database(std::string test_db) {
    int rc = sqlite3_open(test_db.c_str(), &db);
    checkRC(rc);

    createMainTable();
  }

  bool markFileDeleted(std::string file_name) {
    char sql_stmt[512];
    sprintf(sql_stmt, kMarkDelete, file_name.c_str());

    execActionSqlStmt(sql_stmt);

    return sqlite3_changes(db) == 1;
  }

  bool insertFile(std::string id, std::string file_name) {
    char sql_stmt[512];
    sprintf(sql_stmt, kInsertRow, id.c_str(), file_name.c_str());

    execActionSqlStmt(sql_stmt);

    return sqlite3_changes(db) == 1;
  }

  std::optional<Row> getFile(std::string file_name) {
    char sql_stmt[512];
    sprintf(sql_stmt, kQueryFile, file_name.c_str());

    std::vector<Row> rows = execQuerySqlStmt(sql_stmt);

    if (rows.size() == 1) return rows[0];
    return {};
  }

  std::vector<Row> getDeleteRows() { return execQuerySqlStmt(kQueryDelete); }

  bool deleteRow(std::string file_name) {
    char sql_stmt[512];
    sprintf(sql_stmt, kDeleteRow, file_name.c_str());

    execActionSqlStmt(sql_stmt);

    return sqlite3_changes(db) == 1;
  }
};

}  // namespace sqlite
}  // namespace domino
#endif  // DB_HANDLER_H_
