#include "StarUniverseServer.hpp"
#include "StarRoot.hpp"

#include "gtest/gtest.h"

using namespace Star;

TEST(ServerTest, Run) {
  auto& root = Root::singleton();
  UniverseServer server(root.toStoragePath("universe"), root.assets(), root.configuration(), root.itemDatabase());
  server.start();
  server.stop();
  server.join();
}
