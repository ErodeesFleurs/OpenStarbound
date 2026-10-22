import star.logging;
import std;
import star.format;
import star.root_loader;
import star.game_test;
import star.native_runtime;

#include "gtest/gtest.h"

using namespace Star;

GTEST_API_ int main(int argc, char** argv) {
  auto nativeRuntime = Star::NativeRuntime::create();
  if (!nativeRuntime) {
    std::fprintf(stderr, "%s\n", nativeRuntime.error().what());
    return 1;
  }
  testing::InitGoogleTest(&argc, argv);
  auto loader = RootLoader::create({{}, {}, {}, LogLevel::Error, true, {}});
  if (!loader) {
    std::fprintf(stderr, "%s\n", loader.error().what());
    return 1;
  }
  auto parsed = loader->commandParseOrDie(argc, argv);
  if (!parsed) {
    std::fprintf(stderr, "%s\n", parsed.error().what());
    return 1;
  }
  auto* environment = new GameTestEnvironment(std::move(parsed->first));
  testing::AddGlobalTestEnvironment(environment);
  registerGameTests(*environment);
  return RUN_ALL_TESTS();
}
