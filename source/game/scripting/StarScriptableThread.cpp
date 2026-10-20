import star.json;
import star.json_extra;
import star.id_map;
import star.casting;
import star.data_stream;
import star.strong_typedef;
import star.game_types;
import star.variant;
import star.list;
import star.rpc_promise;
import star.bi_map;
import star.a_star;
import star.string;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.lua;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.audio;
import star.map;
import star.set;
import star.io_device;
import star.ref_ptr;
import star.listener;
import std;
import star.format;
import star.version;
import star.logging;

import star.scriptable_thread;
import star.lua_root;
import star.lua_converters;
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
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_animation_component;
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

import star.math_common;
namespace Star {

ScriptableThread::ScriptableThread(Json parameters, LuaBaseComponent* parent)
  : Thread("ScriptableThread: " + parameters.getString("name")),
    m_parameters(std::move(parameters)),
    m_stop(false),
    m_errorOccurred(false),
    m_shouldExpire(false),
    m_parent(parent) {
      m_luaRoot = std::make_shared<LuaRoot>(parent->root());
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
          "config", LuaBindings::makeConfigCallbacks(std::bind(&ScriptableThread::configValue, this, std::placeholders::_1, std::placeholders::_2)));
}

ScriptableThread::~ScriptableThread() {
  stop();
  m_scriptContexts.clear();
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
      auto scriptComponent = std::make_shared<ScriptComponent>(m_luaRoot->root());
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
        m_luaRoot->root().logMap().set(strf("lua_{}_update", m_name), strf("{:4.2f}Hz", tickApproacher.rate()));

      update();
      tickApproacher.setTargetTickRate(1.0f / m_timestep);
      tickApproacher.tick();

      double spareTime = tickApproacher.spareTime();

      std::int64_t spareMilliseconds = std::floor(spareTime * 1000);
      if (spareMilliseconds > 0)
        Thread::sleepPrecise(spareMilliseconds);
    }
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    m_luaRoot->root().logger().error("ScriptableThread exception caught: {}", outputException(e, true));
    m_errorOccurred = true;
  }
  for (auto& p : m_scriptContexts)
    p.second->uninit();
}

std::optional<ChainableJsonMessageResponse> ScriptableThread::receiveMessage(String const& message, JsonArray const& args) {
  std::optional<ChainableJsonMessageResponse> result;
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
    m_luaRoot->root().logMap().set(strf("lua_{}_lua_mem", m_name), m_luaRoot->luaMemoryUsage());
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
