#include "infrastructure/SqliteOrderRepository.hpp"
#include <iostream>

SqliteOrderRepository::SqliteOrderRepository(DatabaseManager& dbManager)
    : dbManager_(dbManager) {}

BookingOrder SqliteOrderRepository::mapRowToOrder(sqlite3_stmt* stmt) {
    int id = sqlite3_column_int(stmt, 0);
    int clientId = sqlite3_column_int(stmt, 1);
    int billboardId = sqlite3_column_int(stmt, 2);
    std::string startDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    std::string endDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    double totalPrice = sqlite3_column_double(stmt, 5);
    std::string statusStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

    DateRange period(startDate, endDate);
    OrderStatus status = orderStatusFromString(statusStr);

    return BookingOrder(id, clientId, billboardId, period, totalPrice, status);
}

std::optional<BookingOrder> SqliteOrderRepository::getById(int id) {
    sqlite3* db = dbManager_.getConnection();
    if (!db) return std::nullopt;

    const char* sql = "SELECT id, user_id, billboard_id, start_date, end_date, total_price, status "
                      "FROM orders WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки getById (order): " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);

    std::optional<BookingOrder> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = mapRowToOrder(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::vector<BookingOrder> SqliteOrderRepository::getAll() {
    std::vector<BookingOrder> orders;
    sqlite3* db = dbManager_.getConnection();
    if (!db) return orders;

    const char* sql = "SELECT id, user_id, billboard_id, start_date, end_date, total_price, status "
                      "FROM orders;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки getAll (orders): " << sqlite3_errmsg(db) << std::endl;
        return orders;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        orders.push_back(mapRowToOrder(stmt));
    }

    sqlite3_finalize(stmt);
    return orders;
}

std::vector<BookingOrder> SqliteOrderRepository::getByBillboardId(int billboardId) {
    std::vector<BookingOrder> orders;
    sqlite3* db = dbManager_.getConnection();
    if (!db) return orders;

    const char* sql = "SELECT id, user_id, billboard_id, start_date, end_date, total_price, status "
                      "FROM orders WHERE billboard_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки getByBillboardId (orders): " << sqlite3_errmsg(db) << std::endl;
        return orders;
    }

    sqlite3_bind_int(stmt, 1, billboardId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        orders.push_back(mapRowToOrder(stmt));
    }

    sqlite3_finalize(stmt);
    return orders;
}

std::vector<BookingOrder> SqliteOrderRepository::getByClientId(int clientId) {
    std::vector<BookingOrder> orders;
    sqlite3* db = dbManager_.getConnection();
    if (!db) return orders;

    const char* sql = "SELECT id, user_id, billboard_id, start_date, end_date, total_price, status "
                      "FROM orders WHERE user_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки getByClientId (orders): " << sqlite3_errmsg(db) << std::endl;
        return orders;
    }

    sqlite3_bind_int(stmt, 1, clientId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        orders.push_back(mapRowToOrder(stmt));
    }

    sqlite3_finalize(stmt);
    return orders;
}

bool SqliteOrderRepository::save(const BookingOrder& order) {
    sqlite3* db = dbManager_.getConnection();
    if (!db) return false;

    const char* sql = "INSERT INTO orders (user_id, billboard_id, start_date, end_date, total_price, status) "
                      "VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки save (order): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string statusStr = orderStatusToString(order.getStatus());

    sqlite3_bind_int(stmt, 1, order.getClientId());
    sqlite3_bind_int(stmt, 2, order.getBillboardId());
    sqlite3_bind_text(stmt, 3, order.getPeriod().getStart().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, order.getPeriod().getEnd().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, order.getTotalPrice());
    sqlite3_bind_text(stmt, 6, statusStr.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Ошибка вставки заказа: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}

bool SqliteOrderRepository::updateStatus(int orderId, OrderStatus newStatus) {
    sqlite3* db = dbManager_.getConnection();
    if (!db) return false;

    const char* sql = "UPDATE orders SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки updateStatus (order): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string statusStr = orderStatusToString(newStatus);

    sqlite3_bind_text(stmt, 1, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, orderId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Ошибка обновления статуса заказа: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}