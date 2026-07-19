#pragma once

#include <json/json.h>
#include <sqlite3.h>

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace crewgle {

using SqlValue = std::variant<int, long long, double, std::string>;

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void exec(const std::string& sql);
    void run(const std::string& sql, const std::vector<SqlValue>& params = {});
    Json::Value query(const std::string& sql, const std::vector<SqlValue>& params = {});
    Json::Value one(const std::string& sql, const std::vector<SqlValue>& params = {});
    int scalarInt(const std::string& sql, const std::vector<SqlValue>& params = {});
    int lastInsertId() const;

private:
    void bind(sqlite3_stmt* stmt, const std::vector<SqlValue>& params);
    sqlite3* db_ = nullptr;
};

using DatabasePtr = std::shared_ptr<Database>;

}
