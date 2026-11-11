#pragma once
#include <string>

// Период аренды [start; end] включительно. Даты хранятся в формате ISO "YYYY-MM-DD",
// как в таблице orders, поэтому их можно сравнивать как строки.
class DateRange {
private:
    std::string startDate;
    std::string endDate;

public:
    // Бросает std::invalid_argument, если дата некорректна или конец раньше начала.
    DateRange(std::string startDate, std::string endDate);

    const std::string& getStart() const { return startDate; }
    const std::string& getEnd() const { return endDate; }

    // Количество суток аренды (обе границы входят в период).
    int getDays() const;

    // Два периода пересекаются, если start1 <= end2 и start2 <= end1.
    bool overlaps(const DateRange& other) const;

    static bool isValidDate(const std::string& date);

private:
    // Номер дня от 1970-01-01, нужен для подсчёта длительности.
    static long long toDayNumber(const std::string& date);
};
