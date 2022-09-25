#include <sqlite3.h>

#include <string>

const char *kDbName = "file_metadata.db";
const char *kCreateFileTable =
    "CREATE TABLE IF NOT EXISTS file_metadata(id text primary_key not null, "
    "file_name"
    "text not null, deleted boolean not null);";

namespace domino {
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

  void createMainTable() { execActionSqlStmt(kCreateFileTable); }

 public:
  DbHandler() {
    int rc = sqlite3_open(kDbName, &db);
    checkRC(rc);

    createMainTable();
  }
};

struct db_row {
  std::string id;
  std::string file_name;
  bool deleted;
};

}  // namespace domino