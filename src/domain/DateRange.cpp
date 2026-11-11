#include "domain/DateRange.hpp"

#include <cctype>
#include <stdexcept>

DateRange::DateRange(std::string startDate, std::string endDate)
    : startDate(std::move(startDate)), endDate(std::move(endDate)) {
    if (!isValidDate(this->startDate) || !isValidDate(this->endDate)) {
        throw std::invalid_argument("Дата должна быть в формате YYYY-MM-DD");
    }
    if (this->endDate < this->startDate) {
        throw std::invalid_argument("Дата окончания раньше даты начала");
    }
}

int DateRange::getDays() const {
    return static_cast<int>(toDayNumber(endDate) - toDayNumber(startDate) + 1);
}

bool DateRange::overlaps(const DateRange& other) const {
    return startDate <= other.endDate && other.startDate <= endDate;
}

bool DateRange::isValidDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(date[i]))) return false;
    }

    const int year = std::stoi(date.substr(0, 4));
    const int month = std::stoi(date.substr(5, 2));
    const int day = std::stoi(date.substr(8, 2));
    if (year < 1970 || month < 1 || month > 12 || day < 1) {
        return false;
    }

    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    const int maxDay = (month == 2 && leap) ? 29 : daysInMonth[month - 1];
    return day <= maxDay;
}

long long DateRange::toDayNumber(const std::string& date) {
    // Алгоритм days_from_civil (Howard Hinnant): дата григорианского календаря -> номер дня.
    long long y = std::stoi(date.substr(0, 4));
    const int m = std::stoi(date.substr(5, 2));
    const int d = std::stoi(date.substr(8, 2));

    y -= m <= 2 ? 1 : 0;
    const long long era = y / 400;
    const long long yoe = y - era * 400;
    const long long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}
