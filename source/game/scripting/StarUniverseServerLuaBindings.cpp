module;
import star.json;
import star.json_extra;
import star.xx_hash;
import star.lua;
import star.game_types;
import star.rpc_promise;
import star.id_map;
import std;
import star.format;
import star.string;
import star.lock_file;
import star.thread;
import star.either;
import star.data_stream;
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
import star.set;
import star.btree;
import star.ordered_map;
import star.lru_cache;
import star.btree_database;
import star.casting;
import star.ordered_set;
import star.list;
import star.bi_map;
import star.version;
import star.random;
import star.perlin;
import star.weighted_pool;
import star.strong_typedef;
import star.worker_pool;

import star.sector_array_2d;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.image;
import star.map;
import star.ref_ptr;
import star.logging;
import star.listener;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.lua_game_converters;
import star.universe;
import star.celestial_coordinate;
import star.host_address;
import star.uuid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.lua_root;
import star.versioning_database;
import star.world_storage;
import star.tile_modification;
import star.world;
import star.celestial_types;
import star.chat_types;
import star.warping;
import star.wiring;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.cellular_light_array;
import star.cellular_lighting;
import star.cellular_liquid;
import star.world_structure;
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
import star.world_template;
import star.system_world_server;
import star.system_world_server_thread;
import star.socket;
import star.tcp;
import star.p2p_networking_service;
import star.net_packet_socket;
import star.universe_connection;
import star.universe_settings;
import star.universe_server;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

module star.universe_server_lua_bindings;

namespace Star::LuaBindings::UniverseServerCallbacks {
  std::optional<String> uuidForClient(UniverseServer* universe, ConnectionId arg1);
  List<ConnectionId> clientIds(UniverseServer* universe);
  std::size_t numberOfClients(UniverseServer* universe);
  bool isConnectedClient(UniverseServer* universe, ConnectionId arg1);
  String clientNick(UniverseServer* universe, ConnectionId arg1);
  std::optional<ConnectionId> findNick(UniverseServer* universe, String const& arg1);
  String clientAccount(UniverseServer* universe, ConnectionId arg1);
  void adminBroadcast(UniverseServer* universe, String const& arg1);
  void adminWhisper(UniverseServer* universe, ConnectionId arg1, String const& arg2);
  bool isAdmin(UniverseServer* universe, ConnectionId arg1);
  void setAdmin(UniverseServer* universe, ConnectionId arg1, std::optional<bool> arg2);
  bool isPvp(UniverseServer* universe, ConnectionId arg1);
  void setPvp(UniverseServer* universe, ConnectionId arg1, std::optional<bool> arg2);
  bool isWorldActive(UniverseServer* universe, String const& worldId);
  StringList activeWorlds(UniverseServer* universe);
  RpcPromise<Json> sendWorldMessage(UniverseServer* universe, String const& worldId, String const& message, LuaVariadic<Json> args);
  bool sendPacket(UniverseServer* universe, ConnectionId clientId, String const& packetTypeName, Json const& args);
  String clientWorld(UniverseServer* universe, ConnectionId clientId);
  void disconnectClient(UniverseServer* universe, ConnectionId clientId, std::optional<String> const& reason);
  void banClient(UniverseServer* universe, ConnectionId clientId, std::optional<String> const& reason, bool banIp, bool banUuid, std::optional<int> timeout);
}

