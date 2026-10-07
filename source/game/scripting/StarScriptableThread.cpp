#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarIdMap.hpp"
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetElementSystem.hpp"
#include "StarGameTypes.hpp"
#include "StarMaybe.hpp"
#include "StarVariant.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"
#include "StarString.hpp"
#include "StarColor.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarLua.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.periodic_function;
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarNetElement.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;
#include "StarLogging.hpp"

// Match module include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarScriptableThread.hpp"
#include "StarLuaRoot.hpp"
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;
import star.game_timers;
import star.actor_movement_controller;
import star.drawable;
import star.animation;
import star.particle;
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
import star.entity_rendering_types;
import star.lounging_entities;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
#include "StarLuaComponents.hpp"
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

import star.tick_rate_monitor;
import star.config_lua_bindings;

namespace Star {

ScriptableThread::ScriptableThread(Json parameters, LuaBaseComponent* parent)
  : Thread("ScriptableThread: " + parameters.getString("name")),
    m_parameters(std::move(parameters)),
    m_stop(false),
    m_errorOccurred(false),
    m_shouldExpire(false),
    m_parent(parent) {
      m_luaRoot = make_shared<LuaRoot>();
      m_luaRoot->luaEngine().setNullTerminated(false);
      m_luaRoot->tuneAutoGarbageCollection(m_parameters.getFloat("luaGcPause",1.2), m_parameters.getFloat("luaGcStepMultiplier",1.2));
      m_name = m_parameters.getString("name");
      m_logMapped = m_parameters.getBool("logMapped",true);
      
      m_timestep = 1.0f / m_parameters.getFloat("tickRate",60.0f);
      
      // since thread's not blocking anything important, allow modifying the instruction limit
      if (auto instructionLimit = m_parameters.optUInt("instructionLimit"))
        m_luaRoot->luaEngine().setInstructionLimit(instructionLimit.value());
        
      m_luaRoot->addCallbacks("thread", makeThreadCallbacks());
      m_luaRoot->addCallbacks(
          "config", LuaBindings::makeConfigCallbacks(bind(&ScriptableThread::configValue, this, _1, _2)));
}

ScriptableThread::~ScriptableThread() {
  m_stop = true;

  m_scriptContexts.clear();
  
  join();
}

void ScriptableThread::addCallbacks(String const& groupName, LuaCallbacks const& callbacks) {
  if (m_threadCallbacks.insert(groupName, callbacks).second) {
    for (auto const& p : m_scriptContexts) {
      p.second->addCallbacks(groupName,callbacks);
      p.second->addThreadCallbacks(groupName,callbacks);
    }
  }
}
void ScriptableThread::removeCallbacks(String const& groupName) {
  if (m_threadCallbacks.remove(groupName)) {
    for (auto const& p : m_scriptContexts) {
      p.second->removeCallbacks(groupName);
      p.second->removeThreadCallbacks(groupName);
    }
  }
}

void ScriptableThread::start() {
  m_stop = false;
  m_errorOccurred = false;
  Thread::start();
}

void ScriptableThread::stop() {
  m_stop = true;
  Thread::join();
}

void ScriptableThread::setPause(bool pause) {
  m_pause = pause;
}

bool ScriptableThread::errorOccurred() {
  return m_errorOccurred;
}

bool ScriptableThread::shouldExpire() {
  return m_shouldExpire;
}

void ScriptableThread::passMessage(Message&& message) {
  RecursiveMutexLocker locker(m_messageMutex);
  m_messages.append(std::move(message));
}

void ScriptableThread::run() {
  try {
    for (auto& p : m_parameters.getObject("scripts")) {
      auto scriptComponent = make_shared<ScriptComponent>();
      scriptComponent->setLuaRoot(m_luaRoot);
      scriptComponent->setScripts(jsonToStringList(p.second.toArray()));
      
      for (auto const& callbackPair : m_threadCallbacks) {
        scriptComponent->addCallbacks(callbackPair.first, callbackPair.second);
        scriptComponent->addThreadCallbacks(callbackPair.first, callbackPair.second);
      }

      m_scriptContexts.set(p.first, scriptComponent);
      scriptComponent->init();
    }
    
    double updateMeasureWindow = m_parameters.getDouble("updateMeasureWindow",0.5);
    TickRateApproacher tickApproacher(1.0f / m_timestep, updateMeasureWindow);

    while (!m_stop && !m_errorOccurred) {
      if (m_logMapped)
        LogMap::set(strf("lua_{}_update", m_name), strf("{:4.2f}Hz", tickApproacher.rate()));

      update();
      tickApproacher.setTargetTickRate(1.0f / m_timestep);
      tickApproacher.tick();

      double spareTime = tickApproacher.spareTime();

      int64_t spareMilliseconds = floor(spareTime * 1000);
      if (spareMilliseconds > 0)
        Thread::sleepPrecise(spareMilliseconds);
    }
  } catch (std::exception const& e) {
    Logger::error("ScriptableThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
  }
  for (auto& p : m_scriptContexts)
    p.second->uninit();
}

Maybe<ChainableJsonMessageResponse> ScriptableThread::receiveMessage(String const& message, JsonArray const& args) {
  Maybe<ChainableJsonMessageResponse> result;
  for (auto& p : m_scriptContexts) {
    result = p.second->handleMessage(message, true, args);
    if (result)
      break;
  }
  return result;
}

void ScriptableThread::update() {
  float dt = m_timestep;
  
  if (dt > 0.0f && !m_pause) {
    for (auto& p : m_scriptContexts) {
      p.second->update(p.second->updateDt(dt));
    }
  }
  
  List<Message> messages;
  {
    RecursiveMutexLocker locker(m_messageMutex);
    messages = std::move(m_messages);
  }
  for (auto& message : messages) {
    if (auto resp = receiveMessage(message.message, message.args))
      if (resp->is<RpcPromise<Json>>())
        message.promise.chain(resp->get<RpcPromise<Json>>());
      else
        message.promise.fulfill(resp->get<Json>());
    else
      message.promise.fail("Message not handled by thread");
  }
  if (m_logMapped)
    LogMap::set(strf("lua_{}_lua_mem", m_name), m_luaRoot->luaMemoryUsage());
}

LuaCallbacks ScriptableThread::makeThreadCallbacks() {
  LuaCallbacks callbacks;

  callbacks.registerCallback("stop", [this]() {
      m_stop = true;
      m_shouldExpire = true;
    });

  callbacks.registerCallback("sendParentMessage", [this](String const& message, LuaVariadic<Json> args) {
      return m_parent->threadPassMessage(m_name, message, JsonArray::from(std::move(args)));
    });

  return callbacks;
}

Json ScriptableThread::configValue(String const& name, Json def) const {
  return m_parameters.get(name, std::move(def));
}
}
