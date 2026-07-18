#include "price_level.h"

namespace order_book {
    void PriceLevel::add_order_back(Order* order) {
        if(!order) return;

        order->next = nullptr;
        order->prev = tail;

        if(tail) tail->next = order;
        else  head->next = order;

        tail = order;
        total_qty_at_price += order->remaining_quantity;
    }
        
    Order* PriceLevel::front() {
        return head;
    }

    const Order* front() const {
        return head;
    }

    void pop_front() {
        if(!head) return;

        total_qty_at_price -= head.remaining_quantity;
        Order* tmp = head;
        head = head->next;

        if(head) head->prev = nullptr;
        else tail = nullptr;

        tmp->next = nullptr;
        tmp->prev = nullptr;
    }

    void remove_order(Order* order) {
        if(!order) return;

        total_qty_at_price -= order->remaining_quantity;
        Order* next = order->next;
        Order* prev = order->prev;

        if(prev) prev->next = next;
        else head = next;

        if(next) next->prev = prev;
        else tail = prev;

        order->prev = nullptr;
        order->next = nullptr;
    }
}

/*
why two front are defibned?
*/