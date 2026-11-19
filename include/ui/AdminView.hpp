#pragma once

#include <memory>
#include "domain/User.hpp"
#include "service/BillboardService.hpp"
#include "service/BookingService.hpp"

namespace ui {

// Управление инвентарём: добавление, редактирование и списание щитов,
// плюс доступ к функциям менеджера.
class AdminView {
public:
    AdminView(std::shared_ptr<User> admin,
              std::shared_ptr<service::BillboardService> billboardService,
              std::shared_ptr<service::BookingService> bookingService);

    void render();

private:
    void showMenu();
    void handleViewInventory();
    void handleAddBillboard();
    void handleEditBillboard();
    void handleDeactivateBillboard();

    static SurfaceType readSurfaceType(const std::string& prompt);

    std::shared_ptr<User> admin_;
    std::shared_ptr<service::BillboardService> billboardService_;
    std::shared_ptr<service::BookingService> bookingService_;
};

} // namespace ui
