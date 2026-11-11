#include "domain/BookingOrder.hpp"

#include <stdexcept>

std::string orderStatusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::Pending: return "Pending";
        case OrderStatus::Approved: return "Approved";
        case OrderStatus::Rejected: return "Rejected";
    }
    return "Pending";
}

OrderStatus orderStatusFromString(const std::string& value) {
    if (value == "Pending") return OrderStatus::Pending;
    if (value == "Approved") return OrderStatus::Approved;
    if (value == "Rejected") return OrderStatus::Rejected;
    throw std::invalid_argument("Неизвестный статус заказа: " + value);
}

BookingOrder::BookingOrder(int id, int clientId, int billboardId, DateRange period, double totalPrice,
                           OrderStatus status)
    : id(id), clientId(clientId), billboardId(billboardId), period(std::move(period)),
      totalPrice(totalPrice), status(status) {
    if (totalPrice < 0.0) {
        throw std::invalid_argument("Стоимость заказа не может быть отрицательной");
    }
}

void BookingOrder::approve() {
    if (!isPending()) {
        throw std::logic_error("Одобрить можно только заявку в статусе Pending");
    }
    status = OrderStatus::Approved;
}

void BookingOrder::reject() {
    if (!isPending()) {
        throw std::logic_error("Отклонить можно только заявку в статусе Pending");
    }
    status = OrderStatus::Rejected;
}
