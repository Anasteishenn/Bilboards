#include "service/PricingStrategies.hpp"

#include <cmath>

namespace service {

namespace {

double roundMoney(double value) {
    return std::round(value * 100.0) / 100.0;
}

PriceQuote makeQuote(const PricingContext& context, double discountPercent, std::string discountName) {
    PriceQuote quote;
    quote.days = context.period.getDays();
    quote.baseCost = roundMoney(context.dailyRate * quote.days);
    quote.discountPercent = discountPercent;
    quote.discountAmount = roundMoney(quote.baseCost * discountPercent / 100.0);
    quote.total = roundMoney(quote.baseCost - quote.discountAmount);
    if (discountPercent > 0.0) {
        quote.discountName = std::move(discountName);
    }
    return quote;
}

} // namespace

PriceQuote StandardPricing::calculate(const PricingContext& context) const {
    return makeQuote(context, 0.0, "");
}

PriceQuote DurationDiscountPricing::calculate(const PricingContext& context) const {
    const int days = context.period.getDays();
    if (days >= 30) return makeQuote(context, 15.0, "Скидка за аренду от 30 суток");
    if (days >= 14) return makeQuote(context, 10.0, "Скидка за аренду от 14 суток");
    if (days >= 7) return makeQuote(context, 5.0, "Скидка за аренду от 7 суток");
    return makeQuote(context, 0.0, "");
}

PriceQuote LoyaltyDiscountPricing::calculate(const PricingContext& context) const {
    if (context.clientApprovedOrders >= minApprovedOrders) {
        return makeQuote(context, 10.0, "Скидка постоянного клиента");
    }
    return makeQuote(context, 0.0, "");
}

PriceQuote BestDiscountPricing::calculate(const PricingContext& context) const {
    const PriceQuote byDuration = duration.calculate(context);
    const PriceQuote byLoyalty = loyalty.calculate(context);
    return byLoyalty.total < byDuration.total ? byLoyalty : byDuration;
}

} // namespace service
