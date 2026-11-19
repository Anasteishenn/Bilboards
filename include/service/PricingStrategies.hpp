#pragma once
#include "service/IPricingStrategy.hpp"

namespace service {

// Без скидок: dailyRate * days.
class StandardPricing : public IPricingStrategy {
public:
    PriceQuote calculate(const PricingContext& context) const override;
    std::string getName() const override { return "Стандартный тариф"; }
};

// Скидка за длительность: от 7 суток — 5%, от 14 — 10%, от 30 — 15%.
class DurationDiscountPricing : public IPricingStrategy {
public:
    PriceQuote calculate(const PricingContext& context) const override;
    std::string getName() const override { return "Скидка за длительность"; }
};

// Скидка постоянного клиента: 10%, если у клиента уже есть minApprovedOrders одобренных заказов.
class LoyaltyDiscountPricing : public IPricingStrategy {
public:
    explicit LoyaltyDiscountPricing(int minApprovedOrders = 3) : minApprovedOrders(minApprovedOrders) {}

    PriceQuote calculate(const PricingContext& context) const override;
    std::string getName() const override { return "Скидка постоянного клиента"; }

private:
    int minApprovedOrders;
};

// Выбирает самую выгодную для клиента скидку из двух стратегий выше (скидки не суммируются).
class BestDiscountPricing : public IPricingStrategy {
public:
    PriceQuote calculate(const PricingContext& context) const override;
    std::string getName() const override { return "Лучшая доступная скидка"; }

private:
    DurationDiscountPricing duration;
    LoyaltyDiscountPricing loyalty;
};

} // namespace service
