#pragma once

#include "StarEntity.hpp"
#include "StarLuaComponents.hpp"
#include "StarScriptedEntity.hpp"
#include "StarBehaviorState.hpp"
#include "StarNetElementSystem.hpp"
#include "StarStagehandDatabase.hpp"
#include "StarRoot.hpp"

namespace Star {

class Stagehand;

class Stagehand : public virtual ScriptedEntity {
public:
  Stagehand(Json const& config);
  Stagehand(ByteArray const& netStore, NetCompatibilityRules rules = {});

  [[nodiscard]] Json diskStore() const;
  [[nodiscard]] ByteArray netStore(NetCompatibilityRules rules = {});

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  [[nodiscard]] EntityType entityType() const override;

  void setPosition(Vec2F const& position);

  [[nodiscard]] Vec2F position() const override;

  [[nodiscard]] RectF metaBoundBox() const override;

  [[nodiscard]] pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  [[nodiscard]] String name() const override;

  void update(float dt, uint64_t currentStep) override;

  [[nodiscard]] bool shouldDestroy() const override;
  
  [[nodiscard]] ClientEntityMode clientEntityMode() const override;

  [[nodiscard]] Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  [[nodiscard]] Maybe<LuaValue> evalScript(String const& code) override;

  [[nodiscard]] String typeName() const;
  
  [[nodiscard]] Json configValue(String const& name, Json const& def = Json()) const;

  [[nodiscard]] Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  using Entity::setUniqueId;

private:
  Stagehand();

  void readConfig(Json config);

  [[nodiscard]] LuaCallbacks makeStagehandCallbacks();

  Json m_config;

  RectF m_boundBox;

  bool m_dead = false;
      
  ClientEntityMode m_clientEntityMode;

  NetElementTopGroup m_netGroup;

  NetElementFloat m_xPosition;
  NetElementFloat m_yPosition;

  NetElementData<Maybe<String>> m_uniqueIdNetState;

  bool m_scripted = false;
  List<BehaviorStatePtr> m_behaviors;
  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>
      m_scriptComponent;
};

}
