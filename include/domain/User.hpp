#pragma once
#include <string>

enum class UserRole {
    Client,
    Manager,
    Admin
};

class User {
private:
    int id{0};
    std::string login;
    std::string passwordHash;
    UserRole role{UserRole::Client};

public:
    User() = default;
    User(int id, std::string login, std::string passwordHash, UserRole role)
        : id(id), login(std::move(login)), passwordHash(std::move(passwordHash)), role(role) {}

    int getId() const { return id; }
    const std::string& getLogin() const { return login; }
    const std::string& getPasswordHash() const { return passwordHash; }
    UserRole getRole() const { return role; }
};