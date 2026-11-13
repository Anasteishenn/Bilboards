#include "ui/ClientView.hpp"
#include "ui/ConsoleApp.hpp"
#include "domain/DateRange.hpp"

#include <iostream>
#include <iomanip>
#include <stdexcept>

namespace ui {

ClientView::ClientView(std::shared_ptr<domain::User> client,
                       std::shared_ptr<service::BillboardService> billboardService,
                       std::shared_ptr<service::BookingService> bookingService)
    : client_(std::move(client)),
      billboardService_(std::move(billboardService)),
      bookingService_(std::move(bookingService)) {}

void ClientView::render() {
    bool inClientSession = true;
    while (inClientSession) {
        showMenu();
        int choice = ConsoleApp::readInt("Выберите пункт меню: ", 0, 4);
        switch (choice) {
            case 1:
                handleViewAllBillboards();
                break;
            case 2:
                handleFilterBillboards();
                break;
            case 3:
                handleCreateBooking();
                break;
            case 4:
                handleViewMyOrders();
                break;
            case 0:
                inClientSession = false;
                break;
        }
    }
}

void ClientView::showMenu() {
    std::cout << "\n============================================\n"
              << "   ЛИЧНЫЙ КАБИНЕТ КЛИЕНТА (" << client_->getUsername() << ")\n"
              << "============================================\n"
              << "  1. Просмотреть весь каталог щитов\n"
              << "  2. Поиск и фильтрация щитов (район / бюджет)\n"
              << "  3. Оформить заявку на бронирование\n"
              << "  4. Мои заявки и статусы договоров\n"
              << "  0. Выйти из аккаунта\n"
              << "--------------------------------------------\n";
}

void ClientView::displayCatalog(const std::vector<domain::Billboard>& billboards) {
    if (billboards.empty()) {
        std::cout << "\n  [i] По вашему запросу рекламных конструкций не найдено.\n";
        return;
    }

    std::cout << "\n+------+---------------------------+-----------------+-------------+--------------+\n"
              << "|  ID  | Адрес                     | Район           | Тип экрана  | Цена (сутки) |\n"
              << "+------+---------------------------+-----------------+-------------+--------------+\n";

    for (const auto& b : billboards) {
        std::cout << "| " << std::setw(4) << b.getId()
                  << " | " << std::setw(25) << std::left << b.getAddress().substr(0, 25)
                  << " | " << std::setw(15) << std::left << b.getDistrict().substr(0, 15)
                  << " | " << std::setw(11) << std::left << b.getTypeString().substr(0, 11)
                  << " | " << std::setw(8) << std::right << std::fixed << std::setprecision(2) << b.getDailyPrice() << " BYN |\n";
    }
    std::cout << "+------+---------------------------+-----------------+-------------+--------------+\n";
}

void ClientView::handleViewAllBillboards() {
    try {
        auto billboards = billboardService_->getAllBillboards();
        displayCatalog(billboards);
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка загрузки каталога]: " << ex.what() << "\n";
    }
}

void ClientView::handleFilterBillboards() {
    std::cout << "\n--- Фильтрация каталога ---\n";
    std::string district = ConsoleApp::readLine("Введите район (или '-' для любого): ");
    double maxPrice = ConsoleApp::readDouble("Максимальная цена за сутки (BYN): ", 0.0);

    try {
        std::string districtFilter = (district == "-") ? "" : district;
        auto filtered = billboardService_->filterBillboards(districtFilter, maxPrice);
        displayCatalog(filtered);
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка фильтрации]: " << ex.what() << "\n";
    }
}

void ClientView::handleCreateBooking() {
    std::cout << "\n--- Оформление заявки на аренду ---\n";
    int billboardId = ConsoleApp::readInt("Введите ID рекламного щита: ", 1, 100000);
    std::string startDate = ConsoleApp::readLine("Дата начала аренды (ГГГГ-ММ-ДД): ");
    std::string endDate = ConsoleApp::readLine("Дата окончания аренды (ГГГГ-ММ-ДД): ");

    try {
        domain::DateRange period(startDate, endDate);
        auto order = bookingService_->createBooking(client_->getId(), billboardId, period);
        std::cout << "\n[ОК] Заявка успешно создана!\n"
                  << "  Номер договора: #" << order.getId() << "\n"
                  << "  Продолжительность: " << period.getTotalDays() << " сут.\n"
                  << "  Итоговая стоимость: " << std::fixed << std::setprecision(2) << order.getTotalCost() << " BYN\n"
                  << "  Статус: Ожидает подтверждения менеджером\n";
    } catch (const std::exception& ex) {
        std::cout << "\n[Отклонено]: " << ex.what() << "\n";
    }
}

void ClientView::handleViewMyOrders() {
    try {
        auto orders = bookingService_->getClientOrders(client_->getId());
        if (orders.empty()) {
            std::cout << "\n  [i] У вас пока нет поданных заявок на бронирование.\n";
            return;
        }

        std::cout << "\n+--------+----------+-------------------------+--------------+----------------+\n"
                  << "| Заказ  | ID Щита  | Период аренды           | Итого (BYN)  | Статус         |\n"
                  << "+--------+----------+-------------------------+--------------+----------------+\n";

        for (const auto& o : orders) {
            std::string periodStr = o.getDateRange().getStartDate() + " - " + o.getDateRange().getEndDate();
            std::cout << "| #" << std::setw(5) << std::left << o.getId()
                      << " | " << std::setw(8) << o.getBillboardId()
                      << " | " << std::setw(23) << periodStr
                      << " | " << std::setw(12) << std::right << std::fixed << std::setprecision(2) << o.getTotalCost()
                      << " | " << std::setw(14) << std::left << o.getStatusString() << " |\n";
        }
        std::cout << "+--------+----------+-------------------------+--------------+----------------+\n";
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка получения списка заказов]: " << ex.what() << "\n";
    }
}

} // namespace ui