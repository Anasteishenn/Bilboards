#include "ui/ConsoleApp.hpp"
#include "ui/AdminView.hpp"
#include "ui/ClientView.hpp"
#include "ui/ManagerView.hpp"

#include <cctype>
#include <cstdio>
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

namespace {

// Читает строку целиком; при закрытом потоке завершает приложение через InputClosed.
std::string readRawLine() {
    std::string input;
    if (!std::getline(std::cin, input)) {
        throw InputClosed{};
    }
    if (!input.empty() && input.back() == '\r') {
        input.pop_back();
    }
    const auto first = input.find_first_not_of(" \t");
    const auto last = input.find_last_not_of(" \t");
    return first == std::string::npos ? "" : input.substr(first, last - first + 1);
}

} // namespace

int ConsoleApp::readInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::cout << prompt;
        const std::string input = readRawLine();
        try {
            std::size_t used = 0;
            const int value = std::stoi(input, &used);
            if (used == input.size() && value >= minVal && value <= maxVal) {
                return value;
            }
        } catch (const std::exception&) {
            // не число или переполнение — сообщение ниже
        }
        std::cout << "  [!] Ошибка ввода. Введите целое число от "
                  << minVal << " до " << maxVal << ".\n";
    }
}

double ConsoleApp::readDouble(const std::string& prompt, double minVal) {
    while (true) {
        std::cout << prompt;
        std::string input = readRawLine();
        for (char& ch : input) {
            if (ch == ',') ch = '.';   // принимаем и "55,5"
        }
        try {
            std::size_t used = 0;
            const double value = std::stod(input, &used);
            if (used == input.size() && value >= minVal) {
                return value;
            }
        } catch (const std::exception&) {
            // не число — сообщение ниже
        }
        std::cout << "  [!] Ошибка ввода. Введите число не меньше " << minVal << ".\n";
    }
}

std::string ConsoleApp::readLine(const std::string& prompt, bool allowEmpty) {
    while (true) {
        std::cout << prompt;
        std::string input = readRawLine();
        if (!input.empty() || allowEmpty) {
            return input;
        }
        std::cout << "  [!] Поле не может быть пустым. Повторите ввод.\n";
    }
}

std::string ConsoleApp::readDate(const std::string& prompt) {
    while (true) {
        const std::string input = readLine(prompt);
        try {
            return DateRange::normalizeDate(input);
        } catch (const std::invalid_argument&) {
            std::cout << "  [!] Некорректная дата. Формат: ДД.ММ.ГГГГ (например, 10.10.2026).\n";
        }
    }
}

bool ConsoleApp::confirm(const std::string& prompt) {
    while (true) {
        const std::string answer = readLine(prompt + " (y/n): ");
        if (answer == "y" || answer == "Y" || answer == "д" || answer == "Д") return true;
        if (answer == "n" || answer == "N" || answer == "н" || answer == "Н") return false;
        std::cout << "  [!] Введите y (да) или n (нет).\n";
    }
}

std::size_t ConsoleApp::displayWidth(const std::string& text) {
    std::size_t width = 0;
    for (unsigned char ch : text) {
        if ((ch & 0xC0) != 0x80) ++width;   // байты продолжения UTF-8 (10xxxxxx) не считаем
    }
    return width;
}

std::string ConsoleApp::fit(const std::string& text, std::size_t width, bool alignRight) {
    std::string result;
    std::size_t symbols = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto ch = static_cast<unsigned char>(text[i]);
        if ((ch & 0xC0) != 0x80) {
            if (symbols == width) break;    // обрезаем по границе символа, не разрывая UTF-8
            ++symbols;
        }
        result += text[i];
    }
    const std::string padding(width - symbols, ' ');
    return alignRight ? padding + result : result + padding;
}

std::string ConsoleApp::money(double value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f", value);
    return buffer;
}

void ConsoleApp::run() {
    try {
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
    } catch (const InputClosed&) {
        std::cout << "\n\nВвод завершён. До свидания!\n";
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
            std::cout << "\n[ОК] Добро пожаловать, " << currentUser_->getLogin() << "!\n";
        } else {
            std::cout << "\n[!] Неверный логин или пароль.\n";
        }
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка авторизации]: " << ex.what() << "\n";
    }
}

void ConsoleApp::handleRegister() {
    std::cout << "\n--- Регистрация клиента ---\n"
              << "  Логин: 3–32 символа (латиница, цифры, '_' и '.'), пароль — не менее 4 символов.\n";
    std::string username = readLine("Придумайте логин: ");
    std::string password = readLine("Придумайте пароль: ");
    if (readLine("Повторите пароль: ") != password) {
        std::cout << "\n[!] Пароли не совпадают. Регистрация отменена.\n";
        return;
    }
    std::string contact = readLine("Контактные данные (телефон/email): ");

    try {
        currentUser_ = authService_->registerUser(username, password, contact);
        std::cout << "\n[ОК] Регистрация успешно завершена! Вы вошли как "
                  << currentUser_->getLogin() << ".\n";
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка регистрации]: " << ex.what() << "\n";
    }
}

void ConsoleApp::routeUserSession() {
    switch (currentUser_->getRole()) {
        case UserRole::Client: {
            ClientView clientView(currentUser_, billboardService_, bookingService_);
            clientView.render();
            break;
        }
        case UserRole::Manager: {
            ManagerView managerView(currentUser_, billboardService_, bookingService_);
            managerView.render();
            break;
        }
        case UserRole::Admin: {
            AdminView adminView(currentUser_, billboardService_, bookingService_);
            adminView.render();
            break;
        }
    }

    authService_->logout();
    currentUser_.reset();
    std::cout << "\nВы вышли из учетной записи.\n";
}

} // namespace ui
