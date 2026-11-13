#pragma once

#include <memory>
#include <vector>
#include "domain/User.hpp"
#include "domain/Billboard.hpp"
#include "service/BillboardService.hpp"
#include "service/BookingService.hpp"

namespace ui {

class ClientView {
public:
    ClientView(std::shared_ptr<domain::User> client,
               std::shared_ptr<service::BillboardService> billboardService,
               std::shared_ptr<service::BookingService> bookingService);

    // Главный цикл меню клиента
    void render();

private:
    void showMenu();
    void displayCatalog(const std::vector<domain::Billboard>& billboards);
    void handleViewAllBillboards();
    void handleFilterBillboards();
    void handleCreateBooking();
    void handleViewMyOrders();

    std::shared_ptr<domain::User> client_;
    std::shared_ptr<service::BillboardService> billboardService_;
    std::shared_ptr<service::BookingService> bookingService_;
};

} // namespace ui