#pragma once

#include "StarNetElementSystem.hpp"
#include "StarMovementController.hpp"
#include "StarItemDescriptor.hpp"
#include "StarGameTimers.hpp"
#include "StarEntity.hpp"
#include "StarScriptedEntity.hpp"
#include "StarDrawable.hpp"
#include "StarLuaComponents.hpp"
#include "StarAssets.hpp"
#include "StarItemDatabase.hpp"

namespace Star {

class Item;
using ItemPtr = SharedPtr<Item>;
class ItemDrop;
using ItemDropPtr = SharedPtr<ItemDrop>;

class ItemDrop : public virtual Entity, public virtual ScriptedEntity {
public:
  // Creates a drop at the given position and adds a hard-coded amount of
  // randomness to the drop position / velocity.
  [[nodiscard]] static ItemDropPtr createRandomizedDrop(ItemPtr const& item, Vec2F const& position, bool eternal, AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase);
  [[nodiscard]] static ItemDropPtr createRandomizedDrop(ItemDescriptor const& itemDescriptor, Vec2F const& position, bool eternal, AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase);

  // Create a drop and throw in the given direction with a hard-coded initial
  // throw velocity (unrelated to magnitude of direction, direction is
  // normalized first).  Initially intangible for 1 second.
  [[nodiscard]] static ItemDropPtr throwDrop(ItemPtr const& item, Vec2F const& position, Vec2F const& velocity, Vec2F const& direction, bool eternal, AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase);
  [[nodiscard]] static ItemDropPtr throwDrop(ItemDescriptor const& itemDescriptor, Vec2F const& position, Vec2F const& velocity, Vec2F const& direction, bool eternal, AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase);

  ItemDrop(ItemPtr item, AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase);
  ItemDrop(Json const& diskStore, AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase);
  ItemDrop(ByteArray netStore, NetCompatibilityRules rules, AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase);

  [[nodiscard]] Json diskStore() const;
  [[nodiscard]] ByteArray netStore(NetCompatibilityRules rules = {}) const;

  [[nodiscard]] EntityType entityType() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  [[nodiscard]] String name() const override;
  [[nodiscard]] String description() const override;

  [[nodiscard]] pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  [[nodiscard]] Vec2F position() const override;
  [[nodiscard]] RectF metaBoundBox() const override;

  [[nodiscard]] bool ephemeral() const override;

  [[nodiscard]] RectF collisionArea() const override;

  void update(float dt, uint64_t currentStep) override;

  [[nodiscard]] bool shouldDestroy() const override;

  void render(RenderCallback* renderCallback) override;
  void renderLightSources(RenderCallback* renderCallback) override;
  // The item that this drop contains
  [[nodiscard]] ItemPtr item() const;

  void setEternal(bool eternal);

  // If intangibleTime is set, will be intangible and unable to be picked up
  // until that amount of time has passed.
  void setIntangibleTime(float intangibleTime);

  // Mark this drop as taken by the given entity.  The drop will animate
  // towards them for a while and then disappear.
  [[nodiscard]] ItemPtr takeBy(EntityId entityId, float timeOffset = 0.0f);

  // Mark this drop as taken, but do not animate it towards a player simply
  // disappear next step.
  [[nodiscard]] ItemPtr take();

  // Item is not taken and is not intangible
  [[nodiscard]] bool canTake() const;

  void setPosition(Vec2F const& position);

  [[nodiscard]] Vec2F velocity() const;
  void setVelocity(Vec2F const& position);
  
  [[nodiscard]] Json configValue(String const& name, Json const& def = Json()) const;
  
  [[nodiscard]] Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  [[nodiscard]] Maybe<LuaValue> evalScript(String const& code) override;
  
  [[nodiscard]] ClientEntityMode clientEntityMode() const override;

private:
  enum class Mode { Intangible, Available, Taken, Dead };
  static EnumMap<Mode> const ModeNames;

  ItemDrop(AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase);

  // Set the movement controller's collision poly to match the
  // item drop drawables
  void updateCollisionPoly();

  void updateTaken(bool master);
  
  [[nodiscard]] LuaCallbacks makeItemDropCallbacks();

  Json m_config;
  Json m_parameters;
  ItemPtr m_item;
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
  ItemDatabaseConstPtr m_itemDatabase;

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
  Maybe<List<Drawable>> m_drawables;
  
  ClientEntityMode m_clientEntityMode;
  
  mutable LuaMessageHandlingComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptComponent;
  Maybe<Mode> m_overrideMode;
};

}
