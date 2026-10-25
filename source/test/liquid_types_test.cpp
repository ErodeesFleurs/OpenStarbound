import std;
import star.cellular_liquid;
import star.format;
import star.game_test;
import star.liquids_database;
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
}
