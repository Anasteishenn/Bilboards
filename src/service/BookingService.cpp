#include "service/BookingService.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

#include "service/PricingStrategies.hpp"

namespace service {

BookingService::BookingService(std::shared_ptr<IOrderRepository> orderRepository,
                               std::shared_ptr<IBillboardRepository> billboardRepository,
                               std::shared_ptr<IPricingStrategy> pricingStrategy)
    : orderRepository(std::move(orderRepository)),
      billboardRepository(std::move(billboardRepository)),
      pricingStrategy(pricingStrategy ? std::move(pricingStrategy) : std::make_shared<BestDiscountPricing>()) {
    if (!this->orderRepository || !this->billboardRepository) {
        throw std::invalid_argument("BookingService: не переданы репозитории");
    }
}

void BookingService::setPricingStrategy(std::shared_ptr<IPricingStrategy> strategy) {
    if (!strategy) {
        throw std::invalid_argument("Стратегия расчёта цены не может быть пустой");
    }
    pricingStrategy = std::move(strategy);
}

PriceQuote BookingService::calculatePrice(int clientId, int billboardId, const DateRange& period) const {
    const Billboard billboard = requireActiveBillboard(billboardId);
    ensureAvailable(billboardId, period);
    return pricingStrategy->calculate({billboard.getDailyRate(), period, countApprovedOrders(clientId)});
}

std::optional<BookingOrder> BookingService::findConflict(int billboardId, const DateRange& period,
                                                         int ignoreOrderId) const {
    for (const auto& order : orderRepository->getByBillboardId(billboardId)) {
        if (order.getId() != ignoreOrderId && order.blocksDates() && order.getPeriod().overlaps(period)) {
            return order;
        }
    }
    return std::nullopt;
}

bool BookingService::isAvailable(int billboardId, const DateRange& period) const {
    return !findConflict(billboardId, period);
}

BookingOrder BookingService::createBooking(int clientId, int billboardId, const DateRange& period) {
    const PriceQuote quote = calculatePrice(clientId, billboardId, period);

    const BookingOrder newOrder(0, clientId, billboardId, period, quote.total, OrderStatus::Pending);
    if (!orderRepository->save(newOrder)) {
        throw std::runtime_error("Не удалось сохранить заявку");
    }

    // id назначает база данных: берём самую новую заявку клиента на этот щит и период.
    std::optional<BookingOrder> saved;
    for (const auto& order : orderRepository->getByClientId(clientId)) {
        if (order.getBillboardId() == billboardId && order.getPeriod().getStart() == period.getStart() &&
            order.getPeriod().getEnd() == period.getEnd() && (!saved || order.getId() > saved->getId())) {
            saved = order;
        }
    }
    return saved ? *saved : newOrder;
}

std::vector<BookingOrder> BookingService::getClientOrders(int clientId) const {
    return orderRepository->getByClientId(clientId);
}

std::vector<BookingOrder> BookingService::getPendingOrders() const {
    std::vector<BookingOrder> pending;
    for (const auto& order : orderRepository->getAll()) {
        if (order.isPending()) {
            pending.push_back(order);
        }
    }
    return pending;
}

std::vector<BookingOrder> BookingService::getAllOrders() const {
    return orderRepository->getAll();
}

void BookingService::approveOrder(int orderId) {
    BookingOrder order = requireOrder(orderId);
    // Пока заявка ждала, те же даты могли одобрить другому клиенту.
    ensureAvailable(order.getBillboardId(), order.getPeriod(), orderId);
    order.approve();
    if (!orderRepository->updateStatus(orderId, order.getStatus())) {
        throw std::runtime_error("Не удалось обновить статус заявки #" + std::to_string(orderId));
    }
}

void BookingService::rejectOrder(int orderId) {
    BookingOrder order = requireOrder(orderId);
    order.reject();
    if (!orderRepository->updateStatus(orderId, order.getStatus())) {
        throw std::runtime_error("Не удалось обновить статус заявки #" + std::to_string(orderId));
    }
}

std::vector<OccupancyReportRow> BookingService::getOccupancyReport(const DateRange& reportPeriod) const {
    std::vector<OccupancyReportRow> report;
    for (const auto& billboard : billboardRepository->getAll()) {
        OccupancyReportRow row{billboard};
        for (const auto& order : orderRepository->getByBillboardId(billboard.getId())) {
            if (order.isPending()) {
                ++row.pendingOrders;
            }
            if (!order.blocksDates()) continue;
            if (const auto common = order.getPeriod().intersection(reportPeriod)) {
                row.bookedDays += common->getDays();
                row.revenue += order.getTotalPrice();
            }
        }
        row.occupancyPercent = std::round(row.bookedDays * 1000.0 / reportPeriod.getDays()) / 10.0;
        report.push_back(row);
    }
    return report;
}

Billboard BookingService::requireActiveBillboard(int billboardId) const {
    const auto billboard = billboardRepository->getById(billboardId);
    if (!billboard) {
        throw std::invalid_argument("Рекламный щит #" + std::to_string(billboardId) + " не найден");
    }
    if (!billboard->isActive()) {
        throw std::invalid_argument("Рекламный щит #" + std::to_string(billboardId) + " списан и недоступен для аренды");
    }
    return *billboard;
}

BookingOrder BookingService::requireOrder(int orderId) const {
    const auto order = orderRepository->getById(orderId);
    if (!order) {
        throw std::invalid_argument("Заявка #" + std::to_string(orderId) + " не найдена");
    }
    return *order;
}

void BookingService::ensureAvailable(int billboardId, const DateRange& period, int ignoreOrderId) const {
    if (const auto conflict = findConflict(billboardId, period, ignoreOrderId)) {
        throw std::runtime_error("Щит #" + std::to_string(billboardId) + " уже занят с " +
                                 DateRange::toDisplayDate(conflict->getPeriod().getStart()) + " по " +
                                 DateRange::toDisplayDate(conflict->getPeriod().getEnd()));
    }
}

int BookingService::countApprovedOrders(int clientId) const {
    const auto orders = orderRepository->getByClientId(clientId);
    return static_cast<int>(std::count_if(orders.begin(), orders.end(),
                                          [](const BookingOrder& order) { return order.blocksDates(); }));
}

} // namespace service
