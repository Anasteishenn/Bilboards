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
    ClientView(std::shared_ptr<User> client,
               std::shared_ptr<service::BillboardService> billboardService,
               std::shared_ptr<service::BookingService> bookingService);

    // Главный цикл меню клиента
    void render();

    // Таблица щитов — общая для клиента, менеджера и администратора.
    static void displayCatalog(const std::vector<Billboard>& billboards, bool showStatus = false);
    // Таблица заказов; withClient — добавить колонку с ID клиента (для менеджера).
    static void displayOrders(const std::vector<BookingOrder>& orders, bool withClient);

private:
    void showMenu();
    void handleViewAllBillboards();
    void handleFilterBillboards();
    void handleCreateBooking();
    void handleViewMyOrders();

    std::shared_ptr<User> client_;
    std::shared_ptr<service::BillboardService> billboardService_;
    std::shared_ptr<service::BookingService> bookingService_;
};

} // namespace ui
