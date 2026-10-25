import std;
import star.cellular_liquid;
import star.format;
import star.game_test;
import star.liquids_database;
import star.rect;
import star.root;
import star.vector;
import star.liquid_types;
#include "gtest/gtest.h"

using namespace Star;

TEST(LiquidStoreTest, OverfullLevelSaturatesNetworkUpdates) {
  auto store = LiquidStore::filled(1, 0.5f);
  auto update = store.update(1, 1.1f, 3.0f);
  ASSERT_TRUE(update);
  EXPECT_EQ(update->liquid, 1);
  EXPECT_EQ(update->level, 255);
  EXPECT_EQ(store.netUpdate().level, 255);
  EXPECT_FLOAT_EQ(store.level, 1.1f);
  EXPECT_FLOAT_EQ(store.pressure, 3.0f);

  // Both overfull levels have the same wire value; only physics changes.
  EXPECT_FALSE(store.update(1, 1.3f, 4.0f));
  EXPECT_FLOAT_EQ(store.level, 1.3f);
  EXPECT_FLOAT_EQ(store.pressure, 4.0f);
}

namespace {
class LiquidBasin : public CellularLiquidWorld<LiquidId> {
public:
  static constexpr int Width = 5;
  static constexpr int Height = 3;
  std::array<CellularLiquidFlowCell<LiquidId>, Width * Height> cells{};
  mutable bool failNextRead = false;

  Vec2I uniqueLocation(Vec2I const& position) const override {
    return {(position[0] % Width + Width) % Width, position[1]};
  }

  std::expected<CellularLiquidCell<LiquidId>, StarException> cell(Vec2I const& position) const override {
    if (std::exchange(failNextRead, false))
      return std::unexpected(StarException("Transient basin read failure"));
    auto point = uniqueLocation(position);
    if (point[1] < 0 || point[1] >= Height)
      return CellularLiquidCollisionCell{};
    return cells[point[1] * Width + point[0]];
  }

  std::expected<void, StarException> setFlow(Vec2I const& position, CellularLiquidFlowCell<LiquidId> const& flow) override {
    auto point = uniqueLocation(position);
    if (point[1] < 0 || point[1] >= Height)
      return std::unexpected(StarException("Liquid flow attempted to overwrite basin boundary"));
    cells[point[1] * Width + point[0]] = flow;
    return {};
  }

  float mass() const {
    float total = 0;
    for (auto const& cell : cells)
      total += cell.level;
    return total;
  }
};

const GameTestRegistration liquidBorrowRecovery("LiquidEngine", "WrappedFlowConservesMassAfterReadFailure", [](GameTest& fixture) {
  auto database = fixture.root().liquidsDatabase();
  ASSERT_TRUE(database) << database.error().what();
  auto water = (*database)->liquidId("water");
  ASSERT_TRUE(water) << water.error().what();
  auto parameters = (*database)->liquidEngineParameters();
  // A closed basin has no drain or sub-threshold evaporation.
  parameters.minimumLiquidLevel = 0;
  auto basin = std::make_shared<LiquidBasin>();
  basin->cells[2 * LiquidBasin::Width] = {*water, 2.0f, 0.0f};
  LiquidCellEngine<LiquidId> engine(43, parameters, basin);
  engine.visitLocation({0, 2});
  ASSERT_TRUE(engine.update());
  basin->failNextRead = true;
  auto failed = engine.update();
  ASSERT_FALSE(failed);
  EXPECT_FLOAT_EQ(basin->mass(), 2.0f);

  engine.visitLocation({0, 2});
  for (unsigned tick = 0; tick < 64; ++tick) {
    auto updated = engine.update();
    ASSERT_TRUE(updated) << updated.error().what();
    EXPECT_NEAR(basin->mass(), 2.0f, 0.0001f);
  }
  float bottomMass = 0;
  for (int x = 0; x < LiquidBasin::Width; ++x)
    bottomMass += basin->cells[x].level;
  EXPECT_GT(bottomMass, 0.5f);
  EXPECT_GT(basin->cells[LiquidBasin::Width - 1].level, 0.01f);
});

class BudgetLiquidBasin : public CellularLiquidWorld<LiquidId> {
public:
  static constexpr int Count = 16;
  std::array<CellularLiquidFlowCell<LiquidId>, Count> cells{};
  mutable bool failNextRead = false;
  mutable std::optional<unsigned> readsBeforeFailure;

  Vec2I uniqueLocation(Vec2I const& position) const override {
    return {(position[0] % (Count * 2) + Count * 2) % (Count * 2), position[1]};
  }

  std::expected<CellularLiquidCell<LiquidId>, StarException> cell(Vec2I const& position) const override {
    if (std::exchange(failNextRead, false))
      return std::unexpected(StarException("Transient budget basin read failure"));
    if (readsBeforeFailure) {
      if (*readsBeforeFailure == 0) {
        readsBeforeFailure.reset();
        return std::unexpected(StarException("Delayed budget basin read failure"));
      }
      --*readsBeforeFailure;
    }
    auto point = uniqueLocation(position);
    if (point[1] != 0 || point[0] % 2 != 0)
      return CellularLiquidCollisionCell{};
    return cells[point[0] / 2];
  }

  std::expected<void, StarException> setFlow(Vec2I const& position,
                                             CellularLiquidFlowCell<LiquidId> const& flow) override {
    auto point = uniqueLocation(position);
    if (point[1] != 0 || point[0] % 2 != 0)
      return std::unexpected(StarException("Budget basin boundary overwritten"));
    cells[point[0] / 2] = flow;
    return {};
  }
};

const GameTestRegistration
  liquidBudgetOrdering("LiquidEngine", "UnlimitedCellsAfterBudgetAndOverlappingRegions", [](GameTest& fixture) {
    auto database = fixture.root().liquidsDatabase();
    ASSERT_TRUE(database) << database.error().what();
    auto water = (*database)->liquidId("water");
    ASSERT_TRUE(water) << water.error().what();
    auto lava = (*database)->liquidId("lava");
    ASSERT_TRUE(lava) << lava.error().what();
    // Isolated cells have no flow or pressure changes, so processing a cell
    // retires it. Active selection is observable without solver instrumentation.
    LiquidCellEngineParameters parameters{};
    auto basin = std::make_shared<BudgetLiquidBasin>();
    LiquidCellEngine<LiquidId> engine(43, parameters, basin);
    engine.setProcessingLimit(0);
    for (int i = 0; i < BudgetLiquidBasin::Count; ++i) {
      basin->cells[i] = {i % 2 ? *lava : *water, 0.5f, 0.5f};
      // Publish separately to establish a deterministic order in each liquid's
      // activation queue, independent of the visited-location hash set.
      engine.visitLocation({i * 2, 0});
      ASSERT_TRUE(engine.update());
    }
    ASSERT_EQ(engine.activeCells(), BudgetLiquidBasin::Count);
    engine.setProcessingLimit(2);
    // Engine membership uses Rect::contains (inclusive max), not belongs.
    // End at odd x=29 so x=28 is unlimited but x=30 remains ordinary.
    engine.setNoProcessingLimitRegions({RectI(20, 0, 28, 1), RectI(24, 0, 29, 1)});
    basin->failNextRead = true;
    auto failed = engine.update();
    ASSERT_FALSE(failed);
    ASSERT_EQ(engine.activeCells(), BudgetLiquidBasin::Count);
    ASSERT_TRUE(engine.update());
    for (int i = 0; i < BudgetLiquidBasin::Count; ++i) {
      bool unlimited = i >= 10 && i <= 14;
      EXPECT_EQ(engine.isActive({i * 2, 0}), !(i < 4 || unlimited)) << i;
      EXPECT_FLOAT_EQ(basin->cells[i].level, 0.5f);
      EXPECT_FLOAT_EQ(basin->cells[i].pressure, 0.5f);
      EXPECT_EQ(basin->cells[i].liquid, i % 2 ? *lava : *water);
    }
    EXPECT_EQ(engine.activeCells(), 7u);

    // Keep the same windows while retired cells become active again. The newly
    // unlimited cell must run with a zero ordinary budget; the newly ordinary
    // cell must stay behind the previously queued ordinary cells.
    engine.setProcessingLimit(0);
    engine.visitLocation({24, 0});
    engine.visitLocation({0, 0});
    ASSERT_TRUE(engine.update());
    EXPECT_TRUE(engine.isActive({24, 0}));
    EXPECT_TRUE(engine.isActive({0, 0}));
    EXPECT_EQ(engine.activeCells(), 9u);
    basin->failNextRead = true;
    ASSERT_FALSE(engine.update());
    EXPECT_EQ(engine.activeCells(), 9u);
    ASSERT_TRUE(engine.update());
    EXPECT_FALSE(engine.isActive({24, 0}));
    EXPECT_TRUE(engine.isActive({0, 0}));
    EXPECT_EQ(engine.activeCells(), 8u);
    engine.setProcessingLimit(1);
    ASSERT_TRUE(engine.update());
    EXPECT_FALSE(engine.isActive({8, 0}));
    EXPECT_FALSE(engine.isActive({10, 0}));
    EXPECT_TRUE(engine.isActive({0, 0}));
    EXPECT_EQ(engine.activeCells(), 6u);

    // Replacing the monitored windows must expose former unlimited cells to the
    // ordinary budget and include newly monitored cells without stale indexing.
    engine.setProcessingLimit(0);
    engine.setNoProcessingLimitRegions({RectI(30, 0, 32, 1)});
    ASSERT_TRUE(engine.update());
    EXPECT_FALSE(engine.isActive({30, 0}));
    EXPECT_TRUE(engine.isActive({12, 0}));
    EXPECT_TRUE(engine.isActive({18, 0}));
    EXPECT_EQ(engine.activeCells(), 5u);
    engine.setProcessingLimit({});
    engine.setLiquidTickDelta(*lava, 0);
    ASSERT_TRUE(engine.update());
    EXPECT_EQ(engine.activeCells(*water), 0u);
    EXPECT_EQ(engine.activeCells(*lava), 2u);
    engine.setLiquidTickDelta(*lava, 1);
    ASSERT_TRUE(engine.update());
    EXPECT_EQ(engine.activeCells(), 0u);
  });

const GameTestRegistration
  liquidPartialReadRecovery("LiquidEngine", "LateReadFailurePreservesRetirementAndBudgetOrder", [](GameTest& fixture) {
    auto database = fixture.root().liquidsDatabase();
    ASSERT_TRUE(database) << database.error().what();
    auto water = (*database)->liquidId("water");
    ASSERT_TRUE(water) << water.error().what();
    // Fail inside the ordinary prefix, then at two different positions in the
    // unlimited suffix after the ordinary budget has already been exhausted.
    for (unsigned successfulReads : {1u, 3u, 5u}) {
      SCOPED_TRACE(successfulReads);
      LiquidCellEngineParameters parameters{};
      auto basin = std::make_shared<BudgetLiquidBasin>();
      LiquidCellEngine<LiquidId> engine(43, parameters, basin);
      engine.setProcessingLimit(0);
      for (int i = 0; i < BudgetLiquidBasin::Count; ++i) {
        basin->cells[i] = {*water, 0.5f, 0.5f};
        engine.visitLocation({i * 2, 0});
        ASSERT_TRUE(engine.update());
      }
      engine.setProcessingLimit(2);
      engine.setNoProcessingLimitRegions({RectI(20, 0, 28, 1), RectI(24, 0, 29, 1)});
      basin->readsBeforeFailure = successfulReads;
      auto failed = engine.update();
      ASSERT_FALSE(failed);
      // The environment fails during selected-cell reads, before finalization.
      // Successful selections have already retired; the failed cell and all
      // later selections must remain active for recovery.
      constexpr std::array<int, 7> selected = {0, 1, 10, 11, 12, 13, 14};
      for (int i = 0; i < BudgetLiquidBasin::Count; ++i) {
        bool retired =
          std::find(selected.begin(), selected.begin() + successfulReads, i) != selected.begin() + successfulReads;
        EXPECT_EQ(engine.isActive({i * 2, 0}), !retired) << i;
      }
      auto expectConservedBasin = [&] {
        float mass = 0;
        for (auto const& cell : basin->cells) {
          EXPECT_EQ(cell.liquid, *water);
          EXPECT_FLOAT_EQ(cell.level, 0.5f);
          EXPECT_FLOAT_EQ(cell.pressure, 0.5f);
          mass += cell.level;
        }
        EXPECT_FLOAT_EQ(mass, BudgetLiquidBasin::Count * 0.5f);
      };
      expectConservedBasin();

      auto recovered = engine.update();
      ASSERT_TRUE(recovered) << recovered.error().what();
      // Recovery gets a fresh budget, not a replay of the failed tick. One
      // ordinary cell retired before the first fault, two before the others.
      int ordinaryRetired = successfulReads == 1 ? 3 : 4;
      for (int i = 0; i < BudgetLiquidBasin::Count; ++i) {
        bool retired = i < ordinaryRetired || (i >= 10 && i <= 14);
        EXPECT_EQ(engine.isActive({i * 2, 0}), !retired) << i;
      }
      expectConservedBasin();

      engine.setProcessingLimit(0);
      ASSERT_TRUE(engine.update());
      EXPECT_TRUE(engine.isActive({ordinaryRetired * 2, 0}));
      engine.setProcessingLimit(1);
      ASSERT_TRUE(engine.update());
      EXPECT_FALSE(engine.isActive({ordinaryRetired * 2, 0}));
      EXPECT_TRUE(engine.isActive({(ordinaryRetired + 1) * 2, 0}));
      expectConservedBasin();
    }
  });
}
