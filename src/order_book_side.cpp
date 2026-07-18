#include "order_book_side.h"

namespace order_book {
    explicit OrderBookSide::OrderBookSide(Side side) : side(side) {}

    bool OrderBookSide::empty() const { return price_levels.empty();}

    void OrderBookSide::insert_order(Order* order) {
        if(!order) return;

        PriceLevel& curr_price_level = price_levels[order->price];
        curr_price_level.add_order_back(order);
    }

    Price OrderBookSide::get_best_price() const {
        if(price_levels.empty()) return 0;

        if(side == Side::BUY) return price_levels.rbegin()->first;
        else if(side == Side::SELL) return price_levels.begin()->first;
    }

    PriceLevel* OrderBookSide::get_best_price_level() {
        if(price_levels.empty()) return nullptr;

        if(side == Side::BUY) return &price_levels.rbegin()->second;
        else if(side == Side::SELL) return &price_levels.begin()->second;
    }

    const PriceLevel* OrderBookSide::get_best_price_level() const {
        if(price_levels.empty()) return nullptr;

        if(side == Side::BUY) return &price_levels.rbegin()->second;
        else if(side == Side::SELL) return &price_levels.begin()->second;
    }

    PriceLevel* OrderBookSide::find_price_level(Price price) {
        auto itr = price_levels.find(price);

        if(itr == price_levels.end()) return nullptr;

        return &itr->second;
    }

    void OrderBookSide::erase_level_if_empty(Price price) {
        auto itr = price_levels.find(price);

        if(itr == price_levels.end()) return ;

        if(itr->second.empty()) price_levels.erase(itr);
    }

}