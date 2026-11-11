#pragma once
#include <vector>
#include <optional>
#include "domain/Billboard.hpp"

class IBillboardRepository {
public:
    virtual ~IBillboardRepository() = default;
    virtual std::optional<Billboard> getById(int id) = 0;
    virtual std::vector<Billboard> getAll() = 0;
    virtual bool save(const Billboard& billboard) = 0;
    virtual bool update(const Billboard& billboard) = 0;
};