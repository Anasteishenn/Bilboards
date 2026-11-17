# 1. Создаем include/ui/ManagerView.hpp
@"
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
    ManagerView(std::shared_ptr<domain::User> manager,
                std::shared_ptr<service::BillboardService> billboardService,
                std::shared_ptr<service::BookingService> bookingService);

    void render();

private:
    void showMenu();
    void displayOrdersTable(const std::vector<domain::BookingOrder>& orders);
    void handleViewPendingOrders();
    void handleProcessOrderDecision();
    void handleOccupancyReport();

    std::shared_ptr<domain::User> manager_;
    std::shared_ptr<service::BillboardService> billboardService_;
    std::shared_ptr<service::BookingService> bookingService_;
};

} // namespace ui
"@ | Set-Content -Path "include/ui/ManagerView.hpp" -Encoding UTF8

# 2. Создаем src/ui/ManagerView.cpp
@"
#include "ui/ManagerView.hpp"
#include "ui/ConsoleApp.hpp"

#include <iostream>
#include <iomanip>
#include <stdexcept>

namespace ui {

ManagerView::ManagerView(std::shared_ptr<domain::User> manager,
                         std::shared_ptr<service::BillboardService> billboardService,
                         std::shared_ptr<service::BookingService> bookingService)
    : manager_(std::move(manager)),
      billboardService_(std::move(billboardService)),
      bookingService_(std::move(bookingService)) {}

void ManagerView::render() {
    bool inManagerSession = true;
    while (inManagerSession) {
        showMenu();
        int choice = ConsoleApp::readInt("Выберите действие: ", 0, 3);
        switch (choice) {
            case 1:
                handleViewPendingOrders();
                break;
            case 2:
                handleProcessOrderDecision();
                break;
            case 3:
                handleOccupancyReport();
                break;
            case 0:
                inManagerSession = false;
                break;
        }
    }
}

void ManagerView::showMenu() {
    std::cout << "\n============================================\n"
              << "   ПАНЕЛЬ МЕНЕДЖЕРА (" << manager_->getUsername() << ")\n"
              << "============================================\n"
              << "  1. Список заявок на рассмотрении (Pending)\n"
              << "  2. Одобрить или отклонить заявку\n"
              << "  3. Отчет по конструкциям и занятости\n"
              << "  0. Выйти из аккаунта\n"
              << "--------------------------------------------\n";
}

void ManagerView::displayOrdersTable(const std::vector<domain::BookingOrder>& orders) {
    if (orders.empty()) {
        std::cout << "\n  [i] Заявок, ожидающих рассмотрения, сейчас нет.\n";
        return;
    }

    std::cout << "\n+--------+----------+----------+-------------------------+--------------+----------------+\n"
              << "| Заказ  | Клиент   | ID Щита  | Период аренды           | Сумма (BYN)  | Статус         |\n"
              << "+--------+----------+----------+-------------------------+--------------+----------------+\n";

    for (const auto& o : orders) {
        std::string periodStr = o.getDateRange().getStartDate() + " - " + o.getDateRange().getEndDate();
        std::cout << "| #" << std::setw(5) << std::left << o.getId()
                  << " | ID:" << std::setw(5) << std::left << o.getClientId()
                  << " | " << std::setw(8) << o.getBillboardId()
                  << " | " << std::setw(23) << periodStr
                  << " | " << std::setw(12) << std::right << std::fixed << std::setprecision(2) << o.getTotalCost()
                  << " | " << std::setw(14) << std::left << o.getStatusString() << " |\n";
    }
    std::cout << "+--------+----------+----------+-------------------------+--------------+----------------+\n";
}

void ManagerView::handleViewPendingOrders() {
    try {
        auto pendingOrders = bookingService_->getPendingOrders();
        displayOrdersTable(pendingOrders);
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка получения заявок]: " << ex.what() << "\n";
    }
}

void ManagerView::handleProcessOrderDecision() {
    std::cout << "\n--- Модерация заявки ---\n";
    int orderId = ConsoleApp::readInt("Введите номер заказа (ID): ", 1, 100000);
    std::cout << "  1. Одобрить заявку (Approve)\n"
              << "  2. Отклонить заявку (Reject)\n"
              << "  0. Отмена\n";
    int decision = ConsoleApp::readInt("Ваше решение: ", 0, 2);

    if (decision == 0) {
        return;
    }

    try {
        if (decision == 1) {
            bookingService_->approveOrder(orderId);
            std::cout << "\n[ОК] Заявка #" << orderId << " успешно ОДОБРЕНА.\n";
        } else {
            bookingService_->rejectOrder(orderId);
            std::cout << "\n[ОК] Заявка #" << orderId << " ОТКЛОНЕНА.\n";
        }
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка обработки заявки]: " << ex.what() << "\n";
    }
}

void ManagerView::handleOccupancyReport() {
    std::cout << "\n========================================================================\n"
              << "              СВОДНЫЙ ОТЧЕТ ПО РЕКЛАМНЫМ КОНСТРУКЦИЯМ                   \n"
              << "========================================================================\n";

    try {
        auto billboards = billboardService_->getAllBillboards();
        auto pendingOrders = bookingService_->getPendingOrders();

        std::cout << "Всего конструкций в базе : " << billboards.size() << "\n"
                  << "Заявок на рассмотрении   : " << pendingOrders.size() << "\n"
                  << "------------------------------------------------------------------------\n"
                  << "|  ID  | Район           | Адрес                     | Ставка (BYN/сут) |\n"
                  << "------------------------------------------------------------------------\n";

        for (const auto& b : billboards) {
            std::cout << "| " << std::setw(4) << b.getId()
                      << " | " << std::setw(15) << std::left << b.getDistrict().substr(0, 15)
                      << " | " << std::setw(25) << std::left << b.getAddress().substr(0, 25)
                      << " | " << std::setw(16) << std::right << std::fixed << std::setprecision(2) << b.getDailyPrice() << " |\n";
        }
        std::cout << "========================================================================\n";
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка формирования отчета]: " << ex.what() << "\n";
    }
}

} // namespace ui
"@ | Set-Content -Path "src/ui/ManagerView.cpp" -Encoding UTF8