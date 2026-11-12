#include "infrastructure/SqliteBillboardRepository.hpp"
#include <iostream>

SqliteBillboardRepository::SqliteBillboardRepository(DatabaseManager& dbManager)
    : dbManager_(dbManager) {}

Billboard SqliteBillboardRepository::mapRowToBillboard(sqlite3_stmt* stmt) {
    int id = sqlite3_column_int(stmt, 0);
    std::string address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string district = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    std::string surfaceStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    std::string size = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    double dailyRate = sqlite3_column_double(stmt, 5);
    bool active = (sqlite3_column_int(stmt, 6) != 0);

    SurfaceType type = surfaceTypeFromString(surfaceStr);

    return Billboard(id, address, district, type, size, dailyRate, active);
}

std::optional<Billboard> SqliteBillboardRepository::getById(int id) {
    sqlite3* db = dbManager_.getConnection();
    if (!db) return std::nullopt;

    const char* sql = "SELECT id, address, district, surface_type, size, daily_rate, is_active "
                      "FROM billboards WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки getById (billboard): " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);

    std::optional<Billboard> result = std::nullopt;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = mapRowToBillboard(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::vector<Billboard> SqliteBillboardRepository::getAll() {
    std::vector<Billboard> billboards;
    sqlite3* db = dbManager_.getConnection();
    if (!db) return billboards;

    const char* sql = "SELECT id, address, district, surface_type, size, daily_rate, is_active "
                      "FROM billboards;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки getAll (billboards): " << sqlite3_errmsg(db) << std::endl;
        return billboards;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        billboards.push_back(mapRowToBillboard(stmt));
    }

    sqlite3_finalize(stmt);
    return billboards;
}

bool SqliteBillboardRepository::save(const Billboard& billboard) {
    sqlite3* db = dbManager_.getConnection();
    if (!db) return false;

    const char* sql = "INSERT INTO billboards (address, district, surface_type, size, daily_rate, is_active) "
                      "VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки save (billboard): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string surfaceStr = surfaceTypeToString(billboard.getSurfaceType());
    int activeInt = billboard.isActive() ? 1 : 0;

    sqlite3_bind_text(stmt, 1, billboard.getAddress().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, billboard.getDistrict().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, surfaceStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, billboard.getSize().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, billboard.getDailyRate());
    sqlite3_bind_int(stmt, 6, activeInt);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Ошибка вставки билборда: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}

bool SqliteBillboardRepository::update(const Billboard& billboard) {
    sqlite3* db = dbManager_.getConnection();
    if (!db) return false;

    const char* sql = "UPDATE billboards SET address = ?, district = ?, surface_type = ?, "
                      "size = ?, daily_rate = ?, is_active = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки update (billboard): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string surfaceStr = surfaceTypeToString(billboard.getSurfaceType());
    int activeInt = billboard.isActive() ? 1 : 0;

    sqlite3_bind_text(stmt, 1, billboard.getAddress().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, billboard.getDistrict().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, surfaceStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, billboard.getSize().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, billboard.getDailyRate());
    sqlite3_bind_int(stmt, 6, activeInt);
    sqlite3_bind_int(stmt, 7, billboard.getId());

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Ошибка обновления билборда: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}