#include "service/AuthService.hpp"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

namespace service {

AuthService::AuthService(std::shared_ptr<IUserRepository> userRepository)
    : userRepository(std::move(userRepository)) {
    if (!this->userRepository) {
        throw std::invalid_argument("AuthService: не передан репозиторий пользователей");
    }
}

std::shared_ptr<User> AuthService::login(const std::string& login, const std::string& password) {
    const auto user = userRepository->getByLogin(login);
    if (!user || user->getPasswordHash() != hashPassword(login, password)) {
        return nullptr;
    }
    currentUser = std::make_shared<User>(*user);
    return currentUser;
}

std::shared_ptr<User> AuthService::registerUser(const std::string& login, const std::string& password,
                                                const std::string& contacts) {
    validateLogin(login);
    validatePassword(password);
    if (userRepository->getByLogin(login)) {
        throw std::invalid_argument("Логин \"" + login + "\" уже занят");
    }

    const User newUser(0, login, hashPassword(login, password), UserRole::Client, contacts);
    if (!userRepository->save(newUser)) {
        throw std::runtime_error("Не удалось сохранить пользователя");
    }

    // id назначает база данных, поэтому перечитываем сохранённую запись.
    const auto saved = userRepository->getByLogin(login);
    currentUser = std::make_shared<User>(saved ? *saved : newUser);
    return currentUser;
}

std::string AuthService::hashPassword(const std::string& login, const std::string& password) {
    std::uint64_t hash = 14695981039346656037ULL;
    for (unsigned char ch : login + ":" + password) {
        hash ^= ch;
        hash *= 1099511628211ULL;
    }
    char buffer[17];
    std::snprintf(buffer, sizeof(buffer), "%016llx", static_cast<unsigned long long>(hash));
    return buffer;
}

void AuthService::validateLogin(const std::string& login) {
    if (login.size() < 3 || login.size() > 32) {
        throw std::invalid_argument("Логин должен содержать от 3 до 32 символов");
    }
    for (unsigned char ch : login) {
        if (!std::isalnum(ch) && ch != '_' && ch != '.') {
            throw std::invalid_argument("Логин может содержать только латинские буквы, цифры, '_' и '.'");
        }
    }
}

void AuthService::validatePassword(const std::string& password) {
    if (password.size() < 4) {
        throw std::invalid_argument("Пароль должен содержать не менее 4 символов");
    }
    if (password.find(' ') != std::string::npos) {
        throw std::invalid_argument("Пароль не должен содержать пробелов");
    }
}

} // namespace service
