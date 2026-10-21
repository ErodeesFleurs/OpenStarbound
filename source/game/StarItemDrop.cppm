module;
import std;
import star.type_identity;
import star.json;
import star.id_map;
import star.game_types;
import star.casting;
import star.vector;
import star.strong_typedef;
import star.list;
import star.rpc_promise;
import star.data_stream;
import star.bi_map;
import star.lua;
import star.string;
import star.color;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;

import star.lua_root;
import star.lua_components;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
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
import star.item_descriptor;
import star.mobile_entity;
import star.scripted_entity;
import star.drawable;
import star.game_timers;

import star.net_element_basic_fields;
import star.net_element_system;

import star.net_compatibility;
import star.byte_array;
namespace Star {
class Root;

class Item;

class ItemDrop : public virtual MobileEntity, public virtual ScriptedEntity {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<ItemDrop, MobileEntity, ScriptedEntity>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<ItemDrop>();
  }

  // Creates a drop at the given position and adds a hard-coded amount of
  // randomness to the drop position / velocity.
  static std::shared_ptr<ItemDrop> createRandomizedDrop(Root& rootContext, std::shared_ptr<Item> const& item, Vec2F const& position, bool eternal = false);
  static std::shared_ptr<ItemDrop> createRandomizedDrop(Root& rootContext, ItemDescriptor const& itemDescriptor, Vec2F const& position, bool eternal = false);

  // Create a drop and throw in the given direction with a hard-coded initial
  // throw velocity (unrelated to magnitude of direction, direction is
  // normalized first).  Initially intangible for 1 second.
  static std::shared_ptr<ItemDrop> throwDrop(Root& rootContext, std::shared_ptr<Item> const& item, Vec2F const& position, Vec2F const& velocity, Vec2F const& direction, bool eternal = false);
  static std::shared_ptr<ItemDrop> throwDrop(Root& rootContext, ItemDescriptor const& itemDescriptor, Vec2F const& position, Vec2F const& velocity, Vec2F const& direction, bool eternal = false);

  ItemDrop(Root& rootContext, std::shared_ptr<Item> item);
  ItemDrop(Root& rootContext, Json const& diskStore);
  ItemDrop(Root& rootContext, ByteArray netStore, NetCompatibilityRules rules = {});

  Json diskStore() const;
  ByteArray netStore(NetCompatibilityRules rules = {}) const;

  EntityType entityType() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  String name() const override;
  String description() const override;

  std::pair<ByteArray, std::uint64_t> writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  Vec2F position() const override;
  RectF metaBoundBox() const override;

  bool ephemeral() const override;

  RectF collisionArea() const override;

  void update(float dt, std::uint64_t currentStep) override;

  bool shouldDestroy() const override;

  virtual void render(RenderCallback* renderCallback) override;
  virtual void renderLightSources(RenderCallback* renderCallback) override;
  
  MovementController* movementController() override;
  
  // The item that this drop contains
  std::shared_ptr<Item> item() const;

  void setEternal(bool eternal);

  // If intangibleTime is set, will be intangible and unable to be picked up
  // until that amount of time has passed.
  void setIntangibleTime(float intangibleTime);

  // Mark this drop as taken by the given entity.  The drop will animate
  // towards them for a while and then disappear.
  std::shared_ptr<Item> takeBy(EntityId entityId, float timeOffset = 0.0f);

  // Mark this drop as taken, but do not animate it towards a player simply
  // disappear next step.
  std::shared_ptr<Item> take();

  // Item is not taken and is not intangible
  bool canTake() const;

  void setPosition(Vec2F const& position);

  Vec2F velocity() const;
  void setVelocity(Vec2F const& position);
  
  Json configValue(String const& name, Json const& def = Json()) const;
  
  std::optional<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args) override;
  std::optional<LuaValue> evalScript(String const& code) override;
  
  ClientEntityMode clientEntityMode() const override;

private:
  enum class Mode { Intangible, Available, Taken, Dead };
  static EnumMap<Mode> const ModeNames;

  ItemDrop(Root& rootContext);

  // Set the movement controller's collision poly to match the
  // item drop drawables
  void updateCollisionPoly();

  void updateTaken(bool master);
  
  LuaCallbacks makeItemDropCallbacks();

  Json m_config;
  Json m_parameters;
  std::shared_ptr<Item> m_item;
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

}

export module star.item_drop;

export namespace Star {
  using ::Star::Item;
  using ::Star::ItemDrop;
}
