#include "StarIdMap.hpp"
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarGameTypes.hpp"
#include "StarDirectives.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetElementSystem.hpp"
#include "StarRpcPromise.hpp"
import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
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
#include "StarLuaRoot.hpp"
import star.force_regions;
import star.world;

import star.player_deployment;
#include "StarPlayer.hpp"

import star.player_lua_bindings;
import star.entity_lua_bindings;
import star.config_lua_bindings;
import star.status_controller_lua_bindings;

namespace Star {

PlayerDeployment::PlayerDeployment(Json const& config) : m_config(config) {
  m_deploying = false;
  m_deployed = false;
}

void PlayerDeployment::diskLoad(Json const& diskStore) {
  m_scriptComponent.setScriptStorage(diskStore.getObject("scriptStorage", JsonObject{}));
}

Json PlayerDeployment::diskStore() const {
  JsonObject result;
  result["scriptStorage"] = m_scriptComponent.getScriptStorage();
  return result;
}

void PlayerDeployment::init(Entity* player, World* world) {
  m_world = world;

  if (m_deploying) {
    m_deployed = true;
    m_deploying = false;
  } else {
    m_deployed = false;
  }

  m_scriptComponent.setScripts(jsonToStringList(m_config.getArray("scripts", JsonArray())));
  m_scriptComponent.setUpdateDelta(m_config.getInt("scriptDelta", 10));

  m_scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(player));
  m_scriptComponent.addCallbacks("player", LuaBindings::makePlayerCallbacks(as<Player>(player)));
  m_scriptComponent.addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(as<Player>(player)->statusController()));
  m_scriptComponent.addCallbacks("config",
      LuaBindings::makeConfigCallbacks([this](String const& name, Json const& def) { return m_config.query(name, def); }));

  m_scriptComponent.init(world);
}

bool PlayerDeployment::canDeploy() {
  Maybe<bool> res = m_scriptComponent.invoke<bool>("canDeploy");
  return res && *res;
}

void PlayerDeployment::setDeploying(bool deploying) {
  m_deploying = deploying;
}

bool PlayerDeployment::isDeploying() const {
  return m_deploying;
}

bool PlayerDeployment::isDeployed() const {
  return m_deployed;
}

void PlayerDeployment::uninit() {
  m_scriptComponent.uninit();
  m_scriptComponent.removeCallbacks("entity");
  m_scriptComponent.removeCallbacks("player");
  m_scriptComponent.removeCallbacks("status");
  m_scriptComponent.removeCallbacks("config");
  m_world = nullptr;
}

void PlayerDeployment::teleportOut() {
  m_scriptComponent.invoke("teleportOut");
}

Maybe<ChainableJsonMessageResponse> PlayerDeployment::receiveMessage(String const& message, bool localMessage, JsonArray const& args) {
  return m_scriptComponent.handleMessage(message, localMessage, args);
}

void PlayerDeployment::update(float dt) {
  m_scriptComponent.update(m_scriptComponent.updateDt(dt));
}

void PlayerDeployment::render(RenderCallback* renderCallback, Vec2F const& position) {
  for (auto drawablePair : m_scriptComponent.drawables()) {
    drawablePair.first.translate(position);
    renderCallback->addDrawable(drawablePair.first, drawablePair.second.value(RenderLayerPlayer));
  }
  renderCallback->addParticles(m_scriptComponent.pullNewParticles());
  for (auto audio : m_scriptComponent.pullNewAudios()) {
    audio->setPosition(position);
    renderCallback->addAudio(audio);
  }
}

void PlayerDeployment::renderLightSources(RenderCallback* renderCallback) {
  renderCallback->addLightSources(m_scriptComponent.lightSources());
}

}
