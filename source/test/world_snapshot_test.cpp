#include "StarCelestialCoordinate.hpp"
#include "StarCelestialDatabase.hpp"
#include "StarDataStreamDevices.hpp"
#include "StarFile.hpp"
#include "StarLogging.hpp"
#include "StarWorldServer.hpp"
#include "StarWorldTemplate.hpp"

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
}

TEST(WorldSnapshot, FixedCoordinateGenerationIsDeterministic) {
  // The same coordinate on the same celestial database has to produce identical
  // tiles.  Generation that depends on anything but its parameters would make
  // differential testing against another implementation meaningless.
  //
  // There is no hard coded expected hash: a celestial database file that is
  // created from scratch gets a random universe uuid, and the uuid feeds the
  // celestial parameters that the world is generated from.  Keep the storage
  // file to reproduce a specific world.
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

  // Sampling a fixed grid around the surface level: reading a tile generates the
  // sector it is in, so only the sampled sectors are generated.
  uint64_t firstHash = 0;
  uint64_t secondHash = 0;
  unsigned samples = 0;
  for (int x : {0, 128, 256, 512, 768}) {
    for (int i = -4; i <= 4; ++i) {
      Vec2I position(x, (int)surfaceLevel + i * 128);
      firstHash ^= tileHash(position, firstWorld->getServerTile(position));
      secondHash ^= tileHash(position, secondWorld->getServerTile(position));
      ++samples;
    }
  }

  Logger::info("WorldSnapshot: {} samples of {} hash to {:x}", samples, coordinate->id(), firstHash);
  EXPECT_EQ(firstHash, secondHash);
}
