#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarImage.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarEither.hpp"
#include "StarMaybe.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarThread.hpp"
#include "StarRect.hpp"
#include "StarInterpolation.hpp"
#include "StarAudio.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarOrderedMap.hpp"
#include "StarArray.hpp"
#include "StarNetCompatibility.hpp"
#include <functional>
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
import star.perlin;
#include "StarBTree.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
#include "StarDataStreamDevices.hpp"
import star.btree_database;
#include "StarOrderedSet.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarAStar.hpp"
#include "StarLua.hpp"
import star.periodic_function;
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
import star.listener;
#include "StarLogging.hpp"

import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.item_descriptor;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.warping;
import star.world_geometry;
import star.wiring;
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
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
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
#include "StarLuaComponents.hpp"
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.game_timers;
import star.world_client;
import star.world_client_thread;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;
import star.actor_movement_controller;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.scripted_entity;
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
import star.radio_message_database;
import star.player;

import star.tick_rate_monitor;

namespace Star {

WorldClientThread::WorldClientThread(ClientSubWorldId subWorldId, UniverseClient* universe)
  : Thread("WorldClientThread: " + String(subWorldId)),
    m_subWorldId(subWorldId),
    m_stop(false),
    m_errorOccurred(false),
    m_shouldExpire(false) {
    m_worldClient = make_shared<WorldClient>(subWorldId,universe);
}

WorldClientThread::~WorldClientThread() {
  m_stop = true;
  join();

  RecursiveMutexLocker locker(m_mutex);
}

ClientSubWorldId WorldClientThread::subWorldId() const {
  return m_subWorldId;
}

void WorldClientThread::start() {
  m_stop = false;
  m_errorOccurred = false;
  Thread::start();
}

void WorldClientThread::stop() {
  m_stop = true;
  Thread::join();
}

void WorldClientThread::setPause(shared_ptr<const atomic<bool>> pause) {
  m_pause = pause;
}

bool WorldClientThread::errorOccurred() {
  return m_errorOccurred;
}

bool WorldClientThread::shouldExpire() {
  return m_shouldExpire;
}

void WorldClientThread::pushIncomingPackets(List<PacketPtr> packets) {
  RecursiveMutexLocker queueLocker(m_queueMutex);
  m_incomingPacketQueue.appendAll(std::move(packets));
}

List<PacketPtr> WorldClientThread::pullOutgoingPackets() {
  RecursiveMutexLocker queueLocker(m_queueMutex);
  return take(m_outgoingPacketQueue);
}

void WorldClientThread::executeAction(WorldClientAction action) {
  RecursiveMutexLocker locker(m_mutex);
  action(this, m_worldClient.get());
}

void WorldClientThread::setUpdateAction(WorldClientAction updateAction) {
  RecursiveMutexLocker locker(m_mutex);
  m_updateAction = updateAction;
}

void WorldClientThread::passMessage(Message&& message) {
  RecursiveMutexLocker locker(m_messageMutex);
  m_messages.append(std::move(message));
}

void WorldClientThread::clearMessages() {
  List<Message> messages;
  {
    RecursiveMutexLocker locker(m_messageMutex);
    messages = std::move(m_messages);
  }
  for (auto& message : messages) {
    message.promise.fail("Messages discarded");
  }
}

void WorldClientThread::run() {
  try {
    auto& root = Root::singleton();
    double updateMeasureWindow = root.assets()->json("/client.config:subWorldUpdateMeasureWindow").toDouble();

    TickRateApproacher tickApproacher(1.0f / GlobalTimestep, updateMeasureWindow);

    while (!m_stop && !m_errorOccurred) {
      LogMap::set(strf("client_{}_update", m_subWorldId), strf("{:4.2f}Hz", tickApproacher.rate()));

      update();
      tickApproacher.setTargetTickRate(1.0f / GlobalTimestep);
      tickApproacher.tick();

      double spareTime = tickApproacher.spareTime();

      int64_t spareMilliseconds = floor(spareTime * 1000);
      if (spareMilliseconds > 0)
        Thread::sleepPrecise(spareMilliseconds);
    }
  } catch (std::exception const& e) {
    Logger::error("WorldClientThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
  }
}

void WorldClientThread::update() {
  RecursiveMutexLocker locker(m_mutex);
  RecursiveMutexLocker queueLocker(m_queueMutex);
  auto incomingPackets = take(m_incomingPacketQueue);
  queueLocker.unlock();
  try {
    m_worldClient->handleIncomingPackets(std::move(incomingPackets));
  } catch (std::exception const& e) {
    Logger::error("WorldClientThread exception caught handling incoming packets: {}", outputException(e, true));
    queueLocker.lock();
    m_outgoingPacketQueue.append({make_shared<ClientSubWorldRequest>(m_subWorldId, WorldId())});
    queueLocker.unlock();
    m_errorOccurred = true;
  }

  float dt = GlobalTimestep * GlobalTimescale;
  if (dt > 0.0f && (!m_pause || *m_pause == false))
    m_worldClient->update(dt);

  // don't handle messages until in world
  if (m_worldClient->inWorld()) {
    List<Message> messages;
    {
      RecursiveMutexLocker locker(m_messageMutex);
      messages = std::move(m_messages);
    }
    for (auto& message : messages) {
      if (auto resp = m_worldClient->receiveMessage(ServerConnectionId, message.message, message.args))
        if (resp->is<RpcPromise<Json>>())
          message.promise.chain(resp->get<RpcPromise<Json>>());
        else
          message.promise.fulfill(resp->get<Json>());
      else
        message.promise.fail("Message not handled by world");
    }
  }

  auto outgoingPackets = m_worldClient->getOutgoingPackets();
  auto shouldDestroy = m_worldClient->pullRequestedDestroy();
  queueLocker.lock();
  m_outgoingPacketQueue.append(make_shared<ClientSubWorldPackets>(m_subWorldId,std::move(outgoingPackets)));
  if (shouldDestroy) {
    Logger::info("WorldClientThread requesting destroy");
    m_outgoingPacketQueue.append(make_shared<ClientSubWorldRequest>(m_subWorldId, WorldId()));
  }
  queueLocker.unlock();

  m_shouldExpire = m_worldClient->shouldExpire();

  if (m_updateAction)
    m_updateAction(this, m_worldClient.get());
}

}