namespace Star {

LuaCallbacks LuaBindings::makeUniverseServerCallbacks(Root& root, UniverseServer* universe) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<std::optional<String>, ConnectionId>("uuidForClient", std::bind(UniverseServerCallbacks::uuidForClient, universe, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<List<ConnectionId>>("clientIds", std::bind(UniverseServerCallbacks::clientIds, universe));
  callbacks.registerCallbackWithSignature<std::size_t>("numberOfClients", std::bind(UniverseServerCallbacks::numberOfClients, universe));
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isConnectedClient", std::bind(UniverseServerCallbacks::isConnectedClient, universe, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<String, ConnectionId>("clientNick", std::bind(UniverseServerCallbacks::clientNick, universe, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<std::optional<ConnectionId>, String>("findNick", std::bind(UniverseServerCallbacks::findNick, universe, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<String, ConnectionId>("clientAccount", std::bind(UniverseServerCallbacks::clientAccount, universe, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, String>("adminBroadcast", std::bind(UniverseServerCallbacks::adminBroadcast, universe, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, ConnectionId, String>("adminWhisper", std::bind(UniverseServerCallbacks::adminWhisper, universe, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isAdmin", std::bind(UniverseServerCallbacks::isAdmin, universe, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isPvp", std::bind(UniverseServerCallbacks::isPvp, universe, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, ConnectionId, bool>("setPvp", std::bind(UniverseServerCallbacks::setPvp, universe, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<bool, String>("isWorldActive", std::bind(UniverseServerCallbacks::isWorldActive, universe, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<StringList>("activeWorlds", std::bind(UniverseServerCallbacks::activeWorlds, universe));
  callbacks.registerCallbackWithSignature<RpcPromise<Json>, String, String, LuaVariadic<Json>>("sendWorldMessage", std::bind(UniverseServerCallbacks::sendWorldMessage, universe, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  callbacks.registerCallbackWithSignature<bool, ConnectionId, String, Json>("sendPacket", std::bind(UniverseServerCallbacks::sendPacket, universe, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  callbacks.registerCallbackWithSignature<String, ConnectionId>("clientWorld", std::bind(UniverseServerCallbacks::clientWorld, universe, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, ConnectionId, std::optional<String>>("disconnectClient", std::bind(UniverseServerCallbacks::disconnectClient, universe, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, ConnectionId, std::optional<String>, bool, bool, std::optional<int>>("banClient", std::bind(UniverseServerCallbacks::banClient, universe, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
  callbacks.registerCallback("warpClient", [universe](ConnectionId clientId, String action, std::optional<bool> deploy) {
    universe->clientWarpPlayer(clientId, parseWarpAction(action), deploy.value_or(false));
  });

  callbacks.registerCallback("setServerAccount", [&root](String const& account, String const& password, std::optional<bool> admin) {
    auto config = root.configuration();
    auto serverUsers = config->get("serverUsers").toObject();
    serverUsers[account] = JsonObject{{"password", password}, {"admin", admin.value_or(false)}};
    config->set("serverUsers", serverUsers);
  });

  callbacks.registerCallback("removeServerAccount", [&root](String const& account) {
    auto config = root.configuration();
    auto serverUsers = config->get("serverUsers").toObject();
    serverUsers.erase(account);
    config->set("serverUsers", serverUsers);
  });

  callbacks.registerCallback("getServerAccounts", [&root]() -> Json {
    auto config = root.configuration();
    auto serverUsers = config->get("serverUsers");
    JsonObject result;
    for (auto& p : serverUsers.iterateObject())
      result[p.first] = JsonObject{{"admin", p.second.getBool("admin", false)}};
    return result;
  });

  
  callbacks.registerCallback("clientOpenProtocolVersion", [universe](ConnectionId clientId) {
    return universe->clientConnectionVersion(clientId);
  });
  
  callbacks.registerCallback("createCustomWorld", [&root, universe](String name, Json templateData) {
    universe->createCustomWorld(CustomWorldId(name), std::make_shared<WorldTemplate>(root, templateData));
  });
  callbacks.registerCallback("createCustomWorldFromConfig", [&root, universe](String name, Json worldConfig) {
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
    auto worldTemplate = std::make_shared<WorldTemplate>(root, worldParameters, skyParameters, worldSeed);
    Json worldProperties = worldConfig.get("worldProperties", JsonObject{});
    universe->createCustomWorld(CustomWorldId(name), worldTemplate);
  });
  
  callbacks.registerCallback("sendOwnUniverseMessage", [universe](String const& message, LuaVariadic<Json> args) -> RpcPromise<Json> {
    return universe->sendUniverseMessage(ServerConnectionId,message,JsonArray::from(std::move(args)));
  });
  callbacks.registerCallback("sendUniverseMessage", [universe](ConnectionId const& connection, String const& message, LuaVariadic<Json> args) -> RpcPromise<Json> {
    return universe->sendUniverseMessage(connection,message,JsonArray::from(std::move(args)));
  });
  
  callbacks.registerCallback("connectionId", []() {
    return ServerConnectionId;
  });

  return callbacks;
}

// Gets a list of client ids
//
// @return A list of numerical client IDs.
std::optional<String> LuaBindings::UniverseServerCallbacks::uuidForClient(UniverseServer* universe, ConnectionId arg1) {
  return universe->uuidForClient(arg1).transform([](Uuid const& str) { return str.hex(); });
}

// Gets a list of client ids
//
// @return A list of numerical client IDs.
List<ConnectionId> LuaBindings::UniverseServerCallbacks::clientIds(UniverseServer* universe) {
  return universe->clientIds();
}

// Gets the number of logged in clients
//
// @return An integer containing the number of logged in clients
std::size_t LuaBindings::UniverseServerCallbacks::numberOfClients(UniverseServer* universe) {
  return universe->numberOfClients();
}

// Returns whether or not the provided client ID is currently connected
//
// @param clientId the client ID in question
// @return A bool that is true if the client is connected and false otherwise
bool LuaBindings::UniverseServerCallbacks::isConnectedClient(UniverseServer* universe, ConnectionId arg1) {
  return universe->isConnectedClient(arg1);
}

// Returns the nickname for the given client ID
//
// @param clientId the client ID in question
// @return A string containing the nickname of the given client
String LuaBindings::UniverseServerCallbacks::clientNick(UniverseServer* universe, ConnectionId arg1) {
  return universe->clientNick(arg1);
}

// Returns the client ID for the given nick
//
// @param nick the nickname of the client to search for
// @return An integer containing the clientID of the nick in question
std::optional<ConnectionId> LuaBindings::UniverseServerCallbacks::findNick(UniverseServer* universe, String const& arg1) {
  return universe->findNick(arg1);
}

// Sends a message to all logged in clients
//
// @param message the message to broadcast
// @return nil
void LuaBindings::UniverseServerCallbacks::adminBroadcast(UniverseServer* universe, String const& arg1) {
  universe->adminBroadcast(arg1);
}

String LuaBindings::UniverseServerCallbacks::clientAccount(UniverseServer* universe, ConnectionId arg1) {
  return universe->clientAccount(arg1);
}


// Sends a message to a specific client
//
// @param clientId the client id to whisper
// @param message the message to whisper
// @return nil
void LuaBindings::UniverseServerCallbacks::adminWhisper(UniverseServer* universe, ConnectionId arg1, String const& arg2) {
  ConnectionId client = arg1;
  String message = arg2;
  universe->adminWhisper(client, message);
}

// Returns whether or not a specific client is flagged as an admin
//
// @param clientId the client id to check
// @return a boolean containing true if the client is an admin, false otherwise
bool LuaBindings::UniverseServerCallbacks::isAdmin(UniverseServer* universe, ConnectionId arg1) {
  return universe->isAdmin(arg1);
}

// Set (or unset) the admin status of a specific user
//
// @param clientId the client id to modify
// @param setAdminTo set admin status to this bool, defaults to true
// @return nil
void LuaBindings::UniverseServerCallbacks::setAdmin(UniverseServer* universe, ConnectionId arg1, std::optional<bool> arg2) {
  ConnectionId client = arg1;
  bool setAdminTo = arg2.value_or(true);
  universe->setAdmin(client, setAdminTo);
}

// Returns whether or not a specific client is flagged as pvp
//
// @param clientId the client id to check
// @return a boolean containing true if the client is flagged as pvp, false
// otherwise
bool LuaBindings::UniverseServerCallbacks::isPvp(UniverseServer* universe, ConnectionId arg1) {
  return universe->isPvp(arg1);
}

// Set (or unset) the pvp status of a specific user
//
// @param clientId the client id to check
// @param setPvp set pvp status to this bool, defaults to true
// @return nil
void LuaBindings::UniverseServerCallbacks::setPvp(UniverseServer* universe, ConnectionId arg1, std::optional<bool> arg2) {
  ConnectionId client = arg1;
  bool setPvpTo = arg2.value_or(true);
  universe->setPvp(client, setPvpTo);
}

bool LuaBindings::UniverseServerCallbacks::isWorldActive(UniverseServer* universe, String const& worldId) {
  return universe->isWorldActive(parseWorldId(worldId));
}

StringList LuaBindings::UniverseServerCallbacks::activeWorlds(UniverseServer* universe) {
  return universe->activeWorlds().transformed(printWorldId);
}

RpcPromise<Json> LuaBindings::UniverseServerCallbacks::sendWorldMessage(UniverseServer* universe, String const& worldId, String const& message, LuaVariadic<Json> args) {
  return universe->sendWorldMessage(parseWorldId(worldId), message, JsonArray::from(std::move(args)));
}

bool LuaBindings::UniverseServerCallbacks::sendPacket(UniverseServer* universe, ConnectionId clientId, String const& packetTypeName, Json const& args) {
  auto packetType = PacketTypeNames.getLeft(packetTypeName);
  auto packet = createPacket(universe->root().random(), packetType, args);
  return universe->sendPacket(clientId, packet);
}

String LuaBindings::UniverseServerCallbacks::clientWorld(UniverseServer* universe, ConnectionId clientId) {
  return printWorldId(universe->clientWorld(clientId));
}

void LuaBindings::UniverseServerCallbacks::disconnectClient(UniverseServer* universe, ConnectionId clientId, std::optional<String> const& reason) {
  return universe->disconnectClient(clientId, reason.value_or(String{}));
}

void LuaBindings::UniverseServerCallbacks::banClient(UniverseServer* universe, ConnectionId clientId, std::optional<String> const& reason, bool banIp, bool banUuid, std::optional<int> timeout) {
  return universe->banUser(clientId, reason.value_or(String{}), std::make_pair(banIp, banUuid), timeout);
}

}
