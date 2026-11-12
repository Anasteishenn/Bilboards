#pragma once

#include <vector>
#include <optional>
#include "repository/IBillboardRepository.hpp"
#include "repository/DatabaseManager.hpp"
#include "sqlite3.h"

class SqliteBillboardRepository : public IBillboardRepository {
public:
    explicit SqliteBillboardRepository(DatabaseManager& dbManager);
    ~SqliteBillboardRepository() override = default;

    std::optional<Billboard> getById(int id) override;
    std::vector<Billboard> getAll() override;
    bool save(const Billboard& billboard) override;
    bool update(const Billboard& billboard) override;

private:
    DatabaseManager& dbManager_;

    static Billboard mapRowToBillboard(sqlite3_stmt* stmt);
};