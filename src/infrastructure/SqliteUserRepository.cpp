#include "infrastructure/SqliteUserRepository.hpp"
#include <iostream>

SqliteUserRepository::SqliteUserRepository(DatabaseManager& dbManager)
    : dbManager_(dbManager) {}

std::string SqliteUserRepository::roleToString(UserRole role) {
    switch (role) {
        case UserRole::Admin:   return "Admin";
        case UserRole::Manager: return "Manager";
        case UserRole::Client:
        default:                return "Client";
    }
}

UserRole SqliteUserRepository::stringToRole(const std::string& roleStr) {
    if (roleStr == "Admin")   return UserRole::Admin;
    if (roleStr == "Manager") return UserRole::Manager;
    return UserRole::Client;
}

std::optional<User> SqliteUserRepository::getById(int id) {
    sqlite3* db = dbManager_.getConnection();
    if (!db) return std::nullopt;

    const char* sql = "SELECT id, username, password_hash, role FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки getById: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id); //привязываем id к знаку ?

    std::optional<User> result = std::nullopt;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int userId = sqlite3_column_int(stmt, 0);
        std::string login = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string passHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string roleStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        result = User(userId, login, passHash, stringToRole(roleStr));
    }

    sqlite3_finalize(stmt);
    return result;
}

std::optional<User> SqliteUserRepository::getByLogin(const std::string& login) {
    sqlite3* db = dbManager_.getConnection();
    if (!db) return std::nullopt;

    const char* sql = "SELECT id, username, password_hash, role FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки getByLogin: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<User> result = std::nullopt;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int userId = sqlite3_column_int(stmt, 0);
        std::string userLogin = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string passHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string roleStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        result = User(userId, userLogin, passHash, stringToRole(roleStr));
    }

    sqlite3_finalize(stmt);
    return result;
}

bool SqliteUserRepository::save(const User& user) {
    sqlite3* db = dbManager_.getConnection();
    if (!db) return false;

    const char* sql = "INSERT INTO users (username, password_hash, role) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки save: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string roleStr = roleToString(user.getRole());

    sqlite3_bind_text(stmt, 1, user.getLogin().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.getPasswordHash().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, roleStr.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Ошибка вставки пользователя: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}