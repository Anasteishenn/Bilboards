#pragma once

#include <string>
#include "sqlite3.h"

class DatabaseManager {
public:
    explicit DatabaseManager(const std::string& dbPath);
    ~DatabaseManager();

    //запрет копирования
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    //запуск базы
    bool initialize(const std::string& schemaPath = "data/schema.sql");
    //для текстовых команд SQL
    bool executeScript(const std::string& sql);
    
    //получатель открытого соединения с БД
    sqlite3* getConnection() const;

private:
    std::string dbPath_;
    sqlite3* db_; //указатель на открытую БД
};