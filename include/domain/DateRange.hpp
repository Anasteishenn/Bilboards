#pragma once
#include <optional>
#include <string>

// Период аренды [start; end] включительно. Даты хранятся в формате ISO "YYYY-MM-DD",
// как в таблице orders, поэтому их можно сравнивать как строки.
class DateRange {
private:
    std::string startDate;
    std::string endDate;

public:
    // Принимает даты в формате "YYYY-MM-DD" или "ДД.ММ.ГГГГ" (приводит к ISO).
    // Бросает std::invalid_argument, если дата некорректна или конец раньше начала.
    DateRange(std::string startDate, std::string endDate);

    const std::string& getStart() const { return startDate; }
    const std::string& getEnd() const { return endDate; }

    // Количество суток аренды (обе границы входят в период).
    int getDays() const;

    // Два периода пересекаются, если start1 <= end2 и start2 <= end1.
    bool overlaps(const DateRange& other) const;

    // Общая часть двух периодов: [max(start1, start2); min(end1, end2)], либо nullopt, если они не пересекаются.
    std::optional<DateRange> intersection(const DateRange& other) const;

    // Период для вывода пользователю: "05.11.2026 – 20.11.2026".
    std::string toString() const;

    static bool isValidDate(const std::string& date);
    // "ДД.ММ.ГГГГ" -> "YYYY-MM-DD"; строка в ISO возвращается как есть.
    // Бросает std::invalid_argument, если дата некорректна.
    static std::string normalizeDate(const std::string& date);
    // "YYYY-MM-DD" -> "ДД.ММ.ГГГГ".
    static std::string toDisplayDate(const std::string& isoDate);

private:
    // Номер дня от 1970-01-01, нужен для подсчёта длительности.
    static long long toDayNumber(const std::string& date);
};
