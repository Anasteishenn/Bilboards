#pragma once
#include <string>

enum class SurfaceType {
    Static,
    Digital,
    Prismatron
};

// Преобразование для хранения в БД (столбец billboards.surface_type) и вывода на экран.
std::string surfaceTypeToString(SurfaceType type);
// Бросает std::invalid_argument для неизвестного значения.
SurfaceType surfaceTypeFromString(const std::string& value);

class Billboard {
private:
    int id{0};
    std::string address;
    std::string district;
    SurfaceType surfaceType{SurfaceType::Static};
    std::string size;
    double dailyRate{0.0};
    bool active{true};

public:
    Billboard() = default;
    // Бросает std::invalid_argument, если адрес/район пустые или ставка не положительная.
    Billboard(int id, std::string address, std::string district, SurfaceType surfaceType,
              std::string size, double dailyRate, bool active = true);

    int getId() const { return id; }
    const std::string& getAddress() const { return address; }
    const std::string& getDistrict() const { return district; }
    SurfaceType getSurfaceType() const { return surfaceType; }
    const std::string& getSize() const { return size; }
    double getDailyRate() const { return dailyRate; }
    bool isActive() const { return active; }

    // Редактирование параметров администратором.
    void setAddress(std::string newAddress);
    void setDistrict(std::string newDistrict);
    void setSurfaceType(SurfaceType newType) { surfaceType = newType; }
    void setSize(std::string newSize) { size = std::move(newSize); }
    void setDailyRate(double newRate);

    // Списание конструкции: неактивный щит не показывается в каталоге и не бронируется.
    void deactivate() { active = false; }
    void activate() { active = true; }
};
