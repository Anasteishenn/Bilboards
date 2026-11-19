#include "service/BillboardService.hpp"

#include <cctype>
#include <stdexcept>

namespace service {

namespace {

// Нижний регистр для латиницы и кириллицы в UTF-8 (А-Я, Ё), чтобы "московский" находил "Московский".
std::string toLowerUtf8(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto ch = static_cast<unsigned char>(text[i]);
        if (ch == 0xD0 && i + 1 < text.size()) {
            const auto next = static_cast<unsigned char>(text[i + 1]);
            if (next >= 0x90 && next <= 0x9F) {          // А-П -> а-п
                result += static_cast<char>(0xD0);
                result += static_cast<char>(next + 0x20);
                ++i;
                continue;
            }
            if (next >= 0xA0 && next <= 0xAF) {          // Р-Я -> р-я
                result += static_cast<char>(0xD1);
                result += static_cast<char>(next - 0x20);
                ++i;
                continue;
            }
            if (next == 0x81) {                          // Ё -> ё
                result += static_cast<char>(0xD1);
                result += static_cast<char>(0x91);
                ++i;
                continue;
            }
        }
        result += static_cast<char>(std::tolower(ch));
    }
    return result;
}

} // namespace

BillboardService::BillboardService(std::shared_ptr<IBillboardRepository> billboardRepository)
    : billboardRepository(std::move(billboardRepository)) {
    if (!this->billboardRepository) {
        throw std::invalid_argument("BillboardService: не передан репозиторий щитов");
    }
}

std::vector<Billboard> BillboardService::getAllBillboards() const {
    std::vector<Billboard> active;
    for (const auto& billboard : billboardRepository->getAll()) {
        if (billboard.isActive()) {
            active.push_back(billboard);
        }
    }
    return active;
}

std::vector<Billboard> BillboardService::getAllBillboardsIncludingInactive() const {
    return billboardRepository->getAll();
}

std::optional<Billboard> BillboardService::getBillboard(int id) const {
    return billboardRepository->getById(id);
}

std::vector<Billboard> BillboardService::filterBillboards(const BillboardFilter& filter) const {
    const std::string district = toLowerUtf8(filter.district);
    std::vector<Billboard> result;
    for (const auto& billboard : getAllBillboards()) {
        // Поиск по части названия района: "моск" найдёт "Московский".
        if (!district.empty() && toLowerUtf8(billboard.getDistrict()).find(district) == std::string::npos) continue;
        if (filter.surfaceType && billboard.getSurfaceType() != *filter.surfaceType) continue;
        if (filter.maxDailyRate > 0.0 && billboard.getDailyRate() > filter.maxDailyRate) continue;
        result.push_back(billboard);
    }
    return result;
}

std::vector<Billboard> BillboardService::filterBillboards(const std::string& district, double maxDailyRate) const {
    BillboardFilter filter;
    filter.district = district;
    filter.maxDailyRate = maxDailyRate;
    return filterBillboards(filter);
}

void BillboardService::addBillboard(const Billboard& billboard) {
    if (!billboardRepository->save(billboard)) {
        throw std::runtime_error("Не удалось сохранить рекламный щит");
    }
}

void BillboardService::updateBillboard(const Billboard& billboard) {
    requireBillboard(billboard.getId());
    if (!billboardRepository->update(billboard)) {
        throw std::runtime_error("Не удалось обновить рекламный щит #" + std::to_string(billboard.getId()));
    }
}

void BillboardService::deactivateBillboard(int id) {
    Billboard billboard = requireBillboard(id);
    billboard.deactivate();
    if (!billboardRepository->update(billboard)) {
        throw std::runtime_error("Не удалось списать рекламный щит #" + std::to_string(id));
    }
}

Billboard BillboardService::requireBillboard(int id) const {
    const auto billboard = billboardRepository->getById(id);
    if (!billboard) {
        throw std::invalid_argument("Рекламный щит #" + std::to_string(id) + " не найден");
    }
    return *billboard;
}

} // namespace service
