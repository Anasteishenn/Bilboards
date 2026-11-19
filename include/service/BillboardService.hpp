#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "domain/Billboard.hpp"
#include "repository/IBillboardRepository.hpp"

namespace service {

// Критерии поиска; пустое поле означает "любое значение".
struct BillboardFilter {
    std::string district;                     // часть названия, без учёта регистра
    std::optional<SurfaceType> surfaceType;
    double maxDailyRate{0.0};                 // 0 — без ограничения
};

// Каталог рекламных щитов и управление инвентарём.
class BillboardService {
public:
    explicit BillboardService(std::shared_ptr<IBillboardRepository> billboardRepository);

    // Активные щиты — то, что видит клиент в каталоге.
    std::vector<Billboard> getAllBillboards() const;
    // Все щиты, включая списанные, — для администратора и отчётов.
    std::vector<Billboard> getAllBillboardsIncludingInactive() const;
    std::optional<Billboard> getBillboard(int id) const;

    std::vector<Billboard> filterBillboards(const BillboardFilter& filter) const;
    // Короткая форма: район ("" — любой) и максимальная ставка (0 — без ограничения).
    std::vector<Billboard> filterBillboards(const std::string& district, double maxDailyRate) const;

    // Администрирование. Бросают std::invalid_argument / std::runtime_error при ошибке.
    void addBillboard(const Billboard& billboard);
    void updateBillboard(const Billboard& billboard);
    void deactivateBillboard(int id);

private:
    std::shared_ptr<IBillboardRepository> billboardRepository;

    Billboard requireBillboard(int id) const;
};

} // namespace service
