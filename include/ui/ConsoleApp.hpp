#pragma once

#include <memory>
#include <string>
#include "domain/User.hpp"
#include "service/AuthService.hpp"
#include "service/BillboardService.hpp"
#include "service/BookingService.hpp"

namespace ui {

// Бросается, когда поток ввода закрыт (Ctrl+Z / Ctrl+D). Не наследуется от std::exception,
// чтобы его не перехватили обработчики ошибок сервисов — приложение завершается штатно в run().
struct InputClosed {};

class ConsoleApp {
public:
    ConsoleApp(std::shared_ptr<service::AuthService> authService,
               std::shared_ptr<service::BillboardService> billboardService,
               std::shared_ptr<service::BookingService> bookingService);

    void run();

    // Защищённый ввод: повторяет запрос до корректного значения, сбрасывает флаги std::cin и очищает буфер.
    static void clearInputBuffer();
    static int readInt(const std::string& prompt, int minVal, int maxVal);
    static double readDouble(const std::string& prompt, double minVal);
    static std::string readLine(const std::string& prompt, bool allowEmpty = false);
    // Дата в формате ДД.ММ.ГГГГ или ГГГГ-ММ-ДД, возвращается в ISO.
    static std::string readDate(const std::string& prompt);
    static bool confirm(const std::string& prompt);

    // Выравнивание колонок таблиц с учётом кириллицы (ширина считается в символах UTF-8, а не в байтах).
    static std::size_t displayWidth(const std::string& text);
    static std::string fit(const std::string& text, std::size_t width, bool alignRight = false);
    static std::string money(double value);

private:
    void showAuthMenu();
    void handleLogin();
    void handleRegister();
    void routeUserSession();

    std::shared_ptr<service::AuthService> authService_;
    std::shared_ptr<service::BillboardService> billboardService_;
    std::shared_ptr<service::BookingService> bookingService_;
    std::shared_ptr<User> currentUser_{nullptr};
};

} // namespace ui
