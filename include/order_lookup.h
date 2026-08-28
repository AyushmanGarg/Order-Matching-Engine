#pragma once

#include <unordered_map>
#include <cstdint>
#include "order.h"

namespace order_book {

    struct OrderHandle {
        Side side;
        Order* order;
    }; 

    class OrderLookup {
        private:
            std::unordered_map<OrderId, OrderHandle> table;
        public:
            OrderLookup() = default;
            void insert(OrderId id, const OrderHandle& handle);
            const OrderHandle* find(OrderId id) const;
            OrderHandle* find(OrderId);
            void erase(OrderId id);
            void clear();
    }; 
}