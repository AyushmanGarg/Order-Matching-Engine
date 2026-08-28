# Order Matching Engine

A single-symbol, single-threaded limit order book with **price-time (FIFO) priority**, written in C++20 with a focus on predictable latency: no heap allocation on the hot path, intrusive linked lists, and a preallocated order pool.

---

## Design

The engine is split into five independent pieces, each responsible for one data structure:

| Component             | Files                       | Structure                                         | Responsibility                                                                                            |
| --------------------- | --------------------------- | ------------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| `Order` / `Trade` | `include/order.h`         | 64-byte POD                                       | Order record with intrusive`next`/`prev` links                                                        |
| `PriceLevel`        | `price_level.{h,cpp}`     | Intrusive doubly-linked FIFO                      | All orders resting at one price, plus a running total quantity                                            |
| `OrderBookSide`     | `order_book_side.{h,cpp}` | `boost::container::flat_map<Price, PriceLevel>` | One side of the book;`side` decides whether the best price is `begin()` (asks) or `rbegin()` (bids) |
| `OrderPool`         | `order_pool.{h,cpp}`      | Preallocated`vector<Node>` + index freelist     | O(1) allocate/deallocate with zero runtime`new`                                                         |
| `OrderLookup`       | `order_lookup.{h,cpp}`    | `unordered_map<OrderId, OrderHandle>`           | Maps an order ID to its side and its address in the pool, for O(1) cancel                                 |
| `MatchingEngine`    | `matching_engine.{h,cpp}` | —                                                | Orchestrates the above:`add_order`, `cancel_order`, `modify_order`                                  |

### Why these structures

- **Intrusive lists** — an order's `next`/`prev` pointers live inside the `Order` itself, so joining or leaving a price level never allocates a list node. Cancel is a pointer unlink, not a search.
- **Object pool** — every order lives in one contiguous `vector`, allocated once at construction. Freed slots are threaded into an index-based freelist, so allocate/deallocate are a couple of loads and stores.
- **`flat_map` for price levels** — levels are stored sorted in contiguous memory, so reading the best price is a single cache-friendly access at one end of the array. The trade-off is that inserting a *new* price level shifts the tail of the array.
- **Hash lookup by ID** — cancel and modify take an order ID, not a location, so the engine needs an ID → address index.

### Complexity

With `L` distinct price levels on a side:

| Operation                             | Cost                                                                               |
| ------------------------------------- | ---------------------------------------------------------------------------------- |
| Best price / best level               | **O(1)**                                                                     |
| Add order, price level already exists | **O(log L)** lookup, O(1) append                                             |
| Add order, new price level            | O(log L) lookup +**O(L)** insert (array shift)                               |
| Match one resting order               | **O(1)**                                                                     |
| Cancel                                | **O(1)** hash lookup + O(1) unlink (O(L) if the level empties and is erased) |
| Modify                                | Cancel + re-add (loses time priority, by design)                                   |

### Matching semantics

- Incoming orders sweep the opposite side from the best price outward, filling the **oldest** order at each level first.
- Trades execute at the **resting (maker) order's** price, not the taker's.
- An order that is not fully filled rests in the book at its limit price; a fully filled order is returned to the pool immediately and never enters the book.
- `modify_order` is implemented as cancel-then-add, so a modified order goes to the back of the queue at its new price.

---

## Building

**Requirements**

- CMake 3.20+
- A C++20 compiler
- Boost headers (header-only; only `boost/container/flat_map.hpp` is used)

GoogleTest and Google Benchmark are fetched automatically by CMake's `FetchContent`, so the first configure needs network access.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

> Always configure with an explicit `CMAKE_BUILD_TYPE`. The default (empty) build type applies **no optimization**, which makes the benchmark numbers meaningless.

### Targets

| Target                   | Output                         | What it is                                |
| ------------------------ | ------------------------------ | ----------------------------------------- |
| `order_book_engine`    | `liborder_book_engine.a`     | The static library — the engine itself   |
| `order_book_simulator` | `build/order_book_simulator` | Executable entry point (currently a stub) |
| `tests`                | `build/tests`                | GoogleTest suite                          |
| `order_book_bench`     | `build/order_book_bench`     | Google Benchmark suite                    |

---

## Running

```bash
# Unit tests
./build/tests
# ...or through CTest
ctest --test-dir build --output-on-failure

# Benchmarks (all)
./build/order_book_bench

# Benchmarks (one)
./build/order_book_bench --benchmark_filter=BM_CrossingOrders
```

The benchmark suite covers pure inserts, crossing orders, cancels, best-price reads, multi-level sweeps, modifies, and a 100k-level book.

---

## Usage

```cpp
#include "matching_engine.h"

using namespace order_book;

// Pool capacity is fixed at construction: the maximum number of
// simultaneously live orders.
MatchingEngine engine(1'000'000);

// NewOrder{ id, side, price, quantity, timestamp, flags }
engine.add_order({1, Side::SELL, 100, 50, /*ts=*/0, /*flags=*/0});

// Crosses the resting sell → returns the resulting fills.
std::vector<Trade> trades = engine.add_order({2, Side::BUY, 100, 30, 1, 0});
// trades[0] == { taker_id: 2, maker_id: 1, price: 100, quantity: 30, ... }
// Order 2 is fully filled and never rests; order 1 still rests with 20 left.

// Cancel-and-replace: re-prices order 1 to 101, sending it to the
// back of the queue at the new level. Returns any fills it produces.
engine.modify_order(1, {1, Side::SELL, 101, 20, 2, 0});

engine.cancel_order(1);   // → true
engine.cancel_order(2);   // → false (already filled, no longer in the book)

// Read-only book access.
Price best_bid = engine.get_bids().get_best_price();
```

---

## Layout

```
include/           Public headers (the library's interface)
  order.h            Order, Trade, Side, OrderId, Price
  price_level.h      PriceLevel
  order_book_side.h  OrderBookSide
  order_pool.h       OrderPool
  order_lookup.h     OrderLookup
  matching_engine.h  NewOrder, MatchingEngine
src/               Implementations + the simulator entry point
tests/             GoogleTest unit tests
benchmarks/        Google Benchmark microbenchmarks
```
