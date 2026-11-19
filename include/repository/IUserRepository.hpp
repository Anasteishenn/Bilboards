#pragma once
#include <string>
#include <optional>
#include "domain/User.hpp"

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual std::optional<User> getById(int id) = 0;
    virtual std::optional<User> getByLogin(const std::string& login) = 0;
    virtual bool save(const User& user) = 0;
};