#include "repository/DatabaseManager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

DatabaseManager::DatabaseManager(const std::string& dbPath)
    : dbPath_(dbPath), db_(nullptr) {}

DatabaseManager::~DatabaseManager() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::initialize(const std::string& schemaPath) {
    //открытие БД (создание)
    int rc = sqlite3_open(dbPath_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    //проверка связи между таблицами
    if (!executeScript("PRAGMA foreign_keys = ON;")) {
        return false;
    }

    //открываем файла data/schema.sql
    std::ifstream schemaFile(schemaPath);
    if (!schemaFile.is_open()) {
        std::cerr << "Не удалось открыть файл схемы: " << schemaPath << std::endl;
        return false;
    }

    //считываем schema.sql в буферную переменную, после чего преобразовываем её в строку
    std::stringstream buffer;
    buffer << schemaFile.rdbuf();
    std::string sqlScript = buffer.str();

    return executeScript(sqlScript);
}

bool DatabaseManager::executeScript(const std::string& sql) {
    if (db_ == nullptr) {
        return false;
    }

    char* errorMessage = nullptr;
    //функция SQLite для выполнения текстовых скриптов без возврата таблиц
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMessage);

    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка SQL: " << (errorMessage ? errorMessage : "Неизвестная ошибка") << std::endl;
        sqlite3_free(errorMessage);
        return false;
    }

    return true;
}

sqlite3* DatabaseManager::getConnection() const {
    return db_;
}