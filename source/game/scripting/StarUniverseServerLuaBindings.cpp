module;
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarXXHash.hpp"
#include "StarLua.hpp"
#include "StarGameTypes.hpp"
#include "StarRpcPromise.hpp"
#include "StarIdMap.hpp"
#include "StarConfig.hpp"
#include "StarMaybe.hpp"
#include "StarString.hpp"
import star.lock_file;
#include "StarThread.hpp"
#include "StarVector.hpp"
#include "StarString.hpp"
#include "StarEither.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarIODevice.hpp"
#include "StarByteArray.hpp"
#include "StarDataStreamDevices.hpp"

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
#include "StarNetCompatibility.hpp"
#include "StarSet.hpp"
#include "StarBTree.hpp"
#include "StarOrderedMap.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
import star.btree_database;
#include "StarCasting.hpp"
#include "StarOrderedSet.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
import star.version;
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.perlin;
import star.weighted_pool;
#include "StarStrongTypedef.hpp"
#include <functional>
#include "StarRect.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarMaybe.hpp"
#include "StarColor.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarVariant.hpp"
#include "StarSet.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"
#include "StarOrderedMap.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarMap.hpp"
#include "StarRandom.hpp"
#include "StarBlockAllocator.hpp"
#include <atomic>
#include <memory>
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarLuaGameConverters.hpp"
import star.universe;
import star.worker_pool;
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
#include "StarLuaRoot.hpp"
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
#include "StarLuaComponents.hpp"
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
#include "StarP2PNetworkingService.hpp"
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
  Maybe<String> uuidForClient(UniverseServer* universe, ConnectionId arg1);
  List<ConnectionId> clientIds(UniverseServer* universe);
  size_t numberOfClients(UniverseServer* universe);
  bool isConnectedClient(UniverseServer* universe, ConnectionId arg1);
  String clientNick(UniverseServer* universe, ConnectionId arg1);
  Maybe<ConnectionId> findNick(UniverseServer* universe, String const& arg1);
  String clientAccount(UniverseServer* universe, ConnectionId arg1);
  void adminBroadcast(UniverseServer* universe, String const& arg1);
  void adminWhisper(UniverseServer* universe, ConnectionId arg1, String const& arg2);
  bool isAdmin(UniverseServer* universe, ConnectionId arg1);
  void setAdmin(UniverseServer* universe, ConnectionId arg1, Maybe<bool> arg2);
  bool isPvp(UniverseServer* universe, ConnectionId arg1);
  void setPvp(UniverseServer* universe, ConnectionId arg1, Maybe<bool> arg2);
  bool isWorldActive(UniverseServer* universe, String const& worldId);
  StringList activeWorlds(UniverseServer* universe);
  RpcPromise<Json> sendWorldMessage(UniverseServer* universe, String const& worldId, String const& message, LuaVariadic<Json> args);
  bool sendPacket(UniverseServer* universe, ConnectionId clientId, String const& packetTypeName, Json const& args);
  String clientWorld(UniverseServer* universe, ConnectionId clientId);
  void disconnectClient(UniverseServer* universe, ConnectionId clientId, Maybe<String> const& reason);
  void banClient(UniverseServer* universe, ConnectionId clientId, Maybe<String> const& reason, bool banIp, bool banUuid, Maybe<int> timeout);
}

namespace Star {

LuaCallbacks LuaBindings::makeUniverseServerCallbacks(UniverseServer* universe) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<Maybe<String>, ConnectionId>("uuidForClient", bind(UniverseServerCallbacks::uuidForClient, universe, _1));
  callbacks.registerCallbackWithSignature<List<ConnectionId>>("clientIds", bind(UniverseServerCallbacks::clientIds, universe));
  callbacks.registerCallbackWithSignature<size_t>("numberOfClients", bind(UniverseServerCallbacks::numberOfClients, universe));
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isConnectedClient", bind(UniverseServerCallbacks::isConnectedClient, universe, _1));
  callbacks.registerCallbackWithSignature<String, ConnectionId>("clientNick", bind(UniverseServerCallbacks::clientNick, universe, _1));
  callbacks.registerCallbackWithSignature<Maybe<ConnectionId>, String>("findNick", bind(UniverseServerCallbacks::findNick, universe, _1));
  callbacks.registerCallbackWithSignature<String, ConnectionId>("clientAccount", bind(UniverseServerCallbacks::clientAccount, universe, _1));
  callbacks.registerCallbackWithSignature<void, String>("adminBroadcast", bind(UniverseServerCallbacks::adminBroadcast, universe, _1));
  callbacks.registerCallbackWithSignature<void, ConnectionId, String>("adminWhisper", bind(UniverseServerCallbacks::adminWhisper, universe, _1, _2));
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isAdmin", bind(UniverseServerCallbacks::isAdmin, universe, _1));
  callbacks.registerCallbackWithSignature<bool, ConnectionId>("isPvp", bind(UniverseServerCallbacks::isPvp, universe, _1));
  callbacks.registerCallbackWithSignature<void, ConnectionId, bool>("setPvp", bind(UniverseServerCallbacks::setPvp, universe, _1, _2));
  callbacks.registerCallbackWithSignature<bool, String>("isWorldActive", bind(UniverseServerCallbacks::isWorldActive, universe, _1));
  callbacks.registerCallbackWithSignature<StringList>("activeWorlds", bind(UniverseServerCallbacks::activeWorlds, universe));
  callbacks.registerCallbackWithSignature<RpcPromise<Json>, String, String, LuaVariadic<Json>>("sendWorldMessage", bind(UniverseServerCallbacks::sendWorldMessage, universe, _1, _2, _3));
  callbacks.registerCallbackWithSignature<bool, ConnectionId, String, Json>("sendPacket", bind(UniverseServerCallbacks::sendPacket, universe, _1, _2, _3));
  callbacks.registerCallbackWithSignature<String, ConnectionId>("clientWorld", bind(UniverseServerCallbacks::clientWorld, universe, _1));
  callbacks.registerCallbackWithSignature<void, ConnectionId, Maybe<String>>("disconnectClient", bind(UniverseServerCallbacks::disconnectClient, universe, _1, _2));
  callbacks.registerCallbackWithSignature<void, ConnectionId, Maybe<String>, bool, bool, Maybe<int>>("banClient", bind(UniverseServerCallbacks::banClient, universe, _1, _2, _3, _4, _5));
  callbacks.registerCallback("warpClient", [universe](ConnectionId clientId, String action, Maybe<bool> deploy) {
    universe->clientWarpPlayer(clientId, parseWarpAction(action), deploy.value(false));
  });

  callbacks.registerCallback("setServerAccount", [](String const& account, String const& password, Maybe<bool> admin) {
    auto config = Root::singleton().configuration();
    auto serverUsers = config->get("serverUsers").toObject();
    serverUsers[account] = JsonObject{{"password", password}, {"admin", admin.value(false)}};
    config->set("serverUsers", serverUsers);
  });

  callbacks.registerCallback("removeServerAccount", [](String const& account) {
    auto config = Root::singleton().configuration();
    auto serverUsers = config->get("serverUsers").toObject();
    serverUsers.erase(account);
    config->set("serverUsers", serverUsers);
  });

  callbacks.registerCallback("getServerAccounts", []() -> Json {
    auto config = Root::singleton().configuration();
    auto serverUsers = config->get("serverUsers");
    JsonObject result;
    for (auto& p : serverUsers.iterateObject())
      result[p.first] = JsonObject{{"admin", p.second.getBool("admin", false)}};
    return result;
  });

  
  callbacks.registerCallback("clientOpenProtocolVersion", [universe](ConnectionId clientId) {
    return universe->clientConnectionVersion(clientId);
  });
  
  callbacks.registerCallback("createCustomWorld", [universe](String name, Json templateData) {
    universe->createCustomWorld(CustomWorldId(name), make_shared<WorldTemplate>(templateData));
  });
  callbacks.registerCallback("createCustomWorldFromConfig", [universe](String name, Json worldConfig) {
    uint64_t worldSeed;
    if (worldConfig.contains("seed"))
      worldSeed = worldConfig.getUInt("seed");
    else
      worldSeed = Random::randu64();

    String worldType = worldConfig.getString("type");

    VisitableWorldParametersPtr worldParameters;
    if (worldType.equalsIgnoreCase("Terrestrial"))
      worldParameters = generateTerrestrialWorldParameters(worldConfig.getString("planetType"), worldConfig.getString("planetSize"), worldSeed);
    else if (worldType.equalsIgnoreCase("Asteroids"))
      worldParameters = generateAsteroidsWorldParameters(worldSeed);
    else if (worldType.equalsIgnoreCase("FloatingDungeon"))
      worldParameters = generateFloatingDungeonWorldParameters(worldConfig.getString("dungeonWorld"));
    else
      throw StarException(strf("Unknown world type: '{}'\n", worldType));

    if (worldConfig.contains("level"))
      worldParameters->threatLevel = worldConfig.getFloat("level");

    if (worldConfig.contains("beamUpRule"))
      worldParameters->beamUpRule = BeamUpRuleNames.getLeft(worldConfig.getString("beamUpRule"));
    worldParameters->disableDeathDrops = worldConfig.getBool("disableDeathDrops", false);

    SkyParameters skyParameters = SkyParameters(worldConfig.get("skyParameters", Json()));
    auto worldTemplate = make_shared<WorldTemplate>(worldParameters, skyParameters, worldSeed);
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
Maybe<String> LuaBindings::UniverseServerCallbacks::uuidForClient(UniverseServer* universe, ConnectionId arg1) {
  return universe->uuidForClient(arg1).apply([](Uuid const& str) { return str.hex(); });
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
size_t LuaBindings::UniverseServerCallbacks::numberOfClients(UniverseServer* universe) {
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
Maybe<ConnectionId> LuaBindings::UniverseServerCallbacks::findNick(UniverseServer* universe, String const& arg1) {
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
void LuaBindings::UniverseServerCallbacks::setAdmin(UniverseServer* universe, ConnectionId arg1, Maybe<bool> arg2) {
  ConnectionId client = arg1;
  bool setAdminTo = arg2.value(true);
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
void LuaBindings::UniverseServerCallbacks::setPvp(UniverseServer* universe, ConnectionId arg1, Maybe<bool> arg2) {
  ConnectionId client = arg1;
  bool setPvpTo = arg2.value(true);
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
  auto packet = createPacket(packetType, args);
  return universe->sendPacket(clientId, packet);
}

String LuaBindings::UniverseServerCallbacks::clientWorld(UniverseServer* universe, ConnectionId clientId) {
  return printWorldId(universe->clientWorld(clientId));
}

void LuaBindings::UniverseServerCallbacks::disconnectClient(UniverseServer* universe, ConnectionId clientId, Maybe<String> const& reason) {
  return universe->disconnectClient(clientId, reason.value());
}

void LuaBindings::UniverseServerCallbacks::banClient(UniverseServer* universe, ConnectionId clientId, Maybe<String> const& reason, bool banIp, bool banUuid, Maybe<int> timeout) {
  return universe->banUser(clientId, reason.value(), make_pair(banIp, banUuid), timeout);
}

}
