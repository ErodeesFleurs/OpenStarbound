module;
import star.json;
import star.json_extra;
import star.xx_hash;
import star.lua;
import star.game_types;
import star.rpc_promise;
import star.strong_typedef;
import star.data_stream;
import star.vector;
import star.maybe;
import star.string;
import star.either;
import star.variant;
import star.color;
import star.bi_map;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.ordered_set;
import star.io_device;
import star.byte_array;
import star.data_stream_devices;

import std;
typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
import std;
import star.format;
import star.id_map;
import star.version;
import star.ordered_map;
import star.casting;
import star.set;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.btree_database;
import star.image;
import star.audio;
import star.map;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.uuid;
import star.celestial_coordinate;
import star.warping;
import star.lua_game_converters;
import star.host_address;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.chat_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.ai_types;
import star.socket;
import star.tcp;
import star.p2p_networking_service;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.world_geometry;
import star.wiring;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.lua_root;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.universe_connection;
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.chat_action;
import star.mixer;
import star.entity_rendering;
import star.world;
import star.lua_components;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.game_timers;
import star.world_client;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;
import star.world_template;

module star.universe_client_lua_bindings;
import star.client_context;
import star.root;

namespace Star {

LuaCallbacks LuaBindings::makeUniverseClientThreadCallbacks(Root& root, UniverseClient* universe) {
  LuaCallbacks callbacks;
  
  callbacks.registerCallback("sendOwnUniverseMessage", [universe](String const& message, LuaVariadic<Json> args) -> RpcPromise<Json> {
    auto pair = RpcPromise<Json>::createPair();
    universe->passMessage({message,JsonArray::from(std::move(args)),pair.second});
    return pair.first;
  });
  callbacks.registerCallback("sendUniverseMessage", [universe](ConnectionId const& connection, String const& message, LuaVariadic<Json> args) -> RpcPromise<Json> {
    return universe->sendUniverseMessage(connection,message,JsonArray::from(std::move(args)));
  });
  callbacks.registerCallback("sendServerUniverseMessage", [universe](String const& message, LuaVariadic<Json> args) -> RpcPromise<Json> {
    return universe->sendUniverseMessage(ServerConnectionId,message,JsonArray::from(std::move(args)));
  });
  
  callbacks.registerCallback("createClientCustomWorld", [universe](String name, Json templateData) {
    universe->createCustomWorld(name, templateData);
  });
  
  callbacks.registerCallback("createClientCustomWorldFromConfig", [&root, universe](String name, Json worldConfig) {
    std::uint64_t worldSeed;
    if (worldConfig.contains("seed"))
      worldSeed = worldConfig.getUInt("seed");
    else
      worldSeed = root.random().randu64();

    String worldType = worldConfig.getString("type");

    std::shared_ptr<VisitableWorldParameters> worldParameters;
    if (worldType.equalsIgnoreCase("Terrestrial"))
      worldParameters = generateTerrestrialWorldParameters(root, worldConfig.getString("planetType"), worldConfig.getString("planetSize"), worldSeed);
    else if (worldType.equalsIgnoreCase("Asteroids"))
      worldParameters = generateAsteroidsWorldParameters(root, worldSeed);
    else if (worldType.equalsIgnoreCase("FloatingDungeon"))
      worldParameters = generateFloatingDungeonWorldParameters(root, worldConfig.getString("dungeonWorld"));
    else
      throw StarException(strf("Unknown world type: '{}'\n", worldType));

    if (worldConfig.contains("level"))
      worldParameters->threatLevel = worldConfig.getFloat("level");

    if (worldConfig.contains("beamUpRule"))
      worldParameters->beamUpRule = BeamUpRuleNames.getLeft(worldConfig.getString("beamUpRule"));
    worldParameters->disableDeathDrops = worldConfig.getBool("disableDeathDrops", false);

    SkyParameters skyParameters = SkyParameters(worldConfig.get("skyParameters", Json()));
    auto worldTemplate = WorldTemplate(root, worldParameters, skyParameters, worldSeed);
    universe->createCustomWorld(name, worldTemplate.store());
  });
  
  callbacks.registerCallback("connectionId", [universe]() {
    if (!universe->isConnected())
      throw StarException("Universe is not connected");
    return universe->clientContext()->connectionId();
  });
  
  callbacks.registerCallback("clientUuid", [universe]() {
    if (!universe->isConnected())
      throw StarException("Universe is not connected");
    return universe->clientContext()->playerUuid().hex();
  });
  
  callbacks.registerCallback("serverOpenProtocolVersion", [universe]() {
    return universe->connectionVersion();
  });
  
  callbacks.registerCallback("playerCount", [universe]() -> Vec2U {
    if (!universe->isConnected())
      return Vec2U();
    return Vec2U(universe->players(),universe->maxPlayers());
  });
  
  return callbacks;
}

LuaCallbacks LuaBindings::makeUniverseClientCallbacks(Root& root, UniverseClient* universe) {
  LuaCallbacks callbacks = makeUniverseClientThreadCallbacks(root, universe);
  
  callbacks.registerCallback("subWorldActive", [universe](String const& worldId) -> bool {
    return universe->subWorldExistsOnWorld(parseWorldId(worldId));
  });
  
  callbacks.registerCallback("loadSubWorld", [universe](String const& worldId) {
    universe->getSubWorldOnWorld(parseWorldId(worldId));
  });
  
  callbacks.registerCallback("unloadSubWorld", [universe](String const& worldId) {
    universe->destroySubWorldOnWorld(parseWorldId(worldId));
  });
  
  callbacks.registerCallback("sendSubWorldMessage", [universe](String const& worldId, String const& message, LuaVariadic<Json> args) -> RpcPromise<Json> {
    return universe->sendSubWorldOnWorldMessage(parseWorldId(worldId), message, JsonArray::from(std::move(args)));
  });
  
  callbacks.registerCallback("sendMainWorldMessage", [universe](String const& message, LuaVariadic<Json> args) -> RpcPromise<Json> {
    return universe->sendMainWorldMessage(message, JsonArray::from(std::move(args)));
  });
  
  // primarily useful on the main world and other non-universe contexts, as universe is accessible everywhere on the main client thread
  callbacks.registerCallback("callScriptContext", [universe](String const& contextName, String const& function, LuaVariadic<LuaValue> const& args) -> Maybe<LuaValue> {
    auto context = universe->scriptContext(contextName);
    if (!context)
      throw StarException::format("Context {} does not exist", contextName);
    return context->invoke(function, args);
  });
  
  // primarily useful on universe contexts
  callbacks.registerCallback("callMainWorldScriptContext", [universe](String const& contextName, String const& function, LuaVariadic<LuaValue> const& args) -> Maybe<LuaValue> {
    if (!universe->isConnected())
      throw StarException("Universe is not connected");
    auto world = universe->worldClient();
    if (!world->inWorld())
      throw StarException("Not in a world");
    auto context = world->scriptContext(contextName);
    if (!context)
      throw StarException::format("Context {} does not exist", contextName);
    return context->invoke(function, args);
  });

  return callbacks;
}

}
