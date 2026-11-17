#pragma once
#include <memory>
#include <string>

#include "domain/User.hpp"
#include "repository/IUserRepository.hpp"

namespace service {

// Регистрация, вход и текущая сессия пользователя.
class AuthService {
public:
    explicit AuthService(std::shared_ptr<IUserRepository> userRepository);

    // Возвращает пользователя при верных логине и пароле, иначе nullptr.
    std::shared_ptr<User> login(const std::string& login, const std::string& password);

    // Регистрирует клиента и сразу открывает для него сессию.
    // Бросает std::invalid_argument при некорректных данных или занятом логине,
    // std::runtime_error — если не удалось сохранить пользователя.
    std::shared_ptr<User> registerUser(const std::string& login, const std::string& password,
                                       const std::string& contacts = "");

    void logout() { currentUser.reset(); }
    std::shared_ptr<User> getCurrentUser() const { return currentUser; }
    bool isAuthenticated() const { return currentUser != nullptr; }
    // Есть ли открытая сессия с указанной ролью (проверка прав перед действием).
    bool hasRole(UserRole role) const { return currentUser && currentUser->getRole() == role; }

    // Детерминированный хеш пароля (FNV-1a 64 бит, логин используется как соль) в виде 16 hex-символов.
    // Один и тот же логин+пароль всегда дают одинаковый хеш — по нему же заполняются тестовые данные в БД.
    static std::string hashPassword(const std::string& login, const std::string& password);

    static void validateLogin(const std::string& login);
    static void validatePassword(const std::string& password);

private:
    std::shared_ptr<IUserRepository> userRepository;
    std::shared_ptr<User> currentUser;
};

} // namespace service
