#include "domain/Billboard.hpp"

#include <stdexcept>

std::string surfaceTypeToString(SurfaceType type) {
    switch (type) {
        case SurfaceType::Static: return "Static";
        case SurfaceType::Digital: return "Digital";
        case SurfaceType::Prismatron: return "Prismatron";
    }
    return "Static";
}

SurfaceType surfaceTypeFromString(const std::string& value) {
    if (value == "Static") return SurfaceType::Static;
    if (value == "Digital") return SurfaceType::Digital;
    if (value == "Prismatron") return SurfaceType::Prismatron;
    throw std::invalid_argument("Неизвестный формат поверхности: " + value);
}

Billboard::Billboard(int id, std::string address, std::string district, SurfaceType surfaceType,
                     std::string size, double dailyRate, bool active)
    : id(id), surfaceType(surfaceType), size(std::move(size)), active(active) {
    setAddress(std::move(address));
    setDistrict(std::move(district));
    setDailyRate(dailyRate);
}

void Billboard::setAddress(std::string newAddress) {
    if (newAddress.empty()) {
        throw std::invalid_argument("Адрес не может быть пустым");
    }
    address = std::move(newAddress);
}

void Billboard::setDistrict(std::string newDistrict) {
    if (newDistrict.empty()) {
        throw std::invalid_argument("Район не может быть пустым");
    }
    district = std::move(newDistrict);
}

void Billboard::setDailyRate(double newRate) {
    if (newRate <= 0.0) {
        throw std::invalid_argument("Суточная ставка должна быть больше нуля");
    }
    dailyRate = newRate;
}
