#include "order_lookup.h"

namespace order_book {

    void OrderLookup::insert(OrderId id, const OrderHandle& handle) {
        table[id] = handle;
    }

    const OrderHandle* OrderLookup::find(OrderId id) const {
        auto itr = table.find(id);
        if (itr == table.end()) {
            return nullptr;
        }
        return &itr->second;
    }

    OrderHandle* OrderLookup::find(OrderId id) {
        auto itr = table.find(id);
        if (itr == table.end()) {
            return nullptr;
        }
        return &itr->second;
    }

    void OrderLookup::erase(OrderId id) {
        table.erase(id);
    }

    void OrderLookup::clear() {
        table.clear();
    }

}