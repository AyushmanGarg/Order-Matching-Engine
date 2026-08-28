#include <iostream>
#include "order.h"

int main() {
    using namespace order_book;

    Order o{};
    o.id = 1;
    o.side = Side::BUY;
    o.price = 100;
    o.quantity = 10;
    o.remaining_quantity = 10;

    std::cout << "Example order ID: " << o.id << "\n";

    return 0;
}