#include "ui/ManagerView.hpp"
#include "ui/ClientView.hpp"
#include "ui/ConsoleApp.hpp"

#include <iostream>
#include <stdexcept>

namespace ui {

ManagerView::ManagerView(std::shared_ptr<User> manager,
                         std::shared_ptr<service::BillboardService> billboardService,
                         std::shared_ptr<service::BookingService> bookingService)
    : manager_(std::move(manager)),
      billboardService_(std::move(billboardService)),
      bookingService_(std::move(bookingService)) {}

void ManagerView::render() {
    bool inManagerSession = true;
    while (inManagerSession) {
        showMenu();
        int choice = ConsoleApp::readInt("Выберите действие: ", 0, 4);
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
            case 4:
                handleViewAllOrders();
                break;
            case 0:
                inManagerSession = false;
                break;
        }
    }
}

void ManagerView::showMenu() {
    std::cout << "\n============================================\n"
              << "   ПАНЕЛЬ МЕНЕДЖЕРА (" << manager_->getLogin() << ")\n"
              << "============================================\n"
              << "  1. Список заявок на рассмотрении (Pending)\n"
              << "  2. Одобрить или отклонить заявку\n"
              << "  3. Отчет по занятости и выручке\n"
              << "  4. Все заявки\n"
              << "  0. Выйти из аккаунта\n"
              << "--------------------------------------------\n";
}

void ManagerView::handleViewPendingOrders() {
    try {
        auto pendingOrders = bookingService_->getPendingOrders();
        if (pendingOrders.empty()) {
            std::cout << "\n  [i] Заявок, ожидающих рассмотрения, сейчас нет.\n";
            return;
        }
        std::cout << "\n--- ОЧЕРЕДЬ ЗАЯВОК НА БРОНИРОВАНИЕ ---";
        ClientView::displayOrders(pendingOrders, true);
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка получения заявок]: " << ex.what() << "\n";
    }
}

void ManagerView::handleViewAllOrders() {
    try {
        auto orders = bookingService_->getAllOrders();
        if (orders.empty()) {
            std::cout << "\n  [i] Заявок пока нет.\n";
            return;
        }
        ClientView::displayOrders(orders, true);
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка получения заявок]: " << ex.what() << "\n";
    }
}

void ManagerView::handleProcessOrderDecision() {
    handleViewPendingOrders();
    std::cout << "\n--- Модерация заявки ---\n";
    int orderId = ConsoleApp::readInt("Введите номер заказа (ID, 0 — назад): ", 0, 100000);
    if (orderId == 0) {
        return;
    }
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
            std::cout << "\n[ОК] Заявка #" << orderId
                      << " одобрена. Щит на выбранные даты заблокирован для других клиентов.\n";
        } else {
            bookingService_->rejectOrder(orderId);
            std::cout << "\n[ОК] Заявка #" << orderId << " ОТКЛОНЕНА.\n";
        }
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка обработки заявки]: " << ex.what() << "\n";
    }
}

void ManagerView::handleOccupancyReport() {
    std::cout << "\n--- Отчёт по занятости и выручке ---\n";
    std::string from = ConsoleApp::readDate("Начало периода отчёта (ДД.ММ.ГГГГ): ");
    std::string to = ConsoleApp::readDate("Конец периода отчёта (ДД.ММ.ГГГГ): ");

    try {
        DateRange period(from, to);
        auto report = bookingService_->getOccupancyReport(period);

        const std::string line =
            "+------+---------------------------+-----------------+---------+-----------+--------------+---------+\n";
        std::cout << "\n  Период: " << period.toString() << " (" << period.getDays() << " сут.)\n"
                  << line
                  << "|  ID  | Адрес                     | Район           | Занято  | Загрузка  | Выручка, BYN | Заявок  |\n"
                  << line;

        double totalRevenue = 0.0;
        int totalBooked = 0;
        int totalPending = 0;
        for (const auto& row : report) {
            std::cout << "| " << ConsoleApp::fit(std::to_string(row.billboard.getId()), 4, true)
                      << " | " << ConsoleApp::fit(row.billboard.getAddress(), 25)
                      << " | " << ConsoleApp::fit(row.billboard.getDistrict(), 15)
                      << " | " << ConsoleApp::fit(std::to_string(row.bookedDays) + " дн.", 7, true)
                      << " | " << ConsoleApp::fit(ConsoleApp::money(row.occupancyPercent) + " %", 9, true)
                      << " | " << ConsoleApp::fit(ConsoleApp::money(row.revenue), 12, true)
                      << " | " << ConsoleApp::fit(std::to_string(row.pendingOrders), 7, true) << " |\n";
            totalRevenue += row.revenue;
            totalBooked += row.bookedDays;
            totalPending += row.pendingOrders;
        }
        std::cout << line;

        const double capacity = static_cast<double>(report.size()) * period.getDays();
        std::cout << "  Всего конструкций     : " << report.size() << "\n"
                  << "  Средняя загрузка      : "
                  << ConsoleApp::money(capacity > 0 ? totalBooked * 100.0 / capacity : 0.0) << " %\n"
                  << "  Выручка за период     : " << ConsoleApp::money(totalRevenue) << " BYN\n"
                  << "  Заявок на рассмотрении: " << totalPending << "\n";
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка формирования отчета]: " << ex.what() << "\n";
    }
}

} // namespace ui
