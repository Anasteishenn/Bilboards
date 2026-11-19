#pragma once
#include <memory>
#include <optional>
#include <vector>

#include "domain/Billboard.hpp"
#include "domain/BookingOrder.hpp"
#include "domain/DateRange.hpp"
#include "repository/IBillboardRepository.hpp"
#include "repository/IOrderRepository.hpp"
#include "service/IPricingStrategy.hpp"

namespace service {

// Строка отчёта менеджера по одному щиту за выбранный период.
struct OccupancyReportRow {
    Billboard billboard;
    int bookedDays{0};          // дни, занятые одобренными заказами внутри периода отчёта
    double occupancyPercent{0}; // bookedDays / дней в периоде * 100
    double revenue{0};          // сумма одобренных заказов, пересекающих период отчёта
    int pendingOrders{0};       // заявок на рассмотрении по этому щиту
};

// Бронирование: расчёт цены, проверка занятости дат, модерация и аналитика.
// Даты щита блокируют только одобренные (Approved) заказы.
class BookingService {
public:
    // По умолчанию используется BestDiscountPricing (лучшая из скидок за длительность и постоянного клиента).
    BookingService(std::shared_ptr<IOrderRepository> orderRepository,
                   std::shared_ptr<IBillboardRepository> billboardRepository,
                   std::shared_ptr<IPricingStrategy> pricingStrategy = nullptr);

    // Strategy можно заменить во время работы, BookingService при этом не меняется.
    void setPricingStrategy(std::shared_ptr<IPricingStrategy> strategy);
    const IPricingStrategy& getPricingStrategy() const { return *pricingStrategy; }

    // Предварительный расчёт без создания заявки (блок "Расчёт" перед подтверждением).
    // Бросает std::invalid_argument, если щит не найден/списан, std::runtime_error — если даты заняты.
    PriceQuote calculatePrice(int clientId, int billboardId, const DateRange& period) const;

    // Одобренный заказ на этот щит, пересекающийся с периодом, либо nullopt.
    std::optional<BookingOrder> findConflict(int billboardId, const DateRange& period, int ignoreOrderId = 0) const;
    bool isAvailable(int billboardId, const DateRange& period) const;

    // Создаёт заявку в статусе Pending. Исключения — как у calculatePrice.
    BookingOrder createBooking(int clientId, int billboardId, const DateRange& period);

    std::vector<BookingOrder> getClientOrders(int clientId) const;
    std::vector<BookingOrder> getPendingOrders() const;
    std::vector<BookingOrder> getAllOrders() const;

    // Модерация. Бросают std::invalid_argument (заказ не найден), std::logic_error (не Pending),
    // std::runtime_error (даты уже заняты другим одобренным заказом или ошибка сохранения).
    void approveOrder(int orderId);
    void rejectOrder(int orderId);

    // Отчёт по занятости и выручке всех щитов за период.
    std::vector<OccupancyReportRow> getOccupancyReport(const DateRange& reportPeriod) const;

private:
    std::shared_ptr<IOrderRepository> orderRepository;
    std::shared_ptr<IBillboardRepository> billboardRepository;
    std::shared_ptr<IPricingStrategy> pricingStrategy;

    Billboard requireActiveBillboard(int billboardId) const;
    BookingOrder requireOrder(int orderId) const;
    void ensureAvailable(int billboardId, const DateRange& period, int ignoreOrderId = 0) const;
    int countApprovedOrders(int clientId) const;
};

} // namespace service
