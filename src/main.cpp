#include <iostream>
#include <memory>
#include <exception>

#include "repository/DatabaseManager.hpp"
#include "infrastructure/SqliteUserRepository.hpp"
#include "infrastructure/SqliteBillboardRepository.hpp"
#include "infrastructure/SqliteOrderRepository.hpp"
#include "service/AuthService.hpp"
#include "service/BillboardService.hpp"
#include "service/BookingService.hpp"
#include "ui/ConsoleApp.hpp"

int main() {
    std::setlocale(LC_ALL, "");

    try {
        auto dbManager = std::make_shared<repository::DatabaseManager>(
            "data/adboard.db", 
            "data/schema.sql"
        );
        dbManager->initialize();

        auto userRepo = std::make_shared<infrastructure::SqliteUserRepository>(dbManager);
        auto billboardRepo = std::make_shared<infrastructure::SqliteBillboardRepository>(dbManager);
        auto orderRepo = std::make_shared<infrastructure::SqliteOrderRepository>(dbManager);

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