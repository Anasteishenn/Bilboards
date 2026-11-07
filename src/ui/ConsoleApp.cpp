#include "ui/ConsoleApp.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>

namespace ui {

ConsoleApp::ConsoleApp(std::shared_ptr<service::AuthService> authService,
                       std::shared_ptr<service::BillboardService> billboardService,
                       std::shared_ptr<service::BookingService> bookingService)
    : authService_(std::move(authService)),
      billboardService_(std::move(billboardService)),
      bookingService_(std::move(bookingService)) {}

void ConsoleApp::clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int ConsoleApp::readInt(const std::string& prompt, int minVal, int maxVal) {
    int value = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= minVal && value <= maxVal) {
            clearInputBuffer();
            return value;
        }
        std::cout << "  [!] Ошибка ввода. Введите целое число от " 
                  << minVal << " до " << maxVal << ".\n";
        clearInputBuffer();
    }
}

double ConsoleApp::readDouble(const std::string& prompt, double minVal) {
    double value = 0.0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= minVal) {
            clearInputBuffer();
            return value;
        }
        std::cout << "  [!] Ошибка ввода. Введите число не меньше " << minVal << ".\n";
        clearInputBuffer();
    }
}

std::string ConsoleApp::readLine(const std::string& prompt) {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        if (!input.empty()) {
            return input;
        }
        std::cout << "  [!] Поле не может быть пустым. Повторите ввод.\n";
    }
}

void ConsoleApp::run() {
    bool isRunning = true;
    while (isRunning) {
        if (!currentUser_) {
            showAuthMenu();
            int choice = readInt("Выберите действие: ", 0, 2);
            switch (choice) {
                case 1:
                    handleLogin();
                    break;
                case 2:
                    handleRegister();
                    break;
                case 0:
                    std::cout << "\nЗавершение работы системы AdBoard Core. До свидания!\n";
                    isRunning = false;
                    break;
            }
        } else {
            routeUserSession();
        }
    }
}

void ConsoleApp::showAuthMenu() {
    std::cout << "\n============================================\n"
              << "   СИСТЕМА БРОНИРОВАНИЯ ЩИТОВ ADBOARD CORE  \n"
              << "============================================\n"
              << "  1. Вход в систему\n"
              << "  2. Регистрация нового клиента\n"
              << "  0. Выход\n"
              << "--------------------------------------------\n";
}

void ConsoleApp::handleLogin() {
    std::cout << "\n--- Авторизация ---\n";
    std::string username = readLine("Логин: ");
    std::string password = readLine("Пароль: ");

    try {
        currentUser_ = authService_->login(username, password);
        if (currentUser_) {
            std::cout << "\n[ОК] Добро пожаловать, " << currentUser_->getUsername() << "!\n";
        } else {
            std::cout << "\n[!] Неверный логин или пароль.\n";
        }
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка авторизации]: " << ex.what() << "\n";
    }
}

void ConsoleApp::handleRegister() {
    std::cout << "\n--- Регистрация клиента ---\n";
    std::string username = readLine("Придумайте логин: ");
    std::string password = readLine("Придумайте пароль: ");
    std::string contact = readLine("Контактные данные (телефон/email): ");

    try {
        currentUser_ = authService_->registerUser(username, password, contact);
        std::cout << "\n[ОК] Регистрация успешно завершена! Вы вошли как " 
                  << currentUser_->getUsername() << ".\n";
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка регистрации]: " << ex.what() << "\n";
    }
}

void ConsoleApp::routeUserSession() {
    if (currentUser_->getRole() == domain::UserRole::Client) {
        std::cout << "\n[Переход в панель клиента...]\n";
    } else if (currentUser_->getRole() == domain::UserRole::Manager ||
               currentUser_->getRole() == domain::UserRole::Admin) {
        std::cout << "\n[Переход в панель менеджера...]\n";
    }

    currentUser_.reset();
    std::cout << "\nВы вышли из учетной записи.\n";
}

}

void ConsoleApp::routeUserSession() {
    if (currentUser_->getRole() == domain::UserRole::Client) {
        ClientView clientView(currentUser_, billboardService_, bookingService_);
        clientView.render();
    } else if (currentUser_->getRole() == domain::UserRole::Manager ||
               currentUser_->getRole() == domain::UserRole::Admin) {
        std::cout << "\n[Переход в панель менеджера...]\n";
        // В Коммите №3 заменить заглушку на ManagerView
    }

    currentUser_.reset();
    std::cout << "\nВы вышли из учетной записи.\n";
}