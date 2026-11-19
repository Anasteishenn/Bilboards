#include "ui/ClientView.hpp"
#include "ui/ConsoleApp.hpp"
#include "domain/DateRange.hpp"

#include <iostream>
#include <optional>
#include <stdexcept>

namespace ui {

namespace {

std::string statusText(OrderStatus status) {
    switch (status) {
        case OrderStatus::Pending: return "Ожидает";
        case OrderStatus::Approved: return "Одобрена";
        case OrderStatus::Rejected: return "Отклонена";
    }
    return orderStatusToString(status);
}

} // namespace

ClientView::ClientView(std::shared_ptr<User> client,
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
              << "   ЛИЧНЫЙ КАБИНЕТ КЛИЕНТА (" << client_->getLogin() << ")\n"
              << "============================================\n"
              << "  1. Просмотреть весь каталог щитов\n"
              << "  2. Поиск и фильтрация щитов (район / тип / бюджет)\n"
              << "  3. Оформить заявку на бронирование\n"
              << "  4. Мои заявки и статусы договоров\n"
              << "  0. Выйти из аккаунта\n"
              << "--------------------------------------------\n";
}

void ClientView::displayCatalog(const std::vector<Billboard>& billboards, bool showStatus) {
    if (billboards.empty()) {
        std::cout << "\n  [i] По вашему запросу рекламных конструкций не найдено.\n";
        return;
    }

    const std::string line = showStatus
        ? "+------+---------------------------+-----------------+-------------+-------+--------------+-----------+\n"
        : "+------+---------------------------+-----------------+-------------+-------+--------------+\n";
    std::cout << '\n' << line
              << "|  ID  | Адрес                     | Район           | Тип экрана  | Размер| Цена (сутки) |"
              << (showStatus ? " Статус    |" : "") << '\n' << line;

    for (const auto& b : billboards) {
        std::cout << "| " << ConsoleApp::fit(std::to_string(b.getId()), 4, true)
                  << " | " << ConsoleApp::fit(b.getAddress(), 25)
                  << " | " << ConsoleApp::fit(b.getDistrict(), 15)
                  << " | " << ConsoleApp::fit(surfaceTypeToString(b.getSurfaceType()), 11)
                  << " | " << ConsoleApp::fit(b.getSize(), 5)
                  << " | " << ConsoleApp::fit(ConsoleApp::money(b.getDailyRate()), 8, true) << " BYN |";
        if (showStatus) {
            std::cout << ' ' << ConsoleApp::fit(b.isActive() ? "Активен" : "Списан", 9) << " |";
        }
        std::cout << '\n';
    }
    std::cout << line;
}

void ClientView::displayOrders(const std::vector<BookingOrder>& orders, bool withClient) {
    const std::string line = withClient
        ? "+--------+----------+----------+-------------------------+--------------+------------+\n"
        : "+--------+----------+-------------------------+--------------+------------+\n";
    std::cout << '\n' << line << "| Заказ  |" << (withClient ? " Клиент   |" : "")
              << " ID Щита  | Период аренды           | Сумма (BYN)  | Статус     |\n" << line;

    for (const auto& o : orders) {
        std::cout << "| #" << ConsoleApp::fit(std::to_string(o.getId()), 5) << " |";
        if (withClient) {
            std::cout << " " << ConsoleApp::fit("ID:" + std::to_string(o.getClientId()), 8) << " |";
        }
        std::cout << " " << ConsoleApp::fit(std::to_string(o.getBillboardId()), 8)
                  << " | " << ConsoleApp::fit(o.getPeriod().toString(), 23)
                  << " | " << ConsoleApp::fit(ConsoleApp::money(o.getTotalPrice()), 12, true)
                  << " | " << ConsoleApp::fit(statusText(o.getStatus()), 10) << " |\n";
    }
    std::cout << line;
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
    service::BillboardFilter filter;
    const std::string district = ConsoleApp::readLine("Введите район или его часть (или '-' для любого): ");
    filter.district = (district == "-") ? "" : district;

    std::cout << "  Тип экрана: 1. Static  2. Digital  3. Prismatron  0. Любой\n";
    switch (ConsoleApp::readInt("Ваш выбор: ", 0, 3)) {
        case 1: filter.surfaceType = SurfaceType::Static; break;
        case 2: filter.surfaceType = SurfaceType::Digital; break;
        case 3: filter.surfaceType = SurfaceType::Prismatron; break;
        default: break;
    }
    filter.maxDailyRate = ConsoleApp::readDouble("Максимальная цена за сутки, BYN (0 — без ограничения): ", 0.0);

    try {
        displayCatalog(billboardService_->filterBillboards(filter));
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка фильтрации]: " << ex.what() << "\n";
    }
}

void ClientView::handleCreateBooking() {
    std::cout << "\n--- Оформление заявки на аренду ---\n";
    int billboardId = ConsoleApp::readInt("Введите ID рекламного щита: ", 1, 100000);
    std::string startDate = ConsoleApp::readDate("Дата начала аренды (ДД.ММ.ГГГГ): ");
    std::string endDate = ConsoleApp::readDate("Дата окончания аренды (ДД.ММ.ГГГГ): ");

    service::PriceQuote quote;
    std::optional<DateRange> period;
    try {
        period.emplace(startDate, endDate);
        quote = bookingService_->calculatePrice(client_->getId(), billboardId, *period);
    } catch (const std::exception& ex) {
        std::cout << "\n[Отклонено]: " << ex.what() << "\n";
        return;
    }

    std::cout << "\n[Расчёт]:\n"
              << "  - Период: " << period->toString() << "\n"
              << "  - Длительность: " << quote.days << " сут.\n"
              << "  - Базовая стоимость: " << ConsoleApp::money(quote.baseCost) << " BYN\n";
    if (quote.discountAmount > 0.0) {
        std::cout << "  - " << quote.discountName << " (" << quote.discountPercent << "%): -"
                  << ConsoleApp::money(quote.discountAmount) << " BYN\n";
    }
    std::cout << "  - ИТОГО К ОПЛАТЕ: " << ConsoleApp::money(quote.total) << " BYN\n\n";

    if (!ConsoleApp::confirm("Подтвердить создание заявки?")) {
        std::cout << "\n[i] Заявка не создана.\n";
        return;
    }

    try {
        auto order = bookingService_->createBooking(client_->getId(), billboardId, *period);
        std::cout << "\n[ОК] Заявка #" << order.getId() << " успешно создана и отправлена менеджеру!\n"
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
        displayOrders(orders, false);
    } catch (const std::exception& ex) {
        std::cout << "\n[Ошибка получения списка заказов]: " << ex.what() << "\n";
    }
}

} // namespace ui
