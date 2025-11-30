#pragma once

// ECS Stagehand Adapter for OpenStarbound
// This adapter implements the Stagehand entity using ECS components
// Stagehands are scripted entities used for world triggers and events

#include "StarEntityAdapter.hpp"
#include "StarStagehandDatabase.hpp"
#include "StarLuaComponents.hpp"
#include "StarBehaviorState.hpp"
#include "StarScriptedEntity.hpp"

namespace Star {
namespace ECS {

// Stagehand-specific component storing all stagehand state
struct StagehandDataComponent {
  // Configuration
  Json config;
  String typeName;
  RectF boundBox;
  
  // State
  bool dead = false;
  ClientEntityMode clientEntityMode = ClientEntityMode::ClientSlaveOnly;
  
  // Scripting
  bool scripted = false;
};

// Tag component for identifying stagehands
struct StagehandTag {};

// Stagehand adapter that wraps ECS entity
class StagehandAdapter : public EntityAdapter, public virtual ScriptedEntity {
public:
  // Create from config
  static shared_ptr<StagehandAdapter> create(
    World* ecsWorld,
    Json const& config);
  
  // Create from network data
  static shared_ptr<StagehandAdapter> createFromNet(
    World* ecsWorld,
    ByteArray const& netStore,
    NetCompatibilityRules rules = {});
  
  // Construct from existing ECS entity
  StagehandAdapter(World* ecsWorld, Entity ecsEntity);
  
  // Serialization
  Json diskStore() const;
  ByteArray netStore(NetCompatibilityRules rules = {});
  
  // Entity interface
  EntityType entityType() const override;
  
  void init(Star::World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;
  
  String name() const override;
  
  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  
  Vec2F position() const override;
  void setPosition(Vec2F const& position);
  
  RectF metaBoundBox() const override;
  
  bool shouldDestroy() const override;
  ClientEntityMode clientEntityMode() const override;
  
  void update(float dt, uint64_t currentStep) override;
  
  // ScriptedEntity interface
  Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  Maybe<LuaValue> evalScript(String const& code) override;
  Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;
  
  // Stagehand-specific methods
  String typeName() const;
  Json configValue(String const& name, Json const& def = Json()) const;
  
  using Entity::setUniqueId;

private:
  void readConfig(Json config);
  LuaCallbacks makeStagehandCallbacks();
  
  // Network state
  NetElementTopGroup m_netGroup;
  NetElementFloat m_xPosition;
  NetElementFloat m_yPosition;
  NetElementData<Maybe<String>> m_uniqueIdNetState;
  
  // Scripting
  List<BehaviorStatePtr> m_behaviors;
  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>
      m_scriptComponent;
};

using StagehandAdapterPtr = shared_ptr<StagehandAdapter>;

} // namespace ECS
} // namespace Star
