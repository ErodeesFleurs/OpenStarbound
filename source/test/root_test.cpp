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
import star.format;
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
import star.networked_animator;
import star.networked_animator_lua_bindings;
import star.scripted_animator_lua_bindings;
import star.drawable;
import star.item;
import star.item_bag;
import star.item_database;
import star.inventory_types;
import star.player_inventory;
import star.player;
import star.player_factory;
import star.player_log;
import star.radio_message_database;
import star.npc;
import star.npc_database;
import star.status_controller;
import star.world_lua_bindings;
import star.lua_game_converters;

#include "gtest/gtest.h"

using namespace Star;

namespace {

class RootIsolationErrorSink : public LogSink {
public:
  RootIsolationErrorSink() { setLevel(LogLevel::Error); }
  [[nodiscard]] std::expected<void, StarException> log(std::string_view message, LogLevel) override {
    ADD_FAILURE() << "Error was logged: " << message;
    return {};
  }
};

const GameTestRegistration rootAll("RootTest", "All", [](GameTest& fixture) {
  auto& root = fixture.root();

  auto expectCatalog = [](auto result) {
    ASSERT_TRUE(result) << result.error().what();
    EXPECT_TRUE(*result);
  };
  expectCatalog(root.assets());
  expectCatalog(root.objectDatabase());
  expectCatalog(root.plantDatabase());
  expectCatalog(root.projectileDatabase());
  expectCatalog(root.monsterDatabase());
  expectCatalog(root.npcDatabase());
  expectCatalog(root.playerFactory());
  expectCatalog(root.entityFactory());
  expectCatalog(root.nameGenerator());
  expectCatalog(root.itemDatabase());
  expectCatalog(root.materialDatabase());
  expectCatalog(root.terrainDatabase());
  expectCatalog(root.biomeDatabase());
  expectCatalog(root.liquidsDatabase());
  expectCatalog(root.statusEffectDatabase());
  expectCatalog(root.damageDatabase());
  expectCatalog(root.particleDatabase());
  expectCatalog(root.effectSourceDatabase());
  expectCatalog(root.functionDatabase());
  expectCatalog(root.treasureDatabase());
  expectCatalog(root.dungeonDefinitions());
  expectCatalog(root.emoteProcessor());
  expectCatalog(root.speciesDatabase());
  expectCatalog(root.imageMetadataDatabase());
  expectCatalog(root.versioningDatabase());
  expectCatalog(root.questTemplateDatabase());
  expectCatalog(root.aiDatabase());
  expectCatalog(root.techDatabase());
  expectCatalog(root.codexDatabase());
  expectCatalog(root.stagehandDatabase());
  expectCatalog(root.behaviorDatabase());
  expectCatalog(root.tenantDatabase());
  expectCatalog(root.danceDatabase());
  expectCatalog(root.spawnTypeDatabase());

  ASSERT_TRUE(root.reload());

  expectCatalog(root.assets());
  expectCatalog(root.objectDatabase());
  expectCatalog(root.plantDatabase());
  expectCatalog(root.projectileDatabase());
  expectCatalog(root.monsterDatabase());
  expectCatalog(root.npcDatabase());
  expectCatalog(root.playerFactory());
  expectCatalog(root.entityFactory());
  expectCatalog(root.nameGenerator());
  expectCatalog(root.itemDatabase());
  expectCatalog(root.materialDatabase());
  expectCatalog(root.terrainDatabase());
  expectCatalog(root.biomeDatabase());
  expectCatalog(root.liquidsDatabase());
  expectCatalog(root.statusEffectDatabase());
  expectCatalog(root.damageDatabase());
  expectCatalog(root.particleDatabase());
  expectCatalog(root.effectSourceDatabase());
  expectCatalog(root.functionDatabase());
  expectCatalog(root.treasureDatabase());
  expectCatalog(root.dungeonDefinitions());
  expectCatalog(root.emoteProcessor());
  expectCatalog(root.speciesDatabase());
  expectCatalog(root.imageMetadataDatabase());
  expectCatalog(root.versioningDatabase());
  expectCatalog(root.questTemplateDatabase());
  expectCatalog(root.aiDatabase());
  expectCatalog(root.techDatabase());
  expectCatalog(root.codexDatabase());
  expectCatalog(root.stagehandDatabase());
  expectCatalog(root.behaviorDatabase());
  expectCatalog(root.tenantDatabase());
  expectCatalog(root.danceDatabase());
  expectCatalog(root.spawnTypeDatabase());
});

[[nodiscard]] std::expected<void, StarException> writeIsolationAssets(String const& overlay, String const& owner) {
  auto config = Json(JsonObject{{"owner", owner}}).repr();
  if (!config) return std::unexpected(std::move(config.error()));
  auto writeConfig = File::writeFile(*config, File::relativeTo(overlay, "root-isolation.config"));
  if (!writeConfig) return std::unexpected(std::move(writeConfig.error()));
  auto tech = Json(JsonObject{
      {"name", "root-isolation"},
      {"type", "Head"},
      {"scripts", JsonArray{}},
      {"description", owner},
      {"shortDescription", owner},
      {"rarity", "common"},
      {"icon", "/interface/missing.png"}
    }).repr();
  if (!tech) return std::unexpected(std::move(tech.error()));
  auto writeTech = File::writeFile(*tech, File::relativeTo(overlay, "root-isolation.tech"));
  if (!writeTech) return std::unexpected(std::move(writeTech.error()));
  auto quotedOwner = Json(owner).repr();
  if (!quotedOwner) return std::unexpected(std::move(quotedOwner.error()));
  return File::writeFile("function isolationScriptOwner() return " + *quotedOwner + " end",
      File::relativeTo(overlay, "root-isolation.lua"));
}

[[nodiscard]] std::expected<Root::Settings, StarException> isolationSettings(
    Root::Settings const& base, String const& directory, String const& owner) {
  auto settings = base;
  auto overlay = File::relativeTo(directory, "assets");
  auto makeOverlay = File::makeDirectory(overlay);
  if (!makeOverlay) return std::unexpected(std::move(makeOverlay.error()));
  auto writeAssets = writeIsolationAssets(overlay, owner);
  if (!writeAssets) return std::unexpected(std::move(writeAssets.error()));
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
  auto configuration = settings.defaultConfiguration.set("rootIsolationOwner", owner);
  if (!configuration) return std::unexpected(std::move(configuration.error()));
  settings.defaultConfiguration = std::move(*configuration);
  return settings;
}

[[nodiscard]] std::expected<void, StarException> expectIsolationConsumers(Root& root, LuaContext& context,
    String const& owner, String const& scriptOwner, String const& configurationOwner) {
  auto assets = root.assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto config = (*assets)->json("/root-isolation.config");
  if (!config) return std::unexpected(std::move(config.error()));
  EXPECT_EQ(config->getString("owner"), owner);
  auto database = root.techDatabase();
  if (!database) return std::unexpected(std::move(database.error()));
  auto tech = (*database)->tech("root-isolation");
  if (!tech) return std::unexpected(std::move(tech.error()));
  EXPECT_EQ(tech->description, owner);
  auto configuration = root.configuration();
  if (!configuration) return std::unexpected(std::move(configuration.error()));
  auto value = (*configuration)->get("rootIsolationOwner");
  if (!value) return std::unexpected(std::move(value.error()));
  EXPECT_EQ(value->toString(), configurationOwner);
  EXPECT_EQ(context.eval<String>("root.assetJson('/root-isolation.config').owner"), owner);
  EXPECT_EQ(context.eval<String>("root.techConfig('root-isolation').description"), owner);
  EXPECT_EQ(context.eval<String>("root.getConfiguration('rootIsolationOwner')"), configurationOwner);
  EXPECT_EQ(context.invokePath<String>("isolationScriptOwner"), scriptOwner);
  return {};
}

[[nodiscard]] std::expected<std::shared_ptr<LuaRoot>, StarException> isolationLua(Root& root, bool utilities = false) {
  auto lua = LuaRoot::create(root);
  if (!lua) return std::unexpected(std::move(lua.error()));
  auto callbacks = LuaBindings::makeRootCallbacks(root);
  if (!callbacks) return std::unexpected(std::move(callbacks.error()));
  (*lua)->addCallbacks("root", *callbacks);
  if (utilities) {
    auto utilityCallbacks = LuaBindings::makeUtilityCallbacks(root.random(), root.logger(), root.logMap());
    if (!utilityCallbacks) return std::unexpected(std::move(utilityCallbacks.error()));
    (*lua)->addCallbacks("sb", *utilityCallbacks);
  }
  return std::move(*lua);
}

[[nodiscard]] std::expected<bool, StarException> isolationLogContains(String const& path, String const& text) {
  auto content = File::readFileString(path);
  if (!content) return std::unexpected(std::move(content.error()));
  return content->contains(text);
}

[[nodiscard]] std::expected<String, StarException> isolationStoredOwner(String const& directory) {
  auto content = File::readFileString(File::relativeTo(directory, "root-isolation.config"));
  if (!content) return std::unexpected(std::move(content.error()));
  auto json = Json::parseJson(*content);
  if (!json) return std::unexpected(std::move(json.error()));
  return json->getString("rootIsolationOwner");
}

const GameTestRegistration strictLuaTags("RootTest", "StrictLuaTagFormatting", [](GameTest& fixture) {
  auto lua = isolationLua(fixture.root(), true);
  ASSERT_TRUE(lua) << lua.error().what();
  auto context = (*lua)->createContext(StringList{});
  ASSERT_TRUE(context) << context.error().what();
  auto result = context->eval<Json>(R"LUA(
    local rejected = {}
    for i, text in ipairs({"<missing>", "<>", "<worldName"}) do
      rejected[i] = not pcall(sb.formatTags, text, {worldName = "日"})
    end
    return {
      text = sb.formatTags("<<worldName> <worldName> {not:std-format}",
        {worldName = "日%<literal>"}),
      binary = sb.formatTags("a\0<x>b", {x = "z\0q"}),
      rejected = rejected,
      recovered = sb.formatTags("<worldName>", {worldName = "again"})
    }
  )LUA");
  ASSERT_TRUE(result) << result.error().what();
  EXPECT_EQ(result->getString("text"), String("<worldName> 日%<literal> {not:std-format}"));
  EXPECT_EQ(result->getString("binary"), String(std::string("a\0z\0qb", 6)));
  EXPECT_EQ(result->get("rejected"), Json(JsonArray{true, true, true}));
  EXPECT_EQ(result->getString("recovered"), String("again"));
});

const GameTestRegistration simultaneousRoots("RootTest", "SimultaneousRoots", [](GameTest& fixture) {
  auto directoryA = File::temporaryDirectory(fixture.root().random());
  ASSERT_TRUE(directoryA) << directoryA.error().what();
  auto cleanupA = finally([&] { EXPECT_TRUE(File::removeDirectoryRecursive(*directoryA)); });
  auto directoryB = File::temporaryDirectory(fixture.root().random());
  ASSERT_TRUE(directoryB) << directoryB.error().what();
  auto cleanupB = finally([&] { EXPECT_TRUE(File::removeDirectoryRecursive(*directoryB)); });
  auto settingsResultA = isolationSettings(fixture.rootSettings(), *directoryA, "A");
  ASSERT_TRUE(settingsResultA) << settingsResultA.error().what();
  auto settingsA = std::move(*settingsResultA);
  auto settingsResultB = isolationSettings(fixture.rootSettings(), *directoryB, "B");
  ASSERT_TRUE(settingsResultB) << settingsResultB.error().what();
  auto settingsB = std::move(*settingsResultB);
  auto overlayA = File::relativeTo(*directoryA, "assets");
  auto overlayB = File::relativeTo(*directoryB, "assets");
  auto createdA = Root::create(settingsA);
  ASSERT_TRUE(createdA) << createdA.error().what();
  auto rootA = std::move(*createdA);
  rootA->logger().addSink(std::make_shared<RootIsolationErrorSink>());
  auto createdB = Root::create(settingsB);
  ASSERT_TRUE(createdB) << createdB.error().what();
  auto rootB = std::move(*createdB);
  rootB->logger().addSink(std::make_shared<RootIsolationErrorSink>());
  auto logPathA = File::relativeTo(*directoryA, "isolation.log");
  auto logPathB = File::relativeTo(*directoryB, "isolation.log");
  auto sinkA = FileLogSink::create(logPathA, LogLevel::Info, true);
  ASSERT_TRUE(sinkA) << sinkA.error().what();
  auto sinkB = FileLogSink::create(logPathB, LogLevel::Info, true);
  ASSERT_TRUE(sinkB) << sinkB.error().what();
  rootA->logger().addSink(*sinkA);
  rootB->logger().addSink(*sinkB);

  EXPECT_NE(rootA->toStoragePath("root-isolation.config"), rootB->toStoragePath("root-isolation.config"));
  {
    auto createdLuaA = isolationLua(*rootA, true);
    ASSERT_TRUE(createdLuaA) << createdLuaA.error().what();
    auto luaA = std::move(*createdLuaA);
    auto createdLuaB = isolationLua(*rootB, true);
    ASSERT_TRUE(createdLuaB) << createdLuaB.error().what();
    auto luaB = std::move(*createdLuaB);
    auto createdContextA = luaA->createContext("/root-isolation.lua");
    ASSERT_TRUE(createdContextA) << createdContextA.error().what();
    auto contextA = std::make_unique<LuaContext>(std::move(*createdContextA));
    auto contextB = luaB->createContext("/root-isolation.lua");
    ASSERT_TRUE(contextB) << contextB.error().what();
    ASSERT_TRUE(expectIsolationConsumers(*rootA, *contextA, "A", "A", "A"));
    ASSERT_TRUE(expectIsolationConsumers(*rootB, *contextB, "B", "B", "B"));

    // Exercise writes through the real Lua root table, not only C++ getters.
    ASSERT_TRUE(contextA->eval("root.setConfiguration('rootIsolationOwner', 'A-written')"));
    ASSERT_TRUE(contextB->eval("root.setConfiguration('rootIsolationOwner', 'B-written')"));
    ASSERT_TRUE(expectIsolationConsumers(*rootA, *contextA, "A", "A", "A-written"));
    ASSERT_TRUE(expectIsolationConsumers(*rootB, *contextB, "B", "B", "B-written"));
    ASSERT_TRUE(contextA->eval("sb.logInfo('root-isolation-log-A'); sb.setLogMap('root-isolation', 'A')"));
    ASSERT_TRUE(contextB->eval("sb.logInfo('root-isolation-log-B'); sb.setLogMap('root-isolation', 'B')"));
    EXPECT_EQ(rootA->logMap().getValue("root-isolation"), "A");
    EXPECT_EQ(rootB->logMap().getValue("root-isolation"), "B");
    rootA->logMap().clear();
    EXPECT_TRUE(rootA->logMap().getValues().empty());
    EXPECT_EQ(rootB->logMap().getValue("root-isolation"), "B");
    EXPECT_EQ(isolationLogContains(logPathA, "root-isolation-log-A"), true);
    EXPECT_EQ(isolationLogContains(logPathA, "root-isolation-log-B"), false);
    EXPECT_EQ(isolationLogContains(logPathB, "root-isolation-log-B"), true);
    EXPECT_EQ(isolationLogContains(logPathB, "root-isolation-log-A"), false);

    // A real filesystem failure must leave both live owners usable and must
    // not prevent a subsequent successful Root construction.
    auto failedSettings = settingsA;
    failedSettings.storageDirectory = File::relativeTo(*directoryA, "not-a-directory");
    ASSERT_TRUE(File::writeFile(String("regular file"), failedSettings.storageDirectory));
    auto failedRoot = Root::create(failedSettings);
    EXPECT_FALSE(failedRoot);
    auto recoveredSettings = settingsA;
    recoveredSettings.storageDirectory = File::relativeTo(*directoryA, "recovered-storage");
    {
      auto recovered = Root::create(recoveredSettings);
      ASSERT_TRUE(recovered) << recovered.error().what();
      (*recovered)->logger().addSink(std::make_shared<RootIsolationErrorSink>());
      auto recoveredLua = isolationLua(**recovered);
      ASSERT_TRUE(recoveredLua) << recoveredLua.error().what();
      auto recoveredContext = (*recoveredLua)->createContext("/root-isolation.lua");
      ASSERT_TRUE(recoveredContext) << recoveredContext.error().what();
      ASSERT_TRUE(expectIsolationConsumers(**recovered, *recoveredContext, "A", "A", "A"));
    }
    ASSERT_TRUE(expectIsolationConsumers(*rootA, *contextA, "A", "A", "A-written"));
    ASSERT_TRUE(expectIsolationConsumers(*rootB, *contextB, "B", "B", "B-written"));

    auto listenerA = std::make_shared<TrackerListener>();
    auto listenerB = std::make_shared<TrackerListener>();
    auto subscriptionA = rootA->registerReloadListener(listenerA);
    auto subscriptionB = rootB->registerReloadListener(listenerB);
    {
      auto assetsA = rootA->assets();
      ASSERT_TRUE(assetsA) << assetsA.error().what();
      auto assetsB = rootB->assets();
      ASSERT_TRUE(assetsB) << assetsB.error().what();
      auto databaseA = rootA->techDatabase();
      ASSERT_TRUE(databaseA) << databaseA.error().what();
      auto databaseB = rootB->techDatabase();
      ASSERT_TRUE(databaseB) << databaseB.error().what();
      ASSERT_TRUE(writeIsolationAssets(overlayA, "A-reloaded"));
      ASSERT_TRUE(writeIsolationAssets(overlayB, "B-reloaded"));

      ASSERT_TRUE(rootA->reload());
      EXPECT_TRUE(listenerA->pullTriggered());
      EXPECT_FALSE(listenerB->pullTriggered());
      EXPECT_FALSE(luaA->scriptLoaded("/root-isolation.lua"));
      EXPECT_TRUE(luaB->scriptLoaded("/root-isolation.lua"));
      auto reloadedAssetsA = rootA->assets();
      ASSERT_TRUE(reloadedAssetsA) << reloadedAssetsA.error().what();
      auto reloadedDatabaseA = rootA->techDatabase();
      ASSERT_TRUE(reloadedDatabaseA) << reloadedDatabaseA.error().what();
      auto retainedAssetsB = rootB->assets();
      ASSERT_TRUE(retainedAssetsB) << retainedAssetsB.error().what();
      auto retainedDatabaseB = rootB->techDatabase();
      ASSERT_TRUE(retainedDatabaseB) << retainedDatabaseB.error().what();
      EXPECT_NE(reloadedAssetsA->get(), assetsA->get());
      EXPECT_NE(reloadedDatabaseA->get(), databaseA->get());
      EXPECT_EQ(retainedAssetsB->get(), assetsB->get());
      EXPECT_EQ(retainedDatabaseB->get(), databaseB->get());
      auto retainedTechA = (*databaseA)->tech("root-isolation");
      ASSERT_TRUE(retainedTechA) << retainedTechA.error().what();
      EXPECT_EQ(retainedTechA->description, "A");
      // Existing closures follow their own owner's reloaded services, while
      // already instantiated Lua functions keep their original script body.
      ASSERT_TRUE(expectIsolationConsumers(*rootA, *contextA, "A-reloaded", "A", "A-written"));
      auto reloadedContextA = luaA->createContext("/root-isolation.lua");
      ASSERT_TRUE(reloadedContextA) << reloadedContextA.error().what();
      ASSERT_TRUE(expectIsolationConsumers(*rootA, *reloadedContextA, "A-reloaded", "A-reloaded", "A-written"));
      ASSERT_TRUE(expectIsolationConsumers(*rootB, *contextB, "B", "B", "B-written"));
      auto cachedContextB = luaB->createContext("/root-isolation.lua");
      ASSERT_TRUE(cachedContextB) << cachedContextB.error().what();
      ASSERT_TRUE(expectIsolationConsumers(*rootB, *cachedContextB, "B", "B", "B-written"));
    }

    // Destroy all A script borrowers before A. B remains live throughout.
    contextA.reset();
    luaA.reset();
    subscriptionA.disconnect();
    rootA.reset();
    EXPECT_FALSE(listenerB->pullTriggered());
    ASSERT_TRUE(expectIsolationConsumers(*rootB, *contextB, "B", "B", "B-written"));
    ASSERT_TRUE(contextB->eval("root.setConfiguration('rootIsolationOwner', 'B-survives')"));
    ASSERT_TRUE(expectIsolationConsumers(*rootB, *contextB, "B", "B", "B-survives"));
    EXPECT_EQ(rootB->logMap().getValue("root-isolation"), "B");
    ASSERT_TRUE(contextB->eval("sb.logInfo('root-isolation-log-B-survives'); sb.setLogMap('root-isolation', 'B-survives')"));
    EXPECT_EQ(rootB->logMap().getValue("root-isolation"), "B-survives");
    EXPECT_EQ(isolationLogContains(logPathB, "root-isolation-log-B-survives"), true);
    EXPECT_EQ(isolationLogContains(logPathA, "root-isolation-log-B-survives"), false);

    ASSERT_TRUE(rootB->reload());
    EXPECT_TRUE(listenerB->pullTriggered());
    EXPECT_FALSE(luaB->scriptLoaded("/root-isolation.lua"));
    ASSERT_TRUE(expectIsolationConsumers(*rootB, *contextB, "B-reloaded", "B", "B-survives"));
    auto reloadedContextB = luaB->createContext("/root-isolation.lua");
    ASSERT_TRUE(reloadedContextB) << reloadedContextB.error().what();
    ASSERT_TRUE(expectIsolationConsumers(*rootB, *reloadedContextB, "B-reloaded", "B-reloaded", "B-survives"));
  }
  rootB.reset();
  EXPECT_EQ(isolationStoredOwner(settingsA.storageDirectory), "A-written");
  EXPECT_EQ(isolationStoredOwner(settingsB.storageDirectory), "B-survives");
});

[[nodiscard]] std::expected<void, StarException> writeIsolationMaterial(
    String const& overlay, MaterialId material, String const& collisionKind) {
  // These are the required MaterialDatabase parser fields. Omit the optional
  // render profile: the consumer is the real headless world's collision path.
  auto content = Json(JsonObject{
      {"materialId", material},
      {"materialName", "root-isolation"},
      {"category", "materials"},
      {"collisionKind", collisionKind}
    }).repr();
  if (!content) return std::unexpected(std::move(content.error()));
  return File::writeFile(*content, File::relativeTo(overlay, "root-isolation.material"));
}

[[nodiscard]] std::expected<void, StarException> placeIsolationMaterial(
    WorldServer& world, Vec2I const& position, MaterialId material, bool collides) {
  auto generated = world.generateRegion(RectI::withSize(position - Vec2I(4, 4), Vec2I(9, 9)));
  if (!generated) return std::unexpected(std::move(generated.error()));
  world.setTileProtectionEnabled(false);
  auto tile = world.modifyServerTile(position, true);
  if (!tile) return std::unexpected(std::move(tile.error()));
  if (!*tile) return std::unexpected(StarException("Isolation material tile is unavailable"));
  EXPECT_EQ((*tile)->get().foreground, EmptyMaterialId);
  if ((*tile)->get().foreground != EmptyMaterialId)
    return std::unexpected(StarException("Isolation material tile is not empty"));
  auto center = Vec2F(position) + Vec2F(0.5f, 0.5f);
  EXPECT_EQ(world.pointTileCollision(center), false);
  // Supply a legitimate attachment surface, then let WorldServer perform the
  // placement and derive its collision from this world's MaterialDatabase.
  (*tile)->get().background = StructureMaterialId;
  auto placed = world.forceModifyTile(
      position, PlaceMaterial{TileLayer::Foreground, material, MaterialHue(0)}, true);
  if (!placed) return std::unexpected(std::move(placed.error()));
  if (!*placed) return std::unexpected(StarException("Isolation material placement was rejected"));
  EXPECT_EQ(world.material(position, TileLayer::Foreground), material);
  EXPECT_EQ(world.pointTileCollision(center), collides);
  EXPECT_EQ(world.lineTileCollision(center - Vec2F(0.25f, 0), center + Vec2F(0.25f, 0)), collides);
  return {};
}

const GameTestRegistration simultaneousWorldMaterials("RootTest", "SimultaneousWorldMaterials", [](GameTest& fixture) {
  // Select a genuinely unused real ID, rather than assuming a mod collection
  // leaves a hard-coded material slot free.
  auto materials = fixture.root().materialDatabase();
  ASSERT_TRUE(materials) << materials.error().what();
  MaterialId material = 0;
  while (material < FirstMetaMaterialId && (*materials)->isValidMaterialId(material))
    ++material;
  ASSERT_LT(material, FirstMetaMaterialId);

  auto directoryA = File::temporaryDirectory(fixture.root().random());
  ASSERT_TRUE(directoryA) << directoryA.error().what();
  auto cleanupA = finally([&] { EXPECT_TRUE(File::removeDirectoryRecursive(*directoryA)); });
  auto directoryB = File::temporaryDirectory(fixture.root().random());
  ASSERT_TRUE(directoryB) << directoryB.error().what();
  auto cleanupB = finally([&] { EXPECT_TRUE(File::removeDirectoryRecursive(*directoryB)); });
  auto settingsA = isolationSettings(fixture.rootSettings(), *directoryA, "A");
  ASSERT_TRUE(settingsA) << settingsA.error().what();
  auto settingsB = isolationSettings(fixture.rootSettings(), *directoryB, "B");
  ASSERT_TRUE(settingsB) << settingsB.error().what();
  auto overlayA = File::relativeTo(*directoryA, "assets");
  auto overlayB = File::relativeTo(*directoryB, "assets");
  ASSERT_TRUE(writeIsolationMaterial(overlayA, material, "block"));
  ASSERT_TRUE(writeIsolationMaterial(overlayB, material, "none"));

  auto createdA = Root::create(*settingsA);
  ASSERT_TRUE(createdA) << createdA.error().what();
  auto rootA = std::move(*createdA);
  rootA->logger().addSink(std::make_shared<RootIsolationErrorSink>());
  auto createdB = Root::create(*settingsB);
  ASSERT_TRUE(createdB) << createdB.error().what();
  auto rootB = std::move(*createdB);
  rootB->logger().addSink(std::make_shared<RootIsolationErrorSink>());
  auto storageA = File::ephemeralFile(rootA->random());
  ASSERT_TRUE(storageA) << storageA.error().what();
  auto createdWorldA = WorldServer::create(*rootA, Vec2U(2048, 2048), *storageA);
  ASSERT_TRUE(createdWorldA) << createdWorldA.error().what();
  auto worldA = std::move(*createdWorldA);
  auto closeWorldA = finally([&] { if (worldA) EXPECT_TRUE(worldA->close()); });
  auto storageB = File::ephemeralFile(rootB->random());
  ASSERT_TRUE(storageB) << storageB.error().what();
  auto createdWorldB = WorldServer::create(*rootB, Vec2U(2048, 2048), *storageB);
  ASSERT_TRUE(createdWorldB) << createdWorldB.error().what();
  auto worldB = std::move(*createdWorldB);
  auto closeWorldB = finally([&] { EXPECT_TRUE(worldB->close()); });
  {
    auto materialsA = rootA->materialDatabase();
    ASSERT_TRUE(materialsA) << materialsA.error().what();
    auto materialsB = rootB->materialDatabase();
    ASSERT_TRUE(materialsB) << materialsB.error().what();
    EXPECT_EQ((*materialsA)->materialId("root-isolation"), material);
    EXPECT_EQ((*materialsB)->materialId("root-isolation"), material);
  }

  // Identical material IDs and placement requests produce different actual
  // world collision behavior, not merely different catalog properties.
  ASSERT_TRUE(placeIsolationMaterial(*worldA, Vec2I(1024, 1024), material, true));
  ASSERT_TRUE(placeIsolationMaterial(*worldB, Vec2I(1024, 1024), material, false));
  ASSERT_TRUE(writeIsolationMaterial(overlayA, material, "none"));
  ASSERT_TRUE(writeIsolationMaterial(overlayB, material, "block"));
  ASSERT_TRUE(rootA->reload());
  ASSERT_TRUE(placeIsolationMaterial(*worldA, Vec2I(1032, 1024), material, false));
  ASSERT_TRUE(placeIsolationMaterial(*worldB, Vec2I(1032, 1024), material, false));

  ASSERT_TRUE(worldA->close());
  worldA.reset();
  rootA.reset();
  ASSERT_TRUE(placeIsolationMaterial(*worldB, Vec2I(1040, 1024), material, false));
  ASSERT_TRUE(rootB->reload());
  ASSERT_TRUE(placeIsolationMaterial(*worldB, Vec2I(1048, 1024), material, true));
});

const GameTestRegistration animatorPartDrawableFirstAdd("AnimatorTest", "PartDrawableFirstAdd", [](GameTest& fixture) {
  auto config = Json::parseJson(R"JSON({"animatedParts":{"parts":{"body":{"properties":{}}}}})JSON");
  ASSERT_TRUE(config) << config.error().what();
  for (bool scripted : {false, true}) {
    for (bool emptyFirst : {false, true}) {
      SCOPED_TRACE(scripted ? "scripted" : "networked");
      SCOPED_TRACE(emptyFirst ? "empty first add" : "nonempty first add");
      auto created = NetworkedAnimator::create(fixture.root(), *config);
      ASSERT_TRUE(created) << created.error().what();
      auto animator = std::move(*created);
      auto callbacks = scripted
          ? LuaBindings::makeScriptedAnimatorCallbacks(*animator,
              [](String const&, Json const& fallback) -> std::expected<Json, StarException> { return fallback; })
          : LuaBindings::makeNetworkedAnimatorCallbacks(*animator);
      ASSERT_TRUE(callbacks) << callbacks.error().what();
      auto engine = LuaEngine::create();
      ASSERT_TRUE(engine) << engine.error().what();
      (*engine)->setExecutionHost(std::make_unique<LuaExecutionScope>(fixture.root()));
      auto contextResult = (*engine)->createContext();
      ASSERT_TRUE(contextResult) << contextResult.error().what();
      auto context = std::move(*contextResult);
      ASSERT_TRUE(context.setCallbacks("animator", *callbacks));

      auto widths = [&]() -> std::expected<std::vector<float>, StarException> {
        auto drawables = animator->drawables();
        if (!drawables) return std::unexpected(std::move(drawables.error()));
        std::vector<float> result;
        for (auto const& drawable : *drawables) {
          auto json = drawable.toJson();
          if (!json) return std::unexpected(std::move(json.error()));
          auto width = json->getFloat("width");
          if (!width) return std::unexpected(std::move(width.error()));
          result.push_back(*width);
        }
        return result;
      };
      if (emptyFirst) {
        ASSERT_TRUE(context.load("animator.addPartJsonDrawables('body', {})"));
        auto output = widths();
        ASSERT_TRUE(output) << output.error().what();
        EXPECT_TRUE(output->empty());
      }
      ASSERT_TRUE(context.load(R"LUA(animator.addPartJsonDrawables('body', {{line = {{0,0}, {1,1}}, width = 11}}))LUA"));
      auto firstOutput = widths();
      ASSERT_TRUE(firstOutput) << firstOutput.error().what();
      EXPECT_EQ(*firstOutput, (std::vector<float>{11}));
      ASSERT_TRUE(context.load(R"LUA(animator.addPartJsonDrawables('body', {{line = {{0,0}, {2,2}}, width = 22}}))LUA"));
      auto appendedOutput = widths();
      ASSERT_TRUE(appendedOutput) << appendedOutput.error().what();
      EXPECT_EQ(*appendedOutput, (std::vector<float>{11, 22}));

      for (std::string_view operation : {"addPartJsonDrawables", "setPartJsonDrawables"}) {
        String script = String("animator.") + operation
            + R"LUA(('body', {{line = {{0,0}, {3,3}}, width = 33}, {line = {{0,0}, {4,4}}, width = 'invalid'}}))LUA";
        auto failed = context.load(script);
        EXPECT_FALSE(failed);
        EXPECT_EQ(context.eval<int>("6 * 7"), 42);
        auto output = widths();
        ASSERT_TRUE(output) << output.error().what();
        EXPECT_EQ(*output, (std::vector<float>{11, 22}));
      }
    }
  }
});

// Use authored items with actual handedness instead of test-only overrides.
std::expected<std::shared_ptr<Item>, StarException> hudAuthoredItem(
    ItemDatabase const& database, ItemType requiredType, bool twoHanded) {
  auto names = database.allItems();
  names.sort();
  for (auto const& name : names) {
    auto type = database.itemType(name);
    if (!type) return std::unexpected(std::move(type.error()));
    if (*type != requiredType)
      continue;
    auto item = database.fromJson(name);
    if (!item) return std::unexpected(std::move(item.error()));
    if (*item && (*item)->twoHanded() == twoHanded)
      return *item;
  }
  return std::unexpected(ItemException("HUD inventory regression requires an authored item with the requested type and handedness"));
}

std::expected<BagSlot, StarException> hudItemBagSlot(Root& root, PlayerInventory const& inventory,
    std::shared_ptr<Item> const& item) {
  auto assets = root.assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto config = (*assets)->json("/player.config:inventory.itemBags");
  if (!config) return std::unexpected(std::move(config.error()));
  auto bags = config->toObject();
  if (!bags) return std::unexpected(std::move(bags.error()));
  auto names = bags->keys();
  names.sort();
  for (auto const& name : names) {
    auto bag = inventory.bagContents(name);
    if (!bag || bag->size() < 2)
      continue;
    auto allowed = inventory.itemAllowedInBag(item, name);
    if (!allowed) return std::unexpected(std::move(allowed.error()));
    if (*allowed)
      // An empty earlier slot must not steal the cursor's native return slot.
      return BagSlot(name, static_cast<std::uint8_t>(bag->size() - 1));
  }
  return std::unexpected(ItemException("HUD inventory regression requires a configured accepting bag"));
}

const GameTestRegistration hudInventoryLinks("HudDomainTest", "InventoryLinksPreserveCursorOwnership", [](GameTest& fixture) {
  auto& root = fixture.root();
  auto database = root.itemDatabase();
  ASSERT_TRUE(database) << database.error().what();
  auto created = PlayerInventory::create(root);
  ASSERT_TRUE(created) << created.error().what();
  auto& inventory = **created;
  ASSERT_GE(inventory.customBarIndexes(), 2);
  auto expectSlot = [](std::expected<std::optional<InventorySlot>, StarException> const& actual,
      std::optional<InventorySlot> const& expected) {
    ASSERT_TRUE(actual) << actual.error().what();
    EXPECT_TRUE(*actual == expected);
  };
  auto expectItem = [](std::expected<std::shared_ptr<Item>, StarException> const& actual, Json const& descriptor) {
    ASSERT_TRUE(actual) << actual.error().what();
    if (descriptor.isNull()) {
      EXPECT_FALSE(*actual);
      return;
    }
    ASSERT_TRUE(*actual);
    EXPECT_EQ((*actual)->descriptor().toJson(), descriptor);
  };
  auto oneHanded = hudAuthoredItem(**database, ItemType::Consumable, false);
  ASSERT_TRUE(oneHanded) << oneHanded.error().what();
  auto twoHanded = hudAuthoredItem(**database, ItemType::MiningTool, true);
  ASSERT_TRUE(twoHanded) << twoHanded.error().what();
  Json const oneDescriptor = (*oneHanded)->descriptor().toJson();
  Json const twoDescriptor = (*twoHanded)->descriptor().toJson();
  auto oneSlot = hudItemBagSlot(root, inventory, *oneHanded);
  auto twoSlot = hudItemBagSlot(root, inventory, *twoHanded);
  ASSERT_TRUE(oneSlot) << oneSlot.error().what();
  ASSERT_TRUE(twoSlot) << twoSlot.error().what();
  if (*oneSlot == *twoSlot)
    --twoSlot->second;
  ASSERT_EQ(inventory.setItem(*oneSlot, *oneHanded), true);
  ASSERT_EQ(inventory.setItem(*twoSlot, *twoHanded), true);
  ASSERT_TRUE(inventory.setCustomBarSecondarySlot(1, InventorySlot(*oneSlot)));
  ASSERT_TRUE(inventory.shiftSwap(*oneSlot));
  expectSlot(inventory.customBarSecondarySlot(1),
      std::optional<InventorySlot>(InventorySlot(SwapSlot())));
  ASSERT_TRUE(inventory.linkCustomBarSwap(0, false));
  EXPECT_FALSE(inventory.swapSlotItem());
  expectItem(inventory.itemsAt(*oneSlot), oneDescriptor);
  expectSlot(inventory.customBarSecondarySlot(0), std::optional<InventorySlot>(InventorySlot(*oneSlot)));
  expectSlot(inventory.customBarSecondarySlot(1), std::optional<InventorySlot>(InventorySlot(*oneSlot)));

  ASSERT_TRUE(inventory.setCustomBarPrimarySlot(1, InventorySlot(*twoSlot)));
  ASSERT_TRUE(inventory.shiftSwap(*twoSlot));
  auto beforeInvalid = inventory.store();
  ASSERT_TRUE(beforeInvalid) << beforeInvalid.error().what();
  for (bool primary : {false, true}) {
    EXPECT_FALSE(inventory.linkCustomBarSwap(inventory.customBarIndexes(), primary));
    EXPECT_FALSE(inventory.clearCustomBarLink(inventory.customBarIndexes(), primary));
    auto afterInvalid = inventory.store();
    ASSERT_TRUE(afterInvalid) << afterInvalid.error().what();
    EXPECT_EQ(*afterInvalid, *beforeInvalid);
  }
  // Dropping onto the right half redirects a two-handed tool to primary,
  // returns it to its exact bag slot, and retargets all existing cursor links.
  ASSERT_TRUE(inventory.linkCustomBarSwap(0, false));
  EXPECT_FALSE(inventory.swapSlotItem());
  expectItem(inventory.itemsAt(*twoSlot), twoDescriptor);
  expectSlot(inventory.customBarPrimarySlot(0), std::optional<InventorySlot>(InventorySlot(*twoSlot)));
  expectSlot(inventory.customBarPrimarySlot(1), std::optional<InventorySlot>(InventorySlot(*twoSlot)));
  expectSlot(inventory.customBarSecondarySlot(0), std::optional<InventorySlot>(InventorySlot(*oneSlot)));
  inventory.selectActionBarLocation(CustomBarIndex(0));
  expectItem(inventory.primaryHeldItem(), twoDescriptor);
  expectItem(inventory.secondaryHeldItem(), {});
  ASSERT_TRUE(inventory.clearCustomBarLink(0, false));
  expectSlot(inventory.customBarPrimarySlot(0), std::optional<InventorySlot>{});
  expectItem(inventory.secondaryHeldItem(), oneDescriptor);
  expectItem(inventory.itemsAt(*twoSlot), twoDescriptor);
  expectSlot(inventory.customBarPrimarySlot(1), std::optional<InventorySlot>(InventorySlot(*twoSlot)));
  ASSERT_TRUE(inventory.clearCustomBarLink(0, false));
  expectSlot(inventory.customBarSecondarySlot(0), std::optional<InventorySlot>{});
  expectItem(inventory.itemsAt(*oneSlot), oneDescriptor);
  expectSlot(inventory.customBarSecondarySlot(1), std::optional<InventorySlot>(InventorySlot(*oneSlot)));
  auto items = inventory.allItems();
  ASSERT_TRUE(items) << items.error().what();
  EXPECT_EQ(items->size(), 2u);
  // Empty-cursor clicks select a valid cell, but cannot install an invalid
  // selection that would make subsequent held-item queries fail.
  ASSERT_TRUE(inventory.linkCustomBarSwap(1, false));
  EXPECT_TRUE(inventory.selectedActionBarLocation() == SelectedActionBarLocation(CustomBarIndex(1)));
  auto beforeEmptyInvalid = inventory.store();
  ASSERT_TRUE(beforeEmptyInvalid) << beforeEmptyInvalid.error().what();
  for (bool primary : {false, true}) {
    EXPECT_FALSE(inventory.linkCustomBarSwap(inventory.customBarIndexes(), primary));
    EXPECT_FALSE(inventory.clearCustomBarLink(inventory.customBarIndexes(), primary));
    auto afterEmptyInvalid = inventory.store();
    ASSERT_TRUE(afterEmptyInvalid) << afterEmptyInvalid.error().what();
    EXPECT_EQ(*afterEmptyInvalid, *beforeEmptyInvalid);
  }
});

const GameTestRegistration hudRadioDequeue("HudDomainTest", "RadioUniqueAccountingOccursAtDequeue", [](GameTest& fixture) {
  auto factory = fixture.root().playerFactory();
  ASSERT_TRUE(factory) << factory.error().what();
  auto created = (*factory)->create();
  ASSERT_TRUE(created) << created.error().what();
  auto& player = **created;
  auto message = [](String const& id, bool unique, bool important = false) {
    return Json(JsonObject{{"messageId", id}, {"type", "generic"}, {"unique", unique},
        {"important", important}, {"text", id}, {"portraitImage", ""}});
  };
  auto unique = message("hud-domain-unique", true);
  ASSERT_TRUE(player.queueRadioMessage(unique));
  ASSERT_TRUE(player.queueRadioMessage(unique));
  EXPECT_FALSE(player.log()->radioMessages().contains("hud-domain-unique"));
  ASSERT_TRUE(player.queueRadioMessage(message("hud-domain-normal", false)));
  ASSERT_TRUE(player.queueRadioMessage(message("hud-domain-important-a", true, true)));
  ASSERT_TRUE(player.queueRadioMessage(message("hud-domain-important-b", true, true)));
  EXPECT_TRUE(player.interruptRadioMessage());
  EXPECT_FALSE(player.interruptRadioMessage());
  for (auto const& id : {String("hud-domain-important-b"), String("hud-domain-important-a"),
      String("hud-domain-unique"), String("hud-domain-normal")}) {
    EXPECT_FALSE(player.log()->radioMessages().contains(id));
    auto next = player.pullPendingRadioMessage();
    ASSERT_TRUE(next) << next.error().what();
    ASSERT_TRUE(*next);
    EXPECT_EQ((**next).messageId, id);
    EXPECT_EQ(player.log()->radioMessages().contains(id), id != "hud-domain-normal");
  }
  auto empty = player.pullPendingRadioMessage();
  ASSERT_TRUE(empty);
  EXPECT_FALSE(*empty); // The duplicate pending unique message was not replayed.
  ASSERT_TRUE(player.queueRadioMessage(unique));
  empty = player.pullPendingRadioMessage();
  ASSERT_TRUE(empty);
  EXPECT_FALSE(*empty); // Dequeue, not UI completion, suppresses later resends.
  for (int replay = 0; replay != 2; ++replay) {
    ASSERT_TRUE(player.queueRadioMessage(message("hud-domain-normal", false)));
    auto next = player.pullPendingRadioMessage();
    ASSERT_TRUE(next && *next);
    EXPECT_EQ((**next).messageId, String("hud-domain-normal"));
    EXPECT_FALSE(player.log()->radioMessages().contains("hud-domain-normal"));
  }
  // The consumed unique set is player-owned, not a shared queue/database cache.
  auto other = (*factory)->create();
  ASSERT_TRUE(other) << other.error().what();
  ASSERT_TRUE((*other)->queueRadioMessage(unique));
  auto next = (*other)->pullPendingRadioMessage();
  ASSERT_TRUE(next && *next);
  EXPECT_EQ((**next).messageId, String("hud-domain-unique"));
});

const GameTestRegistration hudWorldViews("HudDomainTest", "WorldPortraitDeltasFollowNativeEntityIdentity", [](GameTest& fixture) {
  auto& root = fixture.root();
  auto storage = File::ephemeralFile(root.random());
  ASSERT_TRUE(storage) << storage.error().what();
  auto world = WorldServer::create(root, Vec2U(2048, 2048), *storage);
  ASSERT_TRUE(world) << world.error().what();
  auto close = finally([&] { EXPECT_TRUE((*world)->close()); });
  auto database = root.npcDatabase();
  ASSERT_TRUE(database) << database.error().what();
  auto variant = (*database)->generateNpcVariant("human", "villager", 1.0f, 1234, JsonObject());
  ASSERT_TRUE(variant) << variant.error().what();
  auto first = (*database)->createNpc(*variant);
  auto second = (*database)->createNpc(*variant);
  ASSERT_TRUE(first) << first.error().what();
  ASSERT_TRUE(second) << second.error().what();
  (*first)->setPosition(Vec2F(2046, 1024));
  (*second)->setPosition(Vec2F(100, 1024));
  ASSERT_TRUE((*world)->addEntity(*first));
  ASSERT_TRUE((*world)->addEntity(*second));
  auto luaRoot = LuaRoot::create(root);
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  auto context = (*luaRoot)->createContext(StringList{});
  ASSERT_TRUE(context) << context.error().what();
  auto callbacks = LuaBindings::makeWorldCallbacks(root, **world);
  ASSERT_TRUE(callbacks) << callbacks.error().what();
  ASSERT_TRUE(context->setCallbacks("world", *callbacks));
  ASSERT_TRUE(context->set("firstId", (*first)->entityId()));
  ASSERT_TRUE(context->set("secondId", (*second)->entityId()));
  auto discovery = context->eval<bool>(R"LUA(
    local all = world.damageBarEntities()
    local ids = {}
    for _, entry in ipairs(all) do ids[entry.id] = true end
    assert(ids[firstId] and ids[secondId])
    local wrapped = world.damageBarEntities({-4, 1000, 4, 1048}, "Default")
    assert(#wrapped == 1 and wrapped[1].id == firstId)
    assert(#world.damageBarEntities(nil, "Special") == 0)
    assert(#world.damageBarEntities({500, 1000, 510, 1048}) == 0)
    return true
  )LUA");
  ASSERT_TRUE(discovery) << discovery.error().what();
  EXPECT_TRUE(*discovery);
  auto initial = context->eval<bool>(R"LUA(
    initial = world.entityPortraitView(firstId, "full")
    assert(initial and #initial.drawables > 0 and initial.bounds)
    local delta = world.entityPortraitView(firstId, "full", initial.revision)
    return delta.revision == initial.revision and delta.drawables == nil and delta.bounds == nil
  )LUA");
  ASSERT_TRUE(initial) << initial.error().what();
  EXPECT_TRUE(*initial);
  ASSERT_TRUE((*first)->statusController().setResourcePercentage("health", 0.25f));
  auto health = context->eval<bool>(R"LUA(
    local bar = world.entityDamageBar(firstId)
    assert(bar.maxHealth > 0 and math.abs(bar.health / bar.maxHealth - 0.25) < 0.00001)
    local delta = world.entityPortraitView(firstId, "full", initial.revision)
    return delta.revision == initial.revision and delta.drawables == nil
  )LUA");
  ASSERT_TRUE(health) << health.error().what();
  EXPECT_TRUE(*health);
  ASSERT_TRUE((*first)->setBodyDirectives("?multiply=ff0000"));
  auto changed = context->eval<bool>(R"LUA(
    changed = world.entityPortraitView(firstId, "full", initial.revision)
    assert(changed.revision ~= initial.revision and changed.drawables and changed.bounds)
    local stable = world.entityPortraitView(firstId, "full", changed.revision)
    assert(stable.revision == changed.revision and stable.drawables == nil)
    local head = world.entityPortraitView(firstId, "head", changed.revision)
    assert(head.revision ~= changed.revision and head.drawables and head.bounds)
    local full = world.entityPortraitView(firstId, "full", head.revision)
    assert(full.revision ~= head.revision and full.drawables)
    other = world.entityPortraitView(secondId, "full", full.revision)
    return other.revision ~= full.revision and other.drawables ~= nil
  )LUA");
  ASSERT_TRUE(changed) << changed.error().what();
  EXPECT_TRUE(*changed);
  auto firstId = (*first)->entityId();
  ASSERT_TRUE((*world)->removeEntity(firstId, false));
  auto missing = context->eval<bool>(
      "return world.entityDamageBar(firstId) == nil and world.entityPortraitView(firstId, 'full') == nil");
  ASSERT_TRUE(missing) << missing.error().what();
  EXPECT_TRUE(*missing);
  auto replacement = (*database)->createNpc(*variant);
  ASSERT_TRUE(replacement) << replacement.error().what();
  (*replacement)->setPosition(Vec2F(2046, 1024));
  ASSERT_TRUE((*world)->addEntity(*replacement, firstId));
  // The two allocations have identical authored portraits; switching providers
  // must still invalidate the token, including a reused entity ID.
  auto replaced = context->eval<bool>(R"LUA(
    local view = world.entityPortraitView(firstId, "full", other.revision)
    assert(view.revision ~= other.revision and view.drawables and view.bounds)
    local stable = world.entityPortraitView(firstId, "full", view.revision)
    return stable.revision == view.revision and stable.drawables == nil and stable.bounds == nil
  )LUA");
  ASSERT_TRUE(replaced) << replaced.error().what();
  EXPECT_TRUE(*replaced);
});

}
