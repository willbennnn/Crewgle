#include "crewgle/Database.h"

#include <stdexcept>

namespace crewgle {

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }
    exec("PRAGMA foreign_keys = ON;");
}

Database::~Database() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
    }
}

void Database::exec(const std::string& sql) {
    char* error = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = error == nullptr ? "SQLite error" : error;
        sqlite3_free(error);
        throw std::runtime_error(message);
    }
}

void Database::bind(sqlite3_stmt* stmt, const std::vector<SqlValue>& params) {
    for (int i = 0; i < static_cast<int>(params.size()); ++i) {
        const int index = i + 1;
        if (std::holds_alternative<int>(params[i])) {
            sqlite3_bind_int(stmt, index, std::get<int>(params[i]));
        } else if (std::holds_alternative<long long>(params[i])) {
            sqlite3_bind_int64(stmt, index, std::get<long long>(params[i]));
        } else if (std::holds_alternative<double>(params[i])) {
            sqlite3_bind_double(stmt, index, std::get<double>(params[i]));
        } else {
            const auto& value = std::get<std::string>(params[i]);
            sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
        }
    }
}

void Database::run(const std::string& sql, const std::vector<SqlValue>& params) {
    sqlite3_stmt* raw = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &raw, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt(raw, sqlite3_finalize);
    bind(stmt.get(), params);

    const int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }
}

Json::Value Database::query(const std::string& sql, const std::vector<SqlValue>& params) {
    sqlite3_stmt* raw = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &raw, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt(raw, sqlite3_finalize);
    bind(stmt.get(), params);

    Json::Value rows(Json::arrayValue);
    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        Json::Value row;
        for (int i = 0; i < sqlite3_column_count(stmt.get()); ++i) {
            const char* name = sqlite3_column_name(stmt.get(), i);
            switch (sqlite3_column_type(stmt.get(), i)) {
                case SQLITE_INTEGER:
                    row[name] = static_cast<Json::Int64>(sqlite3_column_int64(stmt.get(), i));
                    break;
                case SQLITE_FLOAT:
                    row[name] = sqlite3_column_double(stmt.get(), i);
                    break;
                case SQLITE_NULL:
                    row[name] = Json::Value();
                    break;
                default:
                    row[name] = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), i));
                    break;
            }
        }
        rows.append(row);
    }
    return rows;
}

Json::Value Database::one(const std::string& sql, const std::vector<SqlValue>& params) {
    Json::Value rows = query(sql, params);
    return rows.empty() ? Json::Value() : rows[0];
}

int Database::scalarInt(const std::string& sql, const std::vector<SqlValue>& params) {
    Json::Value row = one(sql, params);
    if (row.isNull() || row.getMemberNames().empty()) {
        return 0;
    }
    return row[row.getMemberNames()[0]].asInt();
}

int Database::lastInsertId() const {
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

}
