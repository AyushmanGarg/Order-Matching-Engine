#pragma once

#include <vector>
#include <cstdint>
#include "order.h"

namespace order_book {

    class OrderPool {
        private:
            struct Node {
                Order order;
                std::uint32_t next;
            }; 

            std::vector<Node> nodes;
            std::uint32_t free_node;
            std::size_t max_capacity;
            std::size_t curr_size;
            std::uint32_t get_idx_from_ptr(const Order* order) const;
        
        public:
            explicit OrderPool(std::size_t max_capacity);
            std::size_t get_capacity() const { return max_capacity; }
            std::size_t get_curr_size() const { return curr_size; }
            Order* allocate();
            void deallocate(Order* order);
            void reset();
    };

}