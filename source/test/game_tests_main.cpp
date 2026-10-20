import star.logging;
import std;
import star.root_loader;
import star.game_test;
import star.native_runtime;

#include "gtest/gtest.h"

using namespace Star;

GTEST_API_ int main(int argc, char** argv) {
  [[maybe_unused]] Star::NativeRuntime nativeRuntime;
  testing::InitGoogleTest(&argc, argv);
  auto* environment = new GameTestEnvironment(RootLoader({{}, {}, {}, LogLevel::Error, true, {}}).commandParseOrDie(argc, argv).first);
  testing::AddGlobalTestEnvironment(environment);
  registerGameTests(*environment);
  return RUN_ALL_TESTS();
}
