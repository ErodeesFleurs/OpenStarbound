import star.json;
import star.xx_hash;
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
import star.time;
import star.version;
import star.file;
import star.id_map;
import star.casting;
import star.variant;
import star.rpc_promise;
import star.strong_typedef;
import star.game_types;
import star.vector;
import star.a_star;
import star.ordered_set;
import star.audio;
import star.map;
import star.set;
import star.color;
import star.string;
import star.either;
import star.random;
import star.periodic_function;
import star.image;
import star.weighted_pool;
import star.net_compatibility;
import std;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.data_stream_devices;
import star.btree_database;
import star.lua;

import star.test_universe;
import star.universe_client;
import star.universe_server;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.statistics_service;
import star.uuid;
import star.item_descriptor;
import star.drawable;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.humanoid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.lua_root;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inspectable_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.game_timers;
import star.actor_movement_controller;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;
import star.liquid_types;
import star.tile_sector_array;
import star.weather_types;
import star.sky_types;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.chat_types;
import star.warping;
import star.wiring;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;

import star.player_storage;
import star.statistics;
import star.client_context;
import star.player_factory;


namespace Star {

TestUniverse::TestUniverse(Root& root, Vec2U clientWindowSize) {

  m_clientWindowSize = clientWindowSize;

  m_storagePath = File::temporaryDirectory(root.random());
  auto playerStorage = std::make_shared<PlayerStorage>(root, File::relativeTo(m_storagePath, "player"));
  auto statistics = std::make_shared<Statistics>(root, File::relativeTo(m_storagePath, "statistics"));
  m_server = std::make_shared<UniverseServer>(root, File::relativeTo(m_storagePath, "universe"));
  m_client = std::make_shared<UniverseClient>(root, playerStorage, statistics, File::relativeTo(m_storagePath, "universeclient"));

  m_server->start();

  m_mainPlayer = root.playerFactory()->create();
  m_mainPlayer->finalizeCreation();
  // A player only carries a ship species once it has been saved and loaded
  // (Player::diskStore falls back to the identity species, Player's save/load
  // ctor reads it back), which is what the real client always does
  // (StarClientApplication::loadPlayer).  The connect packet sends the ship
  // species and the server builds the ship world from it, so a player that was
  // only created in memory cannot connect: the server fails to create its ship
  // world and drops the connection.
  m_mainPlayer->setShipSpecies(m_mainPlayer->species());
  m_mainPlayer->setAdmin(true);
  m_mainPlayer->setModeType(PlayerMode::Survival);
  m_client->setMainPlayer(m_mainPlayer);
  // The real client supplies input callbacks bound to its explicit Input owner.
  // This headless fixture has no input device or client Input, so provide
  // callbacks where nothing is ever bound. OpenStarbound's player
  // scripts (e.g. /scripts/opensb/player/copy_paste.lua) call input.bindDown
  // from update() and would otherwise log a Lua error every frame.
  LuaCallbacks inputCallbacks;
  auto bindState = [](String const&, String const&) -> std::optional<unsigned> { return {}; };
  auto noKey = [](String const&) -> std::optional<unsigned> { return {}; };
  auto noButton = [](String const&) -> std::optional<List<Vec2F>> { return {}; };
  inputCallbacks.registerCallback("bindDown", bindState);
  inputCallbacks.registerCallback("bindUp", bindState);
  inputCallbacks.registerCallback("bindHeld", [](String const&, String const&) -> bool { return false; });
  inputCallbacks.registerCallback("bind", [](String const&, String const&) -> bool { return false; });
  inputCallbacks.registerCallback("keyDown", noKey);
  inputCallbacks.registerCallback("keyUp", noKey);
  inputCallbacks.registerCallback("keyHeld", [](String const&) -> bool { return false; });
  inputCallbacks.registerCallback("key", [](String const&) -> bool { return false; });
  inputCallbacks.registerCallback("mouseDown", noButton);
  inputCallbacks.registerCallback("mouseUp", noButton);
  inputCallbacks.registerCallback("mouseHeld", [](String const&) -> bool { return false; });
  inputCallbacks.registerCallback("mouse", [](String const&) -> bool { return false; });
  inputCallbacks.registerCallback("mousePosition", []() -> Vec2F { return {}; });
  inputCallbacks.registerCallback("getTag", [](String const&) -> unsigned { return 0; });
  inputCallbacks.registerCallback("events", []() -> Json { return JsonArray(); });
  inputCallbacks.registerCallback("resetBinds", [](String const&, String const&) {});
  inputCallbacks.registerCallback("setBinds", [](String const&, String const&, Json const&) {});
  inputCallbacks.registerCallback("getDefaultBinds", [](String const&, String const&) -> Json { return JsonObject(); });
  inputCallbacks.registerCallback("getBinds", [](String const&, String const&) -> Json { return JsonObject(); });
  m_client->setLuaCallbacks("input", inputCallbacks);
  m_client->connect(m_server->addLocalClient(), "test", "");
}

TestUniverse::~TestUniverse() {
  m_client = {};
  m_server = {};
  m_mainPlayer = {};
  File::removeDirectoryRecursive(m_storagePath);
}

void TestUniverse::warpPlayer(WorldId worldId) {
  // The client context only exists once the server has answered the connect
  // handshake, and both the warp call and the caller's expectations read it, so
  // let the client settle first.
  std::int64_t connectStart = Time::monotonicMilliseconds();
  while (!m_client->clientContext() && Time::monotonicMilliseconds() - connectStart < 60000) {
    m_client->update(0.016f);
    Thread::sleep(16);
  }

  m_client->warpPlayer(WarpToWorld(worldId), true);
  // Bounded by wall clock: if the world cannot be loaded the player stays
  // teleporting forever, so give up after five minutes and let the caller's
  // expectations report the failure instead of hanging the test.  A ship flight
  // to a celestial world legitimately takes tens of seconds, and a sanitizer
  // build several times that, so the runner's per-case timeout
  // (OPENSTARBOUND_TEST_TIMEOUT) is the practical limit.
  std::int64_t startTime = Time::monotonicMilliseconds();
  while (m_mainPlayer->isTeleporting() || m_client->playerWorld().empty()) {
    m_client->update(0.016f);
    Thread::sleep(16);
    if (Time::monotonicMilliseconds() - startTime > 300000)
      break;
  }
}

WorldId TestUniverse::currentPlayerWorld() const {
  if (auto clientContext = m_client->clientContext())
    return clientContext->playerWorldId();

  return {};
}

void TestUniverse::update(unsigned times) {
  for (unsigned i = 0; i < times; ++i) {
    m_client->update(0.016f);
    Thread::sleep(16);
  }
}

List<Drawable> TestUniverse::currentClientDrawables() {
  WorldRenderData renderData;
  auto worldClient = m_client->worldClient();
  worldClient->centerClientWindowOnPlayer(m_clientWindowSize);
  worldClient->render(renderData, 0, nullptr);

  List<Drawable> drawables;
  for (auto& ed : renderData.entityDrawables) {
    for (auto& p : ed.layers)
      drawables.appendAll(std::move(p.second));
  }

  return drawables;
}

}
