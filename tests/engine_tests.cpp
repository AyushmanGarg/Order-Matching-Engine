#include <gtest/gtest.h>
#include "matching_engine.h"

using namespace order_book;

static NewOrder make_order(OrderId id, Side side, int64_t price, int64_t qty, std::uint64_t ts = 0) {
    return NewOrder{id, side, price, qty, ts, 0};
}

TEST(MatchingEngineTest, SimpleMatchProducesOneTrade) {
    MatchingEngine engine(100);

    engine.add_order(make_order(1, Side::SELL, 100, 50));
    auto trades = engine.add_order(make_order(2, Side::BUY, 100, 50));

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].maker_id, 1);
    EXPECT_EQ(trades[0].taker_id, 2);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 50);
}

TEST(MatchingEngineTest, PartialFillLeavesRemainingQuantity) {
    MatchingEngine engine(100);

    engine.add_order(make_order(1, Side::SELL, 100, 40));
    auto trades = engine.add_order(make_order(2, Side::BUY, 100, 100));

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 40);

    trades = engine.add_order(make_order(3, Side::SELL, 100, 60));

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 60);
}

TEST(MatchingEngineTest, MultiLevelSweepRespectsPriceTimePriority) {
    MatchingEngine engine(100);

    engine.add_order(make_order(1, Side::SELL, 100, 50));
    engine.add_order(make_order(2, Side::SELL, 100, 30));

    engine.add_order(make_order(3, Side::SELL, 101, 100));

    auto trades = engine.add_order(make_order(9, Side::BUY, 150, 150));

    ASSERT_EQ(trades.size(), 3);

    EXPECT_EQ(trades[0].maker_id, 1);
    EXPECT_EQ(trades[1].maker_id, 2);
    EXPECT_EQ(trades[2].maker_id, 3);

    EXPECT_EQ(trades[0].quantity, 50);
    EXPECT_EQ(trades[1].quantity, 30);
    EXPECT_EQ(trades[2].quantity, 70);
}

TEST(MatchingEngineTest, FIFOWithinSamePrice) {
    MatchingEngine engine(100);

    engine.add_order(make_order(1, Side::SELL, 100, 20));
    engine.add_order(make_order(2, Side::SELL, 100, 20));

    auto trades = engine.add_order(make_order(9, Side::BUY, 100, 20));

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].maker_id, 1);
}

TEST(MatchingEngineTest, CancelRemovesOrder) {
    MatchingEngine engine(100);

    engine.add_order(make_order(1, Side::SELL, 100, 50));
    EXPECT_TRUE(engine.cancel_order(1));

    auto trades = engine.add_order(make_order(2, Side::BUY, 100, 50));
    EXPECT_TRUE(trades.empty());
}

TEST(MatchingEngineTest, ModifyOrderChangesBehavior) {
    MatchingEngine engine(100);

    engine.add_order(make_order(1, Side::SELL, 105, 100));

    NewOrder mod = make_order(1, Side::SELL, 99, 100);
    auto trades = engine.modify_order(1, mod);

    ASSERT_TRUE(trades.empty());

    trades = engine.add_order(make_order(2, Side::BUY, 99, 100));

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].maker_id, 1);
}

TEST(MatchingEngineTest, AddNonCrossingOrderProducesNoTrades) {
    MatchingEngine engine(100);

    auto trades = engine.add_order(make_order(1, Side::BUY, 100, 10));
    EXPECT_TRUE(trades.empty());
}