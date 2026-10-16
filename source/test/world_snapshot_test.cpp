import star.json_extra;
import star.json;
import star.xx_hash;
import star.data_stream_extra;
import star.ordered_map;
import star.rect;
import star.either;
import star.vector;
import star.variant;
import star.game_types;
import star.material_types;
import star.color;
import star.bi_map;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.data_stream_devices;
import star.file;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import star.version;
import star.id_map;
import star.casting;
import star.image;
import star.string;
import star.strong_typedef;
import star.rpc_promise;
import star.set;
import star.worker_pool;

import std;
import star.sector_array_2d;
import star.net_compatibility;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.btree_database;
import star.ordered_set;
import star.map;

import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_types;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.lua_root;
import star.tile_sector_array;
import star.collision_block;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.tile_modification;
import star.world;
import star.liquid_types;
import star.tile_damage;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.chat_types;
import star.uuid;
import star.warping;
import star.wiring;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.cellular_light_array;
import star.cellular_lighting;
import star.cellular_liquid;
import star.lua_components;
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.world_render_data;
import star.world_client_state;
import star.interpolation_tracker;
import star.spawn_type_database;
import star.spawner;
import star.world_server;
import star.world_server_thread;
import star.world_structure;
import star.world_template;


#include "gtest/gtest.h"


import star.celestial_database;

import star.algorithm;
using namespace Star;

namespace {
  // Hashes one tile together with its position.  The per sample hashes are
  // combined order independently, so that two worlds can be compared without
  // serialising all of them.
  std::uint64_t tileHash(Vec2I position, ServerTile const& tile) {
    DataStreamBuffer buffer;
    buffer.write(position);
    tile.write(buffer);

    std::uint64_t hash = 14695981039346656037ull;
    for (auto byte : buffer.data())
      hash = (hash ^ (std::uint8_t)byte) * 1099511628211ull;
    return hash;
  }

  // Samples a fixed grid around the surface of a world.  Reading a tile
  // generates the sector it is in, so only the sampled sectors are generated.
  std::uint64_t sampleWorld(WorldServer& world, float surfaceLevel) {
    std::uint64_t hash = 0;
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
  auto celestialDatabase = std::make_shared<CelestialMasterDatabase>(File::relativeTo(storagePath, "universe.chunks"));

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
  float surfaceLevel = std::make_shared<WorldTemplate>(*coordinate, celestialDatabase)->surfaceLevel();

  auto makeWorld = [&]() {
    auto worldTemplate = std::make_shared<WorldTemplate>(*coordinate, celestialDatabase);
    return std::make_shared<WorldServer>(worldTemplate, File::ephemeralFile());
  };
  auto firstWorld = makeWorld();
  auto secondWorld = makeWorld();

  std::uint64_t firstHash = sampleWorld(*firstWorld, surfaceLevel);
  std::uint64_t secondHash = sampleWorld(*secondWorld, surfaceLevel);
  std::cerr << "WorldSnapshot: " << coordinate->id() << " hashes to 0x" << std::hex << firstHash << std::dec << std::endl;

  EXPECT_EQ(firstHash, secondHash);
  EXPECT_EQ(firstHash, 0x801745d507acfcaaull) << "update the expected hash (printed above)";
  firstWorld->close();
  secondWorld->close();
  celestialDatabase->close();
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
    auto world = std::make_shared<WorldServer>(worldSize, File::ephemeralFile());
    world->setCentralStructure(shipStructure);
    return world;
  };
  auto firstWorld = makeWorld();
  auto secondWorld = makeWorld();

  // The ship structure is placed around the middle of the world.
  Vec2I center((int)worldSize[0] / 2, (int)worldSize[1] / 2);
  std::uint64_t firstHash = 0;
  std::uint64_t secondHash = 0;
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
  firstWorld->close();
  secondWorld->close();
}

TEST(WorldSnapshot, StoppedWorldRetainsClientsAndExportsRecoverableChunks) {
  auto storage = File::temporaryFile();
  auto path = storage->fileName();
  auto cleanup = finally([&]() { storage->close(); File::remove(path); });
  auto world = std::make_shared<WorldServer>(Vec2U(2048, 2048), storage);
  auto speciesShips = Root::singleton().assets()->json("/universe_server.config:speciesShips");
  world->setCentralStructure(WorldStructure(jsonToStringList(speciesShips.get("human"))[0]));
  world->setProperty("stopSaveRegression", Json(42));

  WorldServerThread thread(world, ClientShipWorldId(Uuid()));
  ASSERT_TRUE(thread.addClient(MinClientConnectionId, SpawnTargetPosition(Vec2F(1024, 1024)),
      true, false, NetCompatibilityRules{}, MainClientWorldId));
  thread.start();
  thread.stop();

  // UniverseServer still needs this list to migrate players after joining.
  ASSERT_TRUE(thread.hasClient(MinClientConnectionId));
  EXPECT_EQ(thread.clients().size(), 1u);
  EXPECT_FALSE(thread.serverErrorOccurred());
  auto finalItem = std::make_shared<GiveItemPacket>(ItemDescriptor("money", 7, Json()));
  ASSERT_TRUE(world->sendPacket(MinClientConnectionId, finalItem));

  // An asynchronous warp may wait for its destination.  Merely queueing it
  // must leave the stopped source alive, with its final outgoing queue intact.
  auto waitingChunks = thread.readChunks();
  ASSERT_FALSE(waitingChunks.empty());
  ASSERT_TRUE(thread.hasClient(MinClientConnectionId));
  ASSERT_TRUE(storage->isOpen());

  // Ship/custom-world snapshots are exported after stop, before final close.
  thread.unloadAll(true);
  auto chunks = thread.readChunks();
  ASSERT_FALSE(chunks.empty());
  ASSERT_FALSE(thread.serverErrorOccurred());
  auto finalPackets = thread.removeClient(MinClientConnectionId);
  ASSERT_FALSE(finalPackets.empty());
  auto transport = LocalPacketSocket::openPair();
  transport.first->sendPackets(std::move(finalPackets));
  auto delivered = transport.second->receivePackets();
  EXPECT_TRUE(delivered.contains(finalItem));
  ASSERT_FALSE(delivered.empty());
  EXPECT_EQ(delivered.back()->type(), PacketType::WorldStop);
  EXPECT_TRUE(thread.noClients());
  thread.close();
  thread.close();
  EXPECT_FALSE(thread.serverErrorOccurred());

  WorldServer fromChunks(chunks);
  EXPECT_EQ(fromChunks.getProperty("stopSaveRegression", Json()), Json(42));
  fromChunks.close();

  WorldServer fromFile(File::open(path, IOMode::ReadWrite));
  EXPECT_EQ(fromFile.getProperty("stopSaveRegression", Json()), Json(42));
  fromFile.close();
}
