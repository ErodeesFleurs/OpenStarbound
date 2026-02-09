#pragma once

#include "StarConfig.hpp"
#include "StarDrawable.hpp"
#include "StarEntity.hpp"
#include "StarGameTimers.hpp"
#include "StarItemDescriptor.hpp"
#include "StarLuaComponents.hpp"
#include "StarMovementController.hpp"
#include "StarNetElementSystem.hpp"
#include "StarScriptedEntity.hpp"

import std;

namespace Star {

class ItemDrop : public virtual Entity, public virtual ScriptedEntity {
public:
  // Creates a drop at the given position and adds a hard-coded amount of
  // randomness to the drop position / velocity.
  static auto createRandomizedDrop(Ptr<Item> const& item, Vec2F const& position, bool eternal = false) -> Ptr<ItemDrop>;
  static auto createRandomizedDrop(ItemDescriptor const& itemDescriptor, Vec2F const& position, bool eternal = false) -> Ptr<ItemDrop>;

  // Create a drop and throw in the given direction with a hard-coded initial
  // throw velocity (unrelated to magnitude of direction, direction is
  // normalized first).  Initially intangible for 1 second.
  static auto throwDrop(Ptr<Item> const& item, Vec2F const& position, Vec2F const& velocity, Vec2F const& direction, bool eternal = false) -> Ptr<ItemDrop>;
  static auto throwDrop(ItemDescriptor const& itemDescriptor, Vec2F const& position, Vec2F const& velocity, Vec2F const& direction, bool eternal = false) -> Ptr<ItemDrop>;

  ItemDrop(Ptr<Item> item);
  ItemDrop(Json const& diskStore);
  ItemDrop(ByteArray netStore, NetCompatibilityRules rules = {});

  auto diskStore() const -> Json;
  auto netStore(NetCompatibilityRules rules = {}) const -> ByteArray;

  auto entityType() const -> EntityType override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  auto name() const -> String override;
  auto description() const -> String override;

  auto writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t> override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  auto position() const -> Vec2F override;
  auto metaBoundBox() const -> RectF override;

  auto ephemeral() const -> bool override;

  auto collisionArea() const -> RectF override;

  void update(float dt, std::uint64_t currentStep) override;

  auto shouldDestroy() const -> bool override;

  void render(RenderCallback* renderCallback) override;
  void renderLightSources(RenderCallback* renderCallback) override;
  // The item that this drop contains
  auto item() const -> Ptr<Item>;

  void setEternal(bool eternal);

  // If intangibleTime is set, will be intangible and unable to be picked up
  // until that amount of time has passed.
  void setIntangibleTime(float intangibleTime);

  // Mark this drop as taken by the given entity.  The drop will animate
  // towards them for a while and then disappear.
  auto takeBy(EntityId entityId, float timeOffset = 0.0f) -> Ptr<Item>;

  // Mark this drop as taken, but do not animate it towards a player simply
  // disappear next step.
  auto take() -> Ptr<Item>;

  // Item is not taken and is not intangible
  auto canTake() const -> bool;

  void setPosition(Vec2F const& position);

  auto velocity() const -> Vec2F;
  void setVelocity(Vec2F const& position);

  auto configValue(String const& name, Json const& def = Json()) const -> Json;

  auto callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue> override;
  auto evalScript(String const& code) -> std::optional<LuaValue> override;

  auto clientEntityMode() const -> ClientEntityMode override;

private:
  enum class Mode { Intangible,
                    Available,
                    Taken,
                    Dead };
  static EnumMap<Mode> const ModeNames;

  ItemDrop();

  // Set the movement controller's collision poly to match the
  // item drop drawables
  void updateCollisionPoly();

  void updateTaken(bool master);

  auto makeItemDropCallbacks() -> LuaCallbacks;

  Json m_config;
  Json m_parameters;
  Ptr<Item> m_item;
  RectF m_boundBox;
  float m_afterTakenLife;
  float m_overheadTime;
  float m_pickupDistance;
  float m_velocity;
  float m_velocityApproach;
  float m_overheadApproach;
  Vec2F m_overheadOffset;

  float m_combineChance;
  float m_combineRadius;
  double m_ageItemsEvery;

  NetElementTopGroup m_netGroup;
  NetElementEnum<Mode> m_mode;
  NetElementIntegral<EntityId> m_owningEntity;
  NetElementData<ItemDescriptor> m_itemDescriptor;
  MovementController m_movementController;
  RectF m_defaultBoundBox;

  // Only updated on master
  bool m_eternal;
  EpochTimer m_dropAge;
  GameTimer m_intangibleTimer;
  EpochTimer m_ageItemsTimer;

  bool m_drawRarityBeam;
  bool m_overForeground;
  std::optional<List<Drawable>> m_drawables;

  ClientEntityMode m_clientEntityMode;

  mutable LuaMessageHandlingComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptComponent;
  std::optional<Mode> m_overrideMode;
};

}// namespace Star
