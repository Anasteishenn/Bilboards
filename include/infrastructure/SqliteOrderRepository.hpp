#pragma once

#include <vector>
#include <optional>
#include "repository/IOrderRepository.hpp"
#include "repository/DatabaseManager.hpp"
#include "sqlite3.h"

class SqliteOrderRepository : public IOrderRepository {
public:
    explicit SqliteOrderRepository(DatabaseManager& dbManager);
    ~SqliteOrderRepository() override = default;

    std::optional<BookingOrder> getById(int id) override;
    std::vector<BookingOrder> getAll() override;
    std::vector<BookingOrder> getByBillboardId(int billboardId) override;
    std::vector<BookingOrder> getByClientId(int clientId) override;
    bool save(const BookingOrder& order) override;
    bool updateStatus(int orderId, OrderStatus newStatus) override;

private:
    DatabaseManager& dbManager_;

    static BookingOrder mapRowToOrder(sqlite3_stmt* stmt);
};