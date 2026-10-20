import star.json;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.algorithm;
import star.file;
import star.game_test;
import star.lua;
import star.lua_converters;
import star.lua_root;
import star.random;
import star.root_lua_bindings;
import star.string;
import star.tech_database;
import star.collision_block;
import star.game_types;
import star.map;
import star.material_database;
import star.material_types;
import star.rect;
import star.tile_modification;
import star.utility_lua_bindings;
import star.vector;
import star.world_server;
import star.world_tiles;

#include "gtest/gtest.h"

using namespace Star;

namespace {

class RootIsolationErrorSink : public LogSink {
public:
  RootIsolationErrorSink() { setLevel(LogLevel::Error); }
  void log(char const* message, LogLevel) override {
    ADD_FAILURE() << "Error was logged: " << message;
  }
};

const GameTestRegistration rootAll("RootTest", "All", [](GameTest& fixture) {
  auto& root = fixture.root();

  EXPECT_TRUE((bool)root.assets());
  EXPECT_TRUE((bool)root.objectDatabase());
  EXPECT_TRUE((bool)root.plantDatabase());
  EXPECT_TRUE((bool)root.projectileDatabase());
  EXPECT_TRUE((bool)root.monsterDatabase());
  EXPECT_TRUE((bool)root.npcDatabase());
  EXPECT_TRUE((bool)root.playerFactory());
  EXPECT_TRUE((bool)root.entityFactory());
  EXPECT_TRUE((bool)root.nameGenerator());
  EXPECT_TRUE((bool)root.itemDatabase());
  EXPECT_TRUE((bool)root.materialDatabase());
  EXPECT_TRUE((bool)root.terrainDatabase());
  EXPECT_TRUE((bool)root.biomeDatabase());
  EXPECT_TRUE((bool)root.liquidsDatabase());
  EXPECT_TRUE((bool)root.statusEffectDatabase());
  EXPECT_TRUE((bool)root.damageDatabase());
  EXPECT_TRUE((bool)root.particleDatabase());
  EXPECT_TRUE((bool)root.effectSourceDatabase());
  EXPECT_TRUE((bool)root.functionDatabase());
  EXPECT_TRUE((bool)root.treasureDatabase());
  EXPECT_TRUE((bool)root.dungeonDefinitions());
  EXPECT_TRUE((bool)root.emoteProcessor());
  EXPECT_TRUE((bool)root.speciesDatabase());
  EXPECT_TRUE((bool)root.imageMetadataDatabase());
  EXPECT_TRUE((bool)root.versioningDatabase());
  EXPECT_TRUE((bool)root.questTemplateDatabase());
  EXPECT_TRUE((bool)root.aiDatabase());
  EXPECT_TRUE((bool)root.techDatabase());
  EXPECT_TRUE((bool)root.codexDatabase());
  EXPECT_TRUE((bool)root.stagehandDatabase());
  EXPECT_TRUE((bool)root.behaviorDatabase());
  EXPECT_TRUE((bool)root.tenantDatabase());
  EXPECT_TRUE((bool)root.danceDatabase());
  EXPECT_TRUE((bool)root.spawnTypeDatabase());

  root.reload();

  EXPECT_TRUE((bool)root.assets());
  EXPECT_TRUE((bool)root.objectDatabase());
  EXPECT_TRUE((bool)root.plantDatabase());
  EXPECT_TRUE((bool)root.projectileDatabase());
  EXPECT_TRUE((bool)root.monsterDatabase());
  EXPECT_TRUE((bool)root.npcDatabase());
  EXPECT_TRUE((bool)root.playerFactory());
  EXPECT_TRUE((bool)root.entityFactory());
  EXPECT_TRUE((bool)root.nameGenerator());
  EXPECT_TRUE((bool)root.itemDatabase());
  EXPECT_TRUE((bool)root.materialDatabase());
  EXPECT_TRUE((bool)root.terrainDatabase());
  EXPECT_TRUE((bool)root.biomeDatabase());
  EXPECT_TRUE((bool)root.liquidsDatabase());
  EXPECT_TRUE((bool)root.statusEffectDatabase());
  EXPECT_TRUE((bool)root.damageDatabase());
  EXPECT_TRUE((bool)root.particleDatabase());
  EXPECT_TRUE((bool)root.effectSourceDatabase());
  EXPECT_TRUE((bool)root.functionDatabase());
  EXPECT_TRUE((bool)root.treasureDatabase());
  EXPECT_TRUE((bool)root.dungeonDefinitions());
  EXPECT_TRUE((bool)root.emoteProcessor());
  EXPECT_TRUE((bool)root.speciesDatabase());
  EXPECT_TRUE((bool)root.imageMetadataDatabase());
  EXPECT_TRUE((bool)root.versioningDatabase());
  EXPECT_TRUE((bool)root.questTemplateDatabase());
  EXPECT_TRUE((bool)root.aiDatabase());
  EXPECT_TRUE((bool)root.techDatabase());
  EXPECT_TRUE((bool)root.codexDatabase());
  EXPECT_TRUE((bool)root.stagehandDatabase());
  EXPECT_TRUE((bool)root.behaviorDatabase());
  EXPECT_TRUE((bool)root.tenantDatabase());
  EXPECT_TRUE((bool)root.danceDatabase());
  EXPECT_TRUE((bool)root.spawnTypeDatabase());
});

void writeIsolationAssets(String const& overlay, String const& owner) {
  File::writeFile(Json(JsonObject{{"owner", owner}}).repr(),
      File::relativeTo(overlay, "root-isolation.config"));
  File::writeFile(Json(JsonObject{
      {"name", "root-isolation"},
      {"type", "Head"},
      {"scripts", JsonArray{}},
      {"description", owner},
      {"shortDescription", owner},
      {"rarity", "common"},
      {"icon", "/interface/missing.png"}
    }).repr(), File::relativeTo(overlay, "root-isolation.tech"));
  File::writeFile("function isolationScriptOwner() return " + Json(owner).repr() + " end",
      File::relativeTo(overlay, "root-isolation.lua"));
}

Root::Settings isolationSettings(Root::Settings const& base, String const& directory, String const& owner) {
  auto settings = base;
  auto overlay = File::relativeTo(directory, "assets");
  File::makeDirectory(overlay);
  writeIsolationAssets(overlay, owner);
  // Keep the fixture's legal base pak and production mod sources; only the
  // test-owned overlay, runtime configuration, storage, and logs are private.
  settings.assetSources.append(overlay);
  // Isolate explicit reload invalidation from maintenance-thread TTL eviction
  // while loading the real pak on slower test hosts.
  settings.assetsSettings.assetTimeToLive = std::numeric_limits<float>::max();
  settings.storageDirectory = File::relativeTo(directory, "storage");
  settings.runtimeConfigFile = String("root-isolation.config");
  settings.logDirectory = {};
  settings.logFile = {};
  settings.includeUGC = false;
  settings.defaultConfiguration = settings.defaultConfiguration.set("rootIsolationOwner", owner);
  return settings;
}

void expectIsolationConsumers(Root& root, LuaContext& context, String const& owner, String const& scriptOwner,
    String const& configurationOwner) {
  EXPECT_EQ(root.assets()->json("/root-isolation.config").getString("owner"), owner);
  EXPECT_EQ(root.techDatabase()->tech("root-isolation").description, owner);
  EXPECT_EQ(root.configuration()->get("rootIsolationOwner").toString(), configurationOwner);
  EXPECT_EQ(context.eval<String>("root.assetJson('/root-isolation.config').owner"), owner);
  EXPECT_EQ(context.eval<String>("root.techConfig('root-isolation').description"), owner);
  EXPECT_EQ(context.eval<String>("root.getConfiguration('rootIsolationOwner')"), configurationOwner);
  EXPECT_EQ(context.invokePath<String>("isolationScriptOwner"), scriptOwner);
}

const GameTestRegistration simultaneousRoots("RootTest", "SimultaneousRoots", [](GameTest& fixture) {
  auto directoryA = File::temporaryDirectory(fixture.root().random());
  auto cleanupA = finally([&] { File::removeDirectoryRecursive(directoryA); });
  auto directoryB = File::temporaryDirectory(fixture.root().random());
  auto cleanupB = finally([&] { File::removeDirectoryRecursive(directoryB); });
  auto settingsA = isolationSettings(fixture.rootSettings(), directoryA, "A");
  auto settingsB = isolationSettings(fixture.rootSettings(), directoryB, "B");
  auto overlayA = File::relativeTo(directoryA, "assets");
  auto overlayB = File::relativeTo(directoryB, "assets");
  auto rootA = std::make_unique<Root>(settingsA);
  rootA->logger().addSink(std::make_shared<RootIsolationErrorSink>());
  auto rootB = std::make_unique<Root>(settingsB);
  rootB->logger().addSink(std::make_shared<RootIsolationErrorSink>());
  auto logPathA = File::relativeTo(directoryA, "isolation.log");
  auto logPathB = File::relativeTo(directoryB, "isolation.log");
  auto sinkA = std::make_shared<FileLogSink>(logPathA, LogLevel::Info, true);
  auto sinkB = std::make_shared<FileLogSink>(logPathB, LogLevel::Info, true);
  rootA->logger().addSink(sinkA);
  rootB->logger().addSink(sinkB);

  EXPECT_NE(rootA->toStoragePath("root-isolation.config"), rootB->toStoragePath("root-isolation.config"));
  {
    auto luaA = std::make_unique<LuaRoot>(*rootA);
    LuaRoot luaB(*rootB);
    luaA->addCallbacks("root", LuaBindings::makeRootCallbacks(*rootA));
    luaB.addCallbacks("root", LuaBindings::makeRootCallbacks(*rootB));
    luaA->addCallbacks("sb", LuaBindings::makeUtilityCallbacks(rootA->random(), rootA->logger(), rootA->logMap()));
    luaB.addCallbacks("sb", LuaBindings::makeUtilityCallbacks(rootB->random(), rootB->logger(), rootB->logMap()));
    auto contextA = std::make_unique<LuaContext>(luaA->createContext("/root-isolation.lua"));
    auto contextB = luaB.createContext("/root-isolation.lua");
    expectIsolationConsumers(*rootA, *contextA, "A", "A", "A");
    expectIsolationConsumers(*rootB, contextB, "B", "B", "B");

    // Exercise writes through the real Lua root table, not only C++ getters.
    contextA->eval("root.setConfiguration('rootIsolationOwner', 'A-written')");
    contextB.eval("root.setConfiguration('rootIsolationOwner', 'B-written')");
    expectIsolationConsumers(*rootA, *contextA, "A", "A", "A-written");
    expectIsolationConsumers(*rootB, contextB, "B", "B", "B-written");
    contextA->eval("sb.logInfo('root-isolation-log-A'); sb.setLogMap('root-isolation', 'A')");
    contextB.eval("sb.logInfo('root-isolation-log-B'); sb.setLogMap('root-isolation', 'B')");
    EXPECT_EQ(rootA->logMap().getValue("root-isolation"), "A");
    EXPECT_EQ(rootB->logMap().getValue("root-isolation"), "B");
    rootA->logMap().clear();
    EXPECT_TRUE(rootA->logMap().getValues().empty());
    EXPECT_EQ(rootB->logMap().getValue("root-isolation"), "B");
    EXPECT_TRUE(File::readFileString(logPathA).contains("root-isolation-log-A"));
    EXPECT_FALSE(File::readFileString(logPathA).contains("root-isolation-log-B"));
    EXPECT_TRUE(File::readFileString(logPathB).contains("root-isolation-log-B"));
    EXPECT_FALSE(File::readFileString(logPathB).contains("root-isolation-log-A"));

    // A real filesystem failure must leave both live owners usable and must
    // not prevent a subsequent successful Root construction.
    auto failedSettings = settingsA;
    failedSettings.storageDirectory = File::relativeTo(directoryA, "not-a-directory");
    File::writeFile("regular file", failedSettings.storageDirectory);
    EXPECT_THROW((Root{failedSettings}), std::exception);
    auto recoveredSettings = settingsA;
    recoveredSettings.storageDirectory = File::relativeTo(directoryA, "recovered-storage");
    {
      Root recovered(recoveredSettings);
      recovered.logger().addSink(std::make_shared<RootIsolationErrorSink>());
      LuaRoot recoveredLua(recovered);
      recoveredLua.addCallbacks("root", LuaBindings::makeRootCallbacks(recovered));
      auto recoveredContext = recoveredLua.createContext("/root-isolation.lua");
      expectIsolationConsumers(recovered, recoveredContext, "A", "A", "A");
    }
    expectIsolationConsumers(*rootA, *contextA, "A", "A", "A-written");
    expectIsolationConsumers(*rootB, contextB, "B", "B", "B-written");

    auto listenerA = std::make_shared<TrackerListener>();
    auto listenerB = std::make_shared<TrackerListener>();
    auto subscriptionA = rootA->registerReloadListener(listenerA);
    auto subscriptionB = rootB->registerReloadListener(listenerB);
    {
      auto assetsA = rootA->assets();
      auto assetsB = rootB->assets();
      auto databaseA = rootA->techDatabase();
      auto databaseB = rootB->techDatabase();
      writeIsolationAssets(overlayA, "A-reloaded");
      writeIsolationAssets(overlayB, "B-reloaded");

      rootA->reload();
      EXPECT_TRUE(listenerA->pullTriggered());
      EXPECT_FALSE(listenerB->pullTriggered());
      EXPECT_FALSE(luaA->scriptLoaded("/root-isolation.lua"));
      EXPECT_TRUE(luaB.scriptLoaded("/root-isolation.lua"));
      EXPECT_NE(rootA->assets().get(), assetsA.get());
      EXPECT_NE(rootA->techDatabase().get(), databaseA.get());
      EXPECT_EQ(rootB->assets().get(), assetsB.get());
      EXPECT_EQ(rootB->techDatabase().get(), databaseB.get());
      EXPECT_EQ(databaseA->tech("root-isolation").description, "A");
      // Existing closures follow their own owner's reloaded services, while
      // already instantiated Lua functions keep their original script body.
      expectIsolationConsumers(*rootA, *contextA, "A-reloaded", "A", "A-written");
      auto reloadedContextA = luaA->createContext("/root-isolation.lua");
      expectIsolationConsumers(*rootA, reloadedContextA, "A-reloaded", "A-reloaded", "A-written");
      expectIsolationConsumers(*rootB, contextB, "B", "B", "B-written");
      auto cachedContextB = luaB.createContext("/root-isolation.lua");
      expectIsolationConsumers(*rootB, cachedContextB, "B", "B", "B-written");
    }

    // Destroy all A script borrowers before A. B remains live throughout.
    contextA.reset();
    luaA.reset();
    subscriptionA.disconnect();
    rootA.reset();
    EXPECT_FALSE(listenerB->pullTriggered());
    expectIsolationConsumers(*rootB, contextB, "B", "B", "B-written");
    contextB.eval("root.setConfiguration('rootIsolationOwner', 'B-survives')");
    expectIsolationConsumers(*rootB, contextB, "B", "B", "B-survives");
    EXPECT_EQ(rootB->logMap().getValue("root-isolation"), "B");
    contextB.eval("sb.logInfo('root-isolation-log-B-survives'); sb.setLogMap('root-isolation', 'B-survives')");
    EXPECT_EQ(rootB->logMap().getValue("root-isolation"), "B-survives");
    EXPECT_TRUE(File::readFileString(logPathB).contains("root-isolation-log-B-survives"));
    EXPECT_FALSE(File::readFileString(logPathA).contains("root-isolation-log-B-survives"));

    rootB->reload();
    EXPECT_TRUE(listenerB->pullTriggered());
    EXPECT_FALSE(luaB.scriptLoaded("/root-isolation.lua"));
    expectIsolationConsumers(*rootB, contextB, "B-reloaded", "B", "B-survives");
    auto reloadedContextB = luaB.createContext("/root-isolation.lua");
    expectIsolationConsumers(*rootB, reloadedContextB, "B-reloaded", "B-reloaded", "B-survives");
  }
  rootB.reset();
  EXPECT_EQ(Json::parseJson(File::readFileString(File::relativeTo(settingsA.storageDirectory,
      "root-isolation.config"))).getString("rootIsolationOwner"), "A-written");
  EXPECT_EQ(Json::parseJson(File::readFileString(File::relativeTo(settingsB.storageDirectory,
      "root-isolation.config"))).getString("rootIsolationOwner"), "B-survives");
});

void writeIsolationMaterial(String const& overlay, MaterialId material, String const& collisionKind) {
  // These are the required MaterialDatabase parser fields. Omit the optional
  // render profile: the consumer is the real headless world's collision path.
  File::writeFile(Json(JsonObject{
      {"materialId", material},
      {"materialName", "root-isolation"},
      {"category", "materials"},
      {"collisionKind", collisionKind}
    }).repr(), File::relativeTo(overlay, "root-isolation.material"));
}

void placeIsolationMaterial(WorldServer& world, Vec2I const& position, MaterialId material, bool collides) {
  world.generateRegion(RectI::withSize(position - Vec2I(4, 4), Vec2I(9, 9)));
  world.setTileProtectionEnabled(false);
  auto tile = world.modifyServerTile(position, true);
  ASSERT_NE(tile, nullptr);
  ASSERT_EQ(tile->foreground, EmptyMaterialId);
  auto center = Vec2F(position) + Vec2F(0.5f, 0.5f);
  EXPECT_FALSE(world.pointTileCollision(center));
  // Supply a legitimate attachment surface, then let WorldServer perform the
  // placement and derive its collision from this world's MaterialDatabase.
  tile->background = StructureMaterialId;
  ASSERT_TRUE(world.forceModifyTile(position, PlaceMaterial{TileLayer::Foreground, material, MaterialHue(0)}, true));
  EXPECT_EQ(world.material(position, TileLayer::Foreground), material);
  EXPECT_EQ(world.pointTileCollision(center), collides);
  EXPECT_EQ(world.lineTileCollision(center - Vec2F(0.25f, 0), center + Vec2F(0.25f, 0)), collides);
}

const GameTestRegistration simultaneousWorldMaterials("RootTest", "SimultaneousWorldMaterials", [](GameTest& fixture) {
  // Select a genuinely unused real ID, rather than assuming a mod collection
  // leaves a hard-coded material slot free.
  auto materials = fixture.root().materialDatabase();
  MaterialId material = 0;
  while (material < FirstMetaMaterialId && materials->isValidMaterialId(material))
    ++material;
  ASSERT_LT(material, FirstMetaMaterialId);

  auto directoryA = File::temporaryDirectory(fixture.root().random());
  auto cleanupA = finally([&] { File::removeDirectoryRecursive(directoryA); });
  auto directoryB = File::temporaryDirectory(fixture.root().random());
  auto cleanupB = finally([&] { File::removeDirectoryRecursive(directoryB); });
  auto settingsA = isolationSettings(fixture.rootSettings(), directoryA, "A");
  auto settingsB = isolationSettings(fixture.rootSettings(), directoryB, "B");
  auto overlayA = File::relativeTo(directoryA, "assets");
  auto overlayB = File::relativeTo(directoryB, "assets");
  writeIsolationMaterial(overlayA, material, "block");
  writeIsolationMaterial(overlayB, material, "none");

  auto rootA = std::make_unique<Root>(settingsA);
  rootA->logger().addSink(std::make_shared<RootIsolationErrorSink>());
  auto rootB = std::make_unique<Root>(settingsB);
  rootB->logger().addSink(std::make_shared<RootIsolationErrorSink>());
  auto worldA = std::make_unique<WorldServer>(*rootA, Vec2U(2048, 2048), File::ephemeralFile(rootA->random()));
  auto closeWorldA = finally([&] { if (worldA) worldA->close(); });
  auto worldB = std::make_unique<WorldServer>(*rootB, Vec2U(2048, 2048), File::ephemeralFile(rootB->random()));
  auto closeWorldB = finally([&] { worldB->close(); });
  EXPECT_EQ(rootA->materialDatabase()->materialId("root-isolation"), material);
  EXPECT_EQ(rootB->materialDatabase()->materialId("root-isolation"), material);

  // Identical material IDs and placement requests produce different actual
  // world collision behavior, not merely different catalog properties.
  placeIsolationMaterial(*worldA, Vec2I(1024, 1024), material, true);
  placeIsolationMaterial(*worldB, Vec2I(1024, 1024), material, false);
  writeIsolationMaterial(overlayA, material, "none");
  writeIsolationMaterial(overlayB, material, "block");
  rootA->reload();
  placeIsolationMaterial(*worldA, Vec2I(1032, 1024), material, false);
  placeIsolationMaterial(*worldB, Vec2I(1032, 1024), material, false);

  worldA->close();
  worldA.reset();
  rootA.reset();
  placeIsolationMaterial(*worldB, Vec2I(1040, 1024), material, false);
  rootB->reload();
  placeIsolationMaterial(*worldB, Vec2I(1048, 1024), material, true);
});

}
