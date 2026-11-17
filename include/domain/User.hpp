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
    std::string contacts;

public:
    User() = default;
    User(int id, std::string login, std::string passwordHash, UserRole role, std::string contacts = "")
        : id(id), login(std::move(login)), passwordHash(std::move(passwordHash)), role(role),
          contacts(std::move(contacts)) {}

    int getId() const { return id; }
    const std::string& getLogin() const { return login; }
    const std::string& getPasswordHash() const { return passwordHash; }
    UserRole getRole() const { return role; }
    // Телефон или e-mail (столбец users.contacts), может быть пустым.
    const std::string& getContacts() const { return contacts; }
};
