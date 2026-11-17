#pragma once
#include <string>

#include "domain/DateRange.hpp"

namespace service {

// Исходные данные для расчёта стоимости аренды.
struct PricingContext {
    double dailyRate;           // базовая ставка щита за сутки
    DateRange period;           // период аренды
    int clientApprovedOrders;   // сколько заказов клиента уже одобрено (для скидки постоянного клиента)
};

// Результат расчёта — всё, что нужно показать клиенту в блоке "Расчёт".
struct PriceQuote {
    int days{0};
    double baseCost{0.0};        // dailyRate * days
    double discountPercent{0.0};
    double discountAmount{0.0};
    double total{0.0};           // baseCost - discountAmount
    std::string discountName;    // пустая строка, если скидки нет
};

// Паттерн Strategy: алгоритм расчёта цены можно заменить, не меняя BookingService.
class IPricingStrategy {
public:
    virtual ~IPricingStrategy() = default;
    virtual PriceQuote calculate(const PricingContext& context) const = 0;
    virtual std::string getName() const = 0;
};

} // namespace service
