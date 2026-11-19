#include "ui/AdminView.hpp"
#include "ui/ClientView.hpp"
#include "ui/ConsoleApp.hpp"
#include "ui/ManagerView.hpp"

#include <iostream>
#include <stdexcept>

namespace ui {

AdminView::AdminView(std::shared_ptr<User> admin,
                     std::shared_ptr<service::BillboardService> billboardService,
                     std::shared_ptr<service::BookingService> bookingService)
    : admin_(std::move(admin)),
      billboardService_(std::move(billboardService)),
      bookingService_(std::move(bookingService)) {}

void AdminView::render() {
    ManagerView managerView(admin_, billboardService_, bookingService_);
    bool inAdminSession = true;
    while (inAdminSession) {
        showMenu();
        int choice = ConsoleApp::readInt("Выберите действие: ", 0, 7);
        switch (choice) {
            case 1: handleViewInventory(); break;
            case 2: handleAddBillboard(); break;
            case 3: handleEditBillboard(); break;
            case 4: handleDeactivateBillboard(); break;
            case 5: managerView.handleViewPendingOrders(); break;
            case 6: managerView.handleProcessOrderDecision(); break;
            case 7: managerView.handleOccupancyReport(); break;
            case 0: inAdminSession = false; break;
        }
    }
}

void AdminView::showMenu() {
    std::cout << "\n============================================\n"
              << "   ПАНЕЛЬ АДМИНИСТРАТОРА (" << admin_->getLogin() << ")\n"
              << "============================================\n"
              << "  1. Весь инвентарь (включая списанные)\n"
              << "  2. Добавить рекламную конструкцию\n"
              << "  3. Редактировать конструкцию\n"
              << "  4. Списать (деактивировать) конструкцию\n"
              << "  --- функции менеджера ---\n"
              << "  5. Заявки на рассмотрении\n"
              << "  6. Одобрить или отклонить заявку\n"
              << "  7. Отчет по занятости и выручке\n"
              << "  0. Выйти из аккаунта\n"
              << "--------------------------------------------\n";
}

SurfaceType AdminView::readSurfaceType(const std::string& prompt) {
    std::cout << "  Тип экрана: 1. Static  2. Digital  3. Prismatron\n";
    switch (ConsoleApp::readInt(prompt, 1, 3)) {
        case 2: return SurfaceType::Digital;
        case 3: return SurfaceType::Prismatron;
        default: return SurfaceType::Static;
    }
}

void AdminView::handleViewInventory() {
    try {
        ClientView::displayCatalog(billboardService_->getAllBillboardsIncludingInactive(), true);
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка загрузки инвентаря]: " << ex.what() << "\n";
    }
}

void AdminView::handleAddBillboard() {
    std::cout << "\n--- Новая рекламная конструкция ---\n";
    std::string address = ConsoleApp::readLine("Адрес: ");
    std::string district = ConsoleApp::readLine("Район: ");
    SurfaceType type = readSurfaceType("Ваш выбор: ");
    std::string size = ConsoleApp::readLine("Размер (например, 6x3): ");
    double rate = ConsoleApp::readDouble("Базовая ставка за сутки, BYN: ", 0.01);

    try {
        billboardService_->addBillboard(Billboard(0, address, district, type, size, rate));
        std::cout << "\n[ОК] Конструкция добавлена в каталог.\n";
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка добавления]: " << ex.what() << "\n";
    }
}

void AdminView::handleEditBillboard() {
    std::cout << "\n--- Редактирование конструкции ---\n";
    int id = ConsoleApp::readInt("ID конструкции: ", 1, 100000);
    auto existing = billboardService_->getBillboard(id);
    if (!existing) {
        std::cout << "\n[!] Конструкция #" << id << " не найдена.\n";
        return;
    }
    Billboard billboard = *existing;
    std::cout << "  Пустой ввод (Enter) оставляет текущее значение.\n";

    try {
        std::string value = ConsoleApp::readLine("Адрес [" + billboard.getAddress() + "]: ", true);
        if (!value.empty()) billboard.setAddress(value);

        value = ConsoleApp::readLine("Район [" + billboard.getDistrict() + "]: ", true);
        if (!value.empty()) billboard.setDistrict(value);

        if (ConsoleApp::confirm("Изменить тип экрана [" + surfaceTypeToString(billboard.getSurfaceType()) + "]?")) {
            billboard.setSurfaceType(readSurfaceType("Новый тип: "));
        }

        value = ConsoleApp::readLine("Размер [" + billboard.getSize() + "]: ", true);
        if (!value.empty()) billboard.setSize(value);

        if (ConsoleApp::confirm("Изменить ставку [" + ConsoleApp::money(billboard.getDailyRate()) + " BYN]?")) {
            billboard.setDailyRate(ConsoleApp::readDouble("Новая ставка за сутки, BYN: ", 0.01));
        }

        billboardService_->updateBillboard(billboard);
        std::cout << "\n[ОК] Конструкция #" << id << " обновлена.\n";
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка редактирования]: " << ex.what() << "\n";
    }
}

void AdminView::handleDeactivateBillboard() {
    std::cout << "\n--- Списание конструкции ---\n";
    int id = ConsoleApp::readInt("ID конструкции: ", 1, 100000);
    if (!ConsoleApp::confirm("Списать конструкцию #" + std::to_string(id) + "? Она пропадёт из каталога")) {
        return;
    }
    try {
        billboardService_->deactivateBillboard(id);
        std::cout << "\n[ОК] Конструкция #" << id << " списана.\n";
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка списания]: " << ex.what() << "\n";
    }
}

} // namespace ui
