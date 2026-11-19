#pragma once

#include <memory>
#include <vector>
#include "domain/User.hpp"
#include "domain/BookingOrder.hpp"
#include "service/BillboardService.hpp"
#include "service/BookingService.hpp"

namespace ui {

class ManagerView {
public:
    ManagerView(std::shared_ptr<User> manager,
                std::shared_ptr<service::BillboardService> billboardService,
                std::shared_ptr<service::BookingService> bookingService);

    void render();

    // Модерация и отчёт доступны и администратору (AdminView).
    void handleViewPendingOrders();
    void handleProcessOrderDecision();
    void handleOccupancyReport();

private:
    void showMenu();
    void handleViewAllOrders();

    std::shared_ptr<User> manager_;
    std::shared_ptr<service::BillboardService> billboardService_;
    std::shared_ptr<service::BookingService> bookingService_;
};

} // namespace ui
