#pragma once

#include "StarBehaviorState.hpp"
#include "StarConfig.hpp"
#include "StarEntity.hpp"
#include "StarLuaComponents.hpp"
#include "StarNetElementSystem.hpp"
#include "StarScriptedEntity.hpp"

import std;

namespace Star {

class Stagehand : public virtual ScriptedEntity {
public:
  Stagehand(Json const& config);
  Stagehand(ByteArray const& netStore, NetCompatibilityRules rules = {});

  auto diskStore() const -> Json;
  auto netStore(NetCompatibilityRules rules = {}) -> ByteArray;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  auto entityType() const -> EntityType override;

  void setPosition(Vec2F const& position);

  auto position() const -> Vec2F override;

  auto metaBoundBox() const -> RectF override;

  auto writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, uint64_t> override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  auto name() const -> String override;

  void update(float dt, uint64_t currentStep) override;

  auto shouldDestroy() const -> bool override;

  auto clientEntityMode() const -> ClientEntityMode override;

  auto callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> override;
  auto evalScript(String const& code) -> std::optional<LuaValue> override;

  auto typeName() const -> String;

  auto configValue(String const& name, Json const& def = Json()) const -> Json;

  auto receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) -> std::optional<Json> override;

  using Entity::setUniqueId;

private:
  Stagehand();

  void readConfig(Json config);

  auto makeStagehandCallbacks() -> LuaCallbacks;

  Json m_config;

  RectF m_boundBox;

  bool m_dead = false;

  ClientEntityMode m_clientEntityMode;

  NetElementTopGroup m_netGroup;

  NetElementFloat m_xPosition;
  NetElementFloat m_yPosition;

  NetElementData<std::optional<String>> m_uniqueIdNetState;

  bool m_scripted = false;
  List<Ptr<BehaviorState>> m_behaviors;
  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>
    m_scriptComponent;
};

}// namespace Star
