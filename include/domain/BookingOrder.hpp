#pragma once
#include <string>

#include "domain/DateRange.hpp"

enum class OrderStatus {
    Pending,
    Approved,
    Rejected
};

// Преобразование для хранения в БД (столбец orders.status) и вывода на экран.
std::string orderStatusToString(OrderStatus status);
// Бросает std::invalid_argument для неизвестного значения.
OrderStatus orderStatusFromString(const std::string& value);

class BookingOrder {
private:
    int id{0};
    int clientId{0};
    int billboardId{0};
    DateRange period;
    double totalPrice{0.0};
    OrderStatus status{OrderStatus::Pending};

public:
    // Бросает std::invalid_argument, если стоимость отрицательная.
    BookingOrder(int id, int clientId, int billboardId, DateRange period, double totalPrice,
                 OrderStatus status = OrderStatus::Pending);

    int getId() const { return id; }
    int getClientId() const { return clientId; }
    int getBillboardId() const { return billboardId; }
    const DateRange& getPeriod() const { return period; }
    double getTotalPrice() const { return totalPrice; }
    OrderStatus getStatus() const { return status; }

    bool isPending() const { return status == OrderStatus::Pending; }
    // Одобренный заказ блокирует даты щита для других клиентов.
    bool blocksDates() const { return status == OrderStatus::Approved; }

    // Модерация возможна только для заявок в статусе Pending, иначе std::logic_error.
    void approve();
    void reject();
};
