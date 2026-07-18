#pragma once

#include <boost/container/flat_map.hpp>
#include <map>
#include <cstdint>
#include "order.h"
#include "price_level.h"

namespace order_book {

    class OrderBookSide {
        private:
            Side side;
            boost::container::flat_map<Price, PriceLevel> price_levels;

        public:
            explicit OrderBookSide(Side side);
            bool empty() const;
            void insert_order(Order* order);
            std::int64_t get_best_price() const;
            PriceLevel* get_best_price_level();
            const PriceLevel* get_best_price_level() const;
            PriceLevel* find_price_level(std::int32_t price);
            void erase_level_if_empty(std::int32_t price);
    };

}