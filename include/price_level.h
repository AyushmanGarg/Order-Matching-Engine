#pragma once
#include <cstdint>
#include "order.h"

namespace order_book {

    class PriceLevel {
        private:
            std::int64_t total_qty_at_price = 0;
            Order* head = nullptr;
            Order* tail = nullptr;

        public:
            bool empty() const { return head == nullptr; }
            void add_order_back(Order* order);
            Order* front();
            const Order* front() const;
            void pop_front();
            std::int64_t get_total_quantity() const { return total_qty_at_price; }
            void reduce_total_quantity(std::int64_t amt) { total_qty_at_price -= amt; }
            void remove_order(Order* order);

    };
}