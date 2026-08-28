#pragma once

#include "order.h"
#include "order_pool.h"
#include "price_level.h"
#include "order_lookup.h"
#include "order_book_side.h"
#include <cstdint>
#include <vector>

namespace order_book {

    struct NewOrder {
        OrderId id;
        Side side;
        std::int64_t price;
        std::int64_t quantity;
        std::uint64_t timestamp;
        std::int64_t flags;
    };

    class MatchingEngine {
        private:
            OrderPool pool;
            OrderBookSide bids;
            OrderBookSide asks;
            OrderLookup lookup;

        public:
            explicit MatchingEngine(std::size_t max_orders);
            std::vector<Trade> add_order(const NewOrder& new_order);
            bool cancel_order(OrderId id);
            std::vector<Trade> modify_order(OrderId id, const NewOrder& new_order);

            const OrderBookSide& get_bids() const { return bids; }
            const OrderBookSide& get_asks() const { return asks; }
    };

}