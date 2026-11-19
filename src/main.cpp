#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <clocale>
#include <exception>
#include <iostream>
#include <memory>

#include "repository/DatabaseManager.hpp"
#include "infrastructure/SqliteUserRepository.hpp"
#include "infrastructure/SqliteBillboardRepository.hpp"
#include "infrastructure/SqliteOrderRepository.hpp"
#include "service/AuthService.hpp"
#include "service/BillboardService.hpp"
#include "service/BookingService.hpp"
#include "ui/ConsoleApp.hpp"

// Composition Root: DatabaseManager -> репозитории -> сервисы -> ConsoleApp.
int main() {
#ifdef _WIN32
    // Кириллица в консоли Windows: ввод и вывод в UTF-8.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::setlocale(LC_ALL, "");

    try {
        DatabaseManager dbManager("data/adboard.db");
        if (!dbManager.initialize("data/schema.sql")) {
            std::cerr << "\n[КРИТИЧЕСКИЙ СБОЙ СИСТЕМЫ]: не удалось открыть базу данных data/adboard.db "
                         "или выполнить data/schema.sql. Запускайте программу из папки, где лежит data/.\n";
            return 1;
        }

        auto userRepo = std::make_shared<SqliteUserRepository>(dbManager);
        auto billboardRepo = std::make_shared<SqliteBillboardRepository>(dbManager);
        auto orderRepo = std::make_shared<SqliteOrderRepository>(dbManager);

        auto authService = std::make_shared<service::AuthService>(userRepo);
        auto billboardService = std::make_shared<service::BillboardService>(billboardRepo);
        auto bookingService = std::make_shared<service::BookingService>(orderRepo, billboardRepo);

        ui::ConsoleApp app(authService, billboardService, bookingService);
        app.run();

    } catch (const std::exception& ex) {
        std::cerr << "\n[КРИТИЧЕСКИЙ СБОЙ СИСТЕМЫ]: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
