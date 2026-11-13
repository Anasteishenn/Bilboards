#pragma once

#include <memory>
#include <string>
#include "domain/User.hpp"
#include "service/AuthService.hpp"
#include "service/BillboardService.hpp"
#include "service/BookingService.hpp"

namespace ui {

class ConsoleApp {
public:
    ConsoleApp(std::shared_ptr<service::AuthService> authService,
               std::shared_ptr<service::BillboardService> billboardService,
               std::shared_ptr<service::BookingService> bookingService);

  void run();

    static void clearInputBuffer();
    static int readInt(const std::string& prompt, int minVal, int maxVal);
    static double readDouble(const std::string& prompt, double minVal);
    static std::string readLine(const std::string& prompt);

private:
    void showAuthMenu();
    void handleLogin();
    void handleRegister();
    void routeUserSession();

    std::shared_ptr<service::AuthService> authService_;
    std::shared_ptr<service::BillboardService> billboardService_;
    std::shared_ptr<service::BookingService> bookingService_;
    std::shared_ptr<domain::User> currentUser_{nullptr};
};

} 