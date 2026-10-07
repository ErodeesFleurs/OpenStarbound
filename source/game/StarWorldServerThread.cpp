#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarEither.hpp"
#include "StarRect.hpp"
#include "StarImage.hpp"
#include "StarColor.hpp"
#include "StarInterpolation.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVersion.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarDirectives.hpp"
#include "StarWeightedPool.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarSectorArray2D.hpp"
#include <functional>
#include "StarOrderedMap.hpp"
#include "StarNetCompatibility.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarOrderedSet.hpp"
#include "StarSet.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarMap.hpp"
#include "StarRandom.hpp"
#include "StarBlockAllocator.hpp"
#include "StarThread.hpp"
#include "StarRpcPromise.hpp"
#include "StarAStar.hpp"
#include "StarLua.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarAudio.hpp"
#include "StarNetElement.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarListener.hpp"
#include "StarLogging.hpp"

#include "StarLuaRoot.hpp"
import star.worker_pool;
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
import star.force_regions;
import star.world;
import star.liquid_types;
import star.tile_damage;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
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
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;
import star.game_timers;
import star.actor_movement_controller;
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.humanoid;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.lounging_entities;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.npc_database;
import star.effect_emitter;
import star.npc;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.inventory_types;
import star.ai_types;
import star.entity_rendering;
import star.radio_message_database;
import star.player;

import star.tick_rate_monitor;

namespace Star {

WorldServerThread::WorldServerThread(WorldServerPtr server, WorldId worldId)
  : Thread("WorldServerThread: " + printWorldId(worldId)),
    m_worldServer(std::move(server)),
    m_worldId(std::move(worldId)),
    m_stop(false),
    m_errorOccurred(false),
    m_shouldExpire(true) {
  if (m_worldServer)
    m_worldServer->setWorldId(printWorldId(m_worldId));
}

WorldServerThread::~WorldServerThread() {
  m_stop = true;
  join();

  RecursiveMutexLocker locker(m_mutex);
  for (auto clientId : m_worldServer->clientIds())
    removeClient(clientId);
}

WorldId WorldServerThread::worldId() const {
  return m_worldId;
}

void WorldServerThread::start() {
  m_stop = false;
  m_errorOccurred = false;
  Thread::start();
}

void WorldServerThread::stop() {
  m_stop = true;
  Thread::join();
}

void WorldServerThread::setPause(shared_ptr<const atomic<bool>> pause) {
  m_pause = pause;
}

bool WorldServerThread::serverErrorOccurred() {
  return m_errorOccurred;
}

bool WorldServerThread::shouldExpire() {
  return m_shouldExpire;
}

bool WorldServerThread::spawnTargetValid(SpawnTarget const& spawnTarget) {
  try {
    RecursiveMutexLocker locker(m_mutex);
    return m_worldServer->spawnTargetValid(spawnTarget);
  } catch (std::exception const& e) {
    Logger::error("WorldServerThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
    return false;
  }
}

bool WorldServerThread::addClient(ConnectionId clientId, SpawnTarget const& spawnTarget, bool isLocal, bool isAdmin, NetCompatibilityRules netRules, ClientSubWorldId subWorldId) {
  try {
    RecursiveMutexLocker locker(m_mutex);
    if (m_worldServer->addClient(clientId, spawnTarget, isLocal, isAdmin, netRules, subWorldId)) {
      m_clients.add(clientId);
      return true;
    }

    return false;
  } catch (std::exception const& e) {
    Logger::error("WorldServerThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
    return false;
  }
}

List<PacketPtr> WorldServerThread::removeClient(ConnectionId clientId) {
  RecursiveMutexLocker locker(m_mutex);
  if (!m_clients.contains(clientId))
    return {};

  RecursiveMutexLocker queueLocker(m_queueMutex);

  List<PacketPtr> outgoingPackets;
  try {
    auto incomingPackets = take(m_incomingPacketQueue[clientId]);
    if (m_worldServer->hasClient(clientId))
      m_worldServer->handleIncomingPackets(clientId, std::move(incomingPackets));

    outgoingPackets = take(m_outgoingPacketQueue[clientId]);
    if (m_worldServer->hasClient(clientId))
      outgoingPackets.appendAll(m_worldServer->removeClient(clientId));

  } catch (std::exception const& e) {
    Logger::error("WorldServerThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
  }

  m_clients.remove(clientId);
  m_incomingPacketQueue.remove(clientId);
  m_outgoingPacketQueue.remove(clientId);
  return outgoingPackets;
}

List<ConnectionId> WorldServerThread::clients() const {
  RecursiveMutexLocker locker(m_mutex);
  return m_clients.values();
}

bool WorldServerThread::hasClient(ConnectionId clientId) const {
  RecursiveMutexLocker locker(m_mutex);
  return m_clients.contains(clientId);
}

bool WorldServerThread::noClients() const {
  RecursiveMutexLocker locker(m_mutex);
  return m_clients.empty();
}

ClientSubWorldId WorldServerThread::clientSubWorld(ConnectionId clientId) const {
  RecursiveMutexLocker locker(m_mutex);
  return m_worldServer->clientSubWorld(clientId);
}

List<ConnectionId> WorldServerThread::erroredClients() const {
  RecursiveMutexLocker locker(m_mutex);
  auto unerroredClients = HashSet<ConnectionId>::from(m_worldServer->clientIds());
  return m_clients.difference(unerroredClients).values();
}

void WorldServerThread::pushIncomingPackets(ConnectionId clientId, List<PacketPtr> packets) {
  RecursiveMutexLocker queueLocker(m_queueMutex);
  m_incomingPacketQueue[clientId].appendAll(std::move(packets));
}

List<PacketPtr> WorldServerThread::pullOutgoingPackets(ConnectionId clientId) {
  RecursiveMutexLocker queueLocker(m_queueMutex);
  return take(m_outgoingPacketQueue[clientId]);
}

Maybe<Vec2F> WorldServerThread::playerRevivePosition(ConnectionId clientId) const {
  try {
    RecursiveMutexLocker locker(m_mutex);
    if (auto player = m_worldServer->clientPlayer(clientId))
      return player->position() + player->feetOffset();
    return {};
  } catch (std::exception const& e) {
    Logger::error("WorldServerThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
    return {};
  }
}

Maybe<pair<String, String>> WorldServerThread::pullNewPlanetType() {
  try {
    RecursiveMutexLocker locker(m_mutex);
    return m_worldServer->pullNewPlanetType();
  } catch (std::exception const& e) {
    Logger::error("WorldServerThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
    return {};
  }
}

void WorldServerThread::executeAction(WorldServerAction action) {
  RecursiveMutexLocker locker(m_mutex);
  action(this, m_worldServer.get());
}

void WorldServerThread::setUpdateAction(WorldServerAction updateAction) {
  RecursiveMutexLocker locker(m_mutex);
  m_updateAction = updateAction;
}

void WorldServerThread::passMessages(List<Message>&& messages) {
  RecursiveMutexLocker locker(m_messageMutex);
  m_messages.appendAll(std::move(messages));
}

void WorldServerThread::unloadAll(bool force) {
  try {
    RecursiveMutexLocker locker(m_mutex);
    m_worldServer->unloadAll(force);
  } catch (std::exception const& e) {
    Logger::error("WorldServerThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
  }
}

WorldChunks WorldServerThread::readChunks() {
  try {
    RecursiveMutexLocker locker(m_mutex);
    return m_worldServer->readChunks();
  } catch (std::exception const& e) {
    Logger::error("WorldServerThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
    return {};
  }
}

void WorldServerThread::run() {
  try {
    auto& root = Root::singleton();
    double updateMeasureWindow = root.assets()->json("/universe_server.config:updateMeasureWindow").toDouble();
    double fidelityDecrementScore = root.assets()->json("/universe_server.config:fidelityDecrementScore").toDouble();
    double fidelityIncrementScore = root.assets()->json("/universe_server.config:fidelityIncrementScore").toDouble();

    String serverFidelityMode = root.configuration()->get("serverFidelity").toString();
    Maybe<WorldServerFidelity> lockedFidelity;
    if (!serverFidelityMode.equalsIgnoreCase("automatic"))
      lockedFidelity = WorldServerFidelityNames.getLeft(serverFidelityMode);

    double storageInterval = root.assets()->json("/universe_server.config:worldStorageInterval").toDouble() / 1000.0;
    Timer storageTimer = Timer::withTime(storageInterval);

    TickRateApproacher tickApproacher(1.0f / ServerGlobalTimestep, updateMeasureWindow);
    double fidelityScore = 0.0;
    WorldServerFidelity automaticFidelity = WorldServerFidelity::Medium;

    while (!m_stop && !m_errorOccurred) {
      auto fidelity = lockedFidelity.value(automaticFidelity);
      LogMap::set(strf("server_{}_fidelity", m_worldId), WorldServerFidelityNames.getRight(fidelity));
      LogMap::set(strf("server_{}_update", m_worldId), strf("{:4.2f}Hz", tickApproacher.rate()));

      update(fidelity);
      tickApproacher.setTargetTickRate(1.0f / ServerGlobalTimestep);
      tickApproacher.tick();

      if (storageTimer.timeUp()) {
        sync();
        storageTimer.restart(storageInterval);
      }

      double spareTime = tickApproacher.spareTime();
      fidelityScore += spareTime;

      if (fidelityScore <= fidelityDecrementScore) {
        if (automaticFidelity > WorldServerFidelity::Minimum)
          automaticFidelity = (WorldServerFidelity)((int)automaticFidelity - 1);
        fidelityScore = 0.0;
      }

      if (fidelityScore >= fidelityIncrementScore) {
        if (automaticFidelity < WorldServerFidelity::High)
          automaticFidelity = (WorldServerFidelity)((int)automaticFidelity + 1);
        fidelityScore = 0.0;
      }

      int64_t spareMilliseconds = floor(spareTime * 1000);
      if (spareMilliseconds > 0)
        Thread::sleepPrecise(spareMilliseconds);
    }
  } catch (std::exception const& e) {
    Logger::error("WorldServerThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
  }
}

void WorldServerThread::update(WorldServerFidelity fidelity) {
  RecursiveMutexLocker locker(m_mutex);
  auto unerroredClientIds = m_worldServer->clientIds();
  for (auto clientId : unerroredClientIds) {
    RecursiveMutexLocker queueLocker(m_queueMutex);
    auto incomingPackets = take(m_incomingPacketQueue[clientId]);
    queueLocker.unlock();
    try {
      m_worldServer->handleIncomingPackets(clientId, std::move(incomingPackets));
    } catch (std::exception const& e) {
      Logger::error("WorldServerThread exception caught handling incoming packets for client {}: {}",
          clientId, outputException(e, true));
      RecursiveMutexLocker queueLocker(m_queueMutex);
      m_outgoingPacketQueue[clientId].appendAll(m_worldServer->removeClient(clientId));
      unerroredClientIds.remove(clientId);
    }
  }

  float dt = ServerGlobalTimestep * GlobalTimescale;
  m_worldServer->setFidelity(fidelity);
  if (dt > 0.0f && (!m_pause || *m_pause == false))
    m_worldServer->update(dt);

  List<Message> messages;
  {
    RecursiveMutexLocker locker(m_messageMutex);
    messages = std::move(m_messages);
  }
  for (auto& message : messages) {
    if (auto resp = m_worldServer->receiveMessage(ServerConnectionId, message.message, message.args))
      if (resp->is<RpcPromise<Json>>())
        message.promise.chain(resp->get<RpcPromise<Json>>());
      else
        message.promise.fulfill(resp->get<Json>());
    else
      message.promise.fail("Message not handled by world");
  }

  for (auto& clientId : unerroredClientIds) {
    auto outgoingPackets = m_worldServer->getOutgoingPackets(clientId);
    RecursiveMutexLocker queueLocker(m_queueMutex);
    m_outgoingPacketQueue[clientId].appendAll(std::move(outgoingPackets));
  }

  m_shouldExpire = m_worldServer->shouldExpire();

  if (m_updateAction)
    m_updateAction(this, m_worldServer.get());
}

void WorldServerThread::sync() {
  RecursiveMutexLocker locker(m_mutex);
  Logger::debug("WorldServer: periodic sync to disk of world {}", m_worldId);
  m_worldServer->sync();
}

}
