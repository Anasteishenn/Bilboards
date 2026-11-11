#pragma once

#include <string>
#include <optional>
#include "repository/IUserRepository.hpp"
#include "repository/DatabaseManager.hpp"
#include "sqlite3.h"

class SqliteUserRepository : public IUserRepository {
public:
    explicit SqliteUserRepository(DatabaseManager& dbManager);
    ~SqliteUserRepository() override = default;

    std::optional<User> getById(int id) override;
    std::optional<User> getByLogin(const std::string& login) override;
    bool save(const User& user) override;

private:
    DatabaseManager& dbManager_;

    static std::string roleToString(UserRole role);
    static UserRole stringToRole(const std::string& roleStr);
};