#include <benchmark/benchmark.h>
#include "matching_engine.h"

using namespace order_book;

static void BM_InsertOnly(benchmark::State& state) {
    MatchingEngine engine(1'000'000);

    int64_t price = 100;

    for (auto _ : state) {
        NewOrder o{1, Side::BUY, price, 10, 0, 0};
        engine.add_order(o);
        price++;
    }
}
BENCHMARK(BM_InsertOnly);

static void BM_CrossingOrders(benchmark::State& state) {
    MatchingEngine engine(1'000'000);

    for (int i = 0; i < 10000; i++) {
        engine.add_order({static_cast<OrderId>(i), Side::SELL, 100, 1, 0, 0});
    }

    for (auto _ : state) {
        NewOrder o{999999, Side::BUY, 100, 1, 0, 0};
        engine.add_order(o);
    }
}
BENCHMARK(BM_CrossingOrders);

static void BM_Cancel(benchmark::State& state) {
    MatchingEngine engine(1'000'000);

    for (int i = 0; i < 10000; i++) {
        engine.add_order({static_cast<OrderId>(i), Side::BUY, 100, 1, 0, 0});
    }

    for (auto _ : state) {
        engine.cancel_order(5000);
    }
}
BENCHMARK(BM_Cancel);

static void BM_BestPrice(benchmark::State& state) {
    MatchingEngine engine(100000);

    for (int i = 0; i < 50000; i++) {
        engine.add_order({static_cast<OrderId>(i), Side::SELL, i, 1, 0, 0});
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(engine.get_asks().get_best_price());
    }
}
BENCHMARK(BM_BestPrice);

static void BM_SweepManyLevels(benchmark::State& state) {
    const int LEVELS = 2000;
    MatchingEngine engine(2'000'000);

    for (int i = 0; i < LEVELS; i++) {
        engine.add_order({static_cast<OrderId>(i), Side::SELL, 100 + i, 1, 0, 0});
    }

    for (auto _ : state) {
        NewOrder o{999999, Side::BUY, 100 + LEVELS + 50, LEVELS, 0, 0};
        engine.add_order(o);
    }
}
BENCHMARK(BM_SweepManyLevels);

static void BM_AlwaysCross(benchmark::State& state) {
    MatchingEngine engine(1'000'000);

    for (int i = 0; i < 500000; i++) {
        engine.add_order({static_cast<OrderId>(i), Side::SELL, 100, 1, 0, 0});
    }

    for (auto _ : state) {
        NewOrder o{1'000'000, Side::BUY, 100, 1, 0, 0};
        engine.add_order(o);
    }
}
BENCHMARK(BM_AlwaysCross);

static void BM_ModifyOrder(benchmark::State& state) {
    MatchingEngine engine(1000000);

    for (int i = 0; i < 200000; i++) {
        engine.add_order({static_cast<OrderId>(i), Side::BUY, 100, 10, 0, 0});
    }

    for (auto _ : state) {
        NewOrder modified{12345, Side::BUY, 101, 5, 0, 0};
        engine.modify_order(12345, modified);
    }
}
BENCHMARK(BM_ModifyOrder);

static void BM_LargeBook(benchmark::State& state) {
    MatchingEngine engine(2'000'000);

    const int LEVELS = 100000;

    for (int i = 0; i < LEVELS; i++) {
        engine.add_order({static_cast<OrderId>(i), Side::SELL, i, 1, 0, 0});
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(engine.get_asks().get_best_price());
    }
}
BENCHMARK(BM_LargeBook);

static void BM_CancelHeavy(benchmark::State& state) {
    MatchingEngine engine(1'000'000);

    for (int i = 0; i < 200000; i++) {
        engine.add_order({static_cast<OrderId>(i), Side::BUY, 100, 1, 0, 0});
    }

    for (auto _ : state) {
        engine.cancel_order(1000);
        engine.add_order({1000, Side::BUY, 100, 1, 0, 0});
    }
}
BENCHMARK(BM_CancelHeavy);

BENCHMARK_MAIN();