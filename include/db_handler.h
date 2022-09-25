#include <sqlite3.h>

#include <iostream>
#include <string>
#include <vector>

const char *kDbName = "file_metadata.db";
const char *kCreateFileTable =
    "CREATE TABLE IF NOT EXISTS file_metadata("
    "id text primary_key not null, "
    "file_name text not null unique, "
    "deleted boolean not null"
    ");";

const char *kQueryFile = "SELECT * FROM file_metadata WHERE file_name=\"%s\";";
const char *kInsertRow =
    "INSERT INTO file_metadata (id, file_name, deleted) VALUES "
    "(\"%s\",\"%s\",false);";
const char *kMarkDelete =
    "UPDATE file_metadata SET deleted = true WHERE file_name=\"%s\";";
const char *kQueryDelete = "SELECT * FROM file_meta WHERE deleted = true;";

namespace domino {
struct db_row {
  std::string id;
  std::string file_name;
  bool deleted;
};

class DbHandler {
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

  std::vector<db_row> execQuerySqlStmt(const char *sql_stmt) {
    std::vector<db_row> rows;

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
  DbHandler() {
    int rc = sqlite3_open(kDbName, &db);
    checkRC(rc);

    createMainTable();
  }

  bool markFileDeleted(std::string file_name) {
    char sql_stmt[512];
    sprintf(sql_stmt, kMarkDelete, file_name.c_str());

    std::vector<db_row> rows = execQuerySqlStmt(sql_stmt);

    return rows.size() == 1;
  }

  bool insertFile(std::string id, std::string file_name) {
    char sql_stmt[512];
    sprintf(sql_stmt, kInsertRow, id.c_str(), file_name.c_str());

    std::vector<db_row> rows = execQuerySqlStmt(sql_stmt);

    return rows.size() == 1;
  }

  std::optional<db_row> getFile(std::string file_name) {
    char sql_stmt[512];
    sprintf(sql_stmt, kQueryFile, file_name.c_str());

    std::vector<db_row> rows = execQuerySqlStmt(sql_stmt);

    if (rows.size() == 1) return rows[0];
    return {};
  }

  std::vector<db_row> getDeleteRows() { return execQuerySqlStmt(kQueryDelete); }
};

}  // namespace domino