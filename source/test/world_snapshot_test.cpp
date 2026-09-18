#include "StarCelestialCoordinate.hpp"
#include "StarCelestialDatabase.hpp"
#include "StarDataStreamDevices.hpp"
#include "StarFile.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"
#include "StarWorldServer.hpp"
#include "StarWorldStructure.hpp"
#include "StarWorldTemplate.hpp"

#include <iostream>

#include "gtest/gtest.h"

using namespace Star;

namespace {
  // Hashes one tile together with its position.  The per sample hashes are
  // combined order independently, so that two worlds can be compared without
  // serialising all of them.
  uint64_t tileHash(Vec2I position, ServerTile const& tile) {
    DataStreamBuffer buffer;
    buffer.write(position);
    tile.write(buffer);

    uint64_t hash = 14695981039346656037ull;
    for (auto byte : buffer.data())
      hash = (hash ^ (uint8_t)byte) * 1099511628211ull;
    return hash;
  }

  // Samples a fixed grid around the surface of a world.  Reading a tile
  // generates the sector it is in, so only the sampled sectors are generated.
  uint64_t sampleWorld(WorldServer& world, float surfaceLevel) {
    uint64_t hash = 0;
    for (int x : {0, 128, 256, 384, 512, 640, 768, 896}) {
      for (int y = (int)surfaceLevel - 1280; y <= (int)surfaceLevel + 512; y += 128)
        hash ^= tileHash(Vec2I(x, y), world.getServerTile(Vec2I(x, y)));
    }
    return hash;
  }
}

// World generation is deterministic: it is a pure function of the celestial
// parameters (the coordinate feeds the planet seed, the system/type fields come
// from fixed Perlin seeds in the assets), so these hashes are stable across
// runs and machines and can be compared to a constant.  When world generation
// changes on purpose, rerun with --gtest_filter='WorldSnapshot.*'; the new hash
// is printed to stderr.
TEST(WorldSnapshot, FixedCoordinateGenerationIsDeterministic) {
  auto storagePath = File::temporaryDirectory();
  auto finallyGuard = finally([&storagePath]() { File::removeDirectoryRecursive(storagePath); });
  auto celestialDatabase = make_shared<CelestialMasterDatabase>(File::relativeTo(storagePath, "universe.chunks"));

  // Find a real planet: the celestial generator places systems sparsely, and a
  // hand picked coordinate is most likely empty.
  auto systems = celestialDatabase->scanSystems(RectI::withSize(Vec2I(0, 0), Vec2I(32, 32)));
  ASSERT_FALSE(systems.empty());
  Maybe<CelestialCoordinate> coordinate;
  for (auto const& system : systems) {
    for (auto const& child : celestialDatabase->children(system)) {
      if (child.isPlanetaryBody() && celestialDatabase->parameters(child)) {
        coordinate = child;
        break;
      }
    }
    if (coordinate)
      break;
  }
  ASSERT_TRUE(coordinate.isValid()) << "no planetary body found in the scanned region";
  // A separate template for the surface level: a template handed to a WorldServer
  // is modified by world generation.
  float surfaceLevel = make_shared<WorldTemplate>(*coordinate, celestialDatabase)->surfaceLevel();

  auto makeWorld = [&]() {
    auto worldTemplate = make_shared<WorldTemplate>(*coordinate, celestialDatabase);
    return make_shared<WorldServer>(worldTemplate, File::ephemeralFile());
  };
  auto firstWorld = makeWorld();
  auto secondWorld = makeWorld();

  uint64_t firstHash = sampleWorld(*firstWorld, surfaceLevel);
  uint64_t secondHash = sampleWorld(*secondWorld, surfaceLevel);
  std::cerr << "WorldSnapshot: " << coordinate->id() << " hashes to 0x" << std::hex << firstHash << std::dec << std::endl;

  EXPECT_EQ(firstHash, secondHash);
  EXPECT_EQ(firstHash, 0x801745d507acfcaaull) << "update the expected hash (printed above)";
}

TEST(WorldSnapshot, ShipWorldIsDeterministic) {
  // The ship world does not depend on celestial parameters, so it has its own
  // stable hash.
  auto speciesShips = Root::singleton().assets()->json("/universe_server.config:speciesShips");
  auto shipStructure = WorldStructure(jsonToStringList(speciesShips.get("human"))[0]);
  Vec2U worldSize(2048, 2048);
  if (auto jWorldSize = shipStructure.configValue("worldSize"))
    worldSize = jsonToVec2U(jWorldSize);

  auto makeWorld = [&]() {
    auto world = make_shared<WorldServer>(worldSize, File::ephemeralFile());
    world->setCentralStructure(shipStructure);
    return world;
  };
  auto firstWorld = makeWorld();
  auto secondWorld = makeWorld();

  // The ship structure is placed around the middle of the world.
  Vec2I center((int)worldSize[0] / 2, (int)worldSize[1] / 2);
  uint64_t firstHash = 0;
  uint64_t secondHash = 0;
  unsigned nonEmptyTiles = 0;
  for (int x = -160; x <= 160; x += 16) {
    for (int y = -160; y <= 160; y += 16) {
      Vec2I position = center + Vec2I(x, y);
      auto const& firstTile = firstWorld->getServerTile(position);
      firstHash ^= tileHash(position, firstTile);
      secondHash ^= tileHash(position, secondWorld->getServerTile(position));
      if (firstTile.foreground != EmptyMaterialId)
        ++nonEmptyTiles;
    }
  }
  std::cerr << "WorldSnapshot: ship world hashes to 0x" << std::hex << firstHash << std::dec << std::endl;

  EXPECT_GT(nonEmptyTiles, 0u) << "the sampled region does not contain any ship structure";
  EXPECT_EQ(firstHash, secondHash);
  EXPECT_EQ(firstHash, 0x96a3df5c62832b0bull) << "update the expected hash (printed above)";
}
