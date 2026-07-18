#pragma once

#include <cstdint>

namespace order_book {
    enum class Side : std::unit8_t {
        // scoped enumeration so we need to access items using Side::BUY
        BUY,
        SELL
    };
    //__builtin_prefetch(cur->next, 0, 1); this tells cpu that we will be soon needing the data at this memory.  It hides the latency by overlapping the fetch with useful work. this is called software prefecthing.
    using OrderId = std::uint64_t;
    using Price = std::int32_t;

    struct alignas(64) Order {
        Order *next;
        Order *prev;
        OrderId id;
        Side side;
        std::int64_t quantity;
        std::int64_t remaining_quantity;
        std::int64_t timestamp;
        std::int32_t flags;
        Price price;
    };

    // no alignas(64) for trade because we dont want it to consume extra 24 bytes
    struct Trade {
        OrderId taker_id;
        OrderId maker_id;
        Price price;
        std::int64_t quantity;
        std::int64_t remaining_quantity;
    };

}