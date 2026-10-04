#pragma once
#include <vector>
#include <optional>
#include "domain/BookingOrder.hpp"

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;
    virtual std::optional<BookingOrder> getById(int id) = 0;
    virtual std::vector<BookingOrder> getAll() = 0;
    virtual std::vector<BookingOrder> getByBillboardId(int billboardId) = 0;
    virtual std::vector<BookingOrder> getByClientId(int clientId) = 0;
    virtual bool save(const BookingOrder& order) = 0;
    virtual bool updateStatus(int orderId, OrderStatus newStatus) = 0;
};