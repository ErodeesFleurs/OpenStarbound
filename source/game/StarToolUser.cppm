module;
import std;
import star.set;
import star.json;
import star.net_compatibility;
import star.id_map;
import star.game_types;
import star.string;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.bi_map;
import star.strong_typedef;
import star.vector;
import star.variant;
import star.casting;

import star.drawable;
import star.item_descriptor;
import star.status_types;
import star.damage_types;
import star.world_geometry;
import star.damage;
import star.entity_rendering_types;
import star.entity;
import star.collision_block;
import star.force_regions;
import star.physics_entity;

import star.light_source;

import star.data_stream_devices;
import star.net_element;
import star.net_element_basic_fields;
import star.net_element_float_fields;
import star.net_element_sync_group;

namespace Star {

class Humanoid;

class ToolUserEntity;
class Item;
class World;
class EffectEmitter;
class Root;

class ToolUser : public NetElementSyncGroup {
public:
  explicit ToolUser(Root& root);

  Json diskStore() const;
  void diskLoad(Json const& diskStore);

  void init(ToolUserEntity* user);
  void uninit();

  std::shared_ptr<Item> primaryHandItem() const;
  std::shared_ptr<Item> altHandItem() const;
  ItemDescriptor primaryHandItemDescriptor() const;
  ItemDescriptor altHandItemDescriptor() const;

  List<LightSource> lightSources() const;
  void effects(EffectEmitter& emitter) const;
  List<PersistentStatusEffect> statusEffects() const;

  std::optional<float> toolRadius() const;
  // FIXME: There is a render method in ToolUser, why can't this be rendered
  // with the rest of everything else, there are TILE previews and OBJECT
  // previews, but of course one has to go through the render method and the
  // other has to be rendered separately.
  List<Drawable> renderObjectPreviews(Vec2F aimPosition, Direction walkingDirection, bool inToolRange, Color favoriteColor);
  // Returns the facing override direciton if there is one
  std::optional<Direction> setupHumanoidHandItems(Humanoid& humanoid, Vec2F position, Vec2F aimPosition) const;
  void setupHumanoidHandItemDrawables(Humanoid& humanoid) const;

  Vec2F armPosition(Humanoid const& humanoid, ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset) const;
  Vec2F handOffset(Humanoid const& humanoid, ToolHand hand, Direction facingDirection) const;
  Vec2F handPosition(ToolHand hand, Humanoid const& humanoid, Vec2F const& handOffset) const;
  bool queryShieldHit(DamageSource const& source) const;

  void tick(float dt, bool shifting, HashSet<MoveControlType> const& moves);

  void beginPrimaryFire();
  void beginAltFire();
  void endPrimaryFire();
  void endAltFire();

  bool firingPrimary() const;
  bool firingAlt() const;

  List<DamageSource> damageSources() const;
  List<PhysicsForceRegion> forceRegions() const;

  void render(RenderCallback* renderCallback, bool inToolRange, bool shifting, EntityRenderLayer renderLayer);

  void setItems(std::shared_ptr<Item> primaryHandItem, std::shared_ptr<Item> altHandItem);

  void suppressItems(bool suppress);

  std::optional<ChainableJsonMessageResponse> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});

  float beamGunRadius() const;

private:
  class NetItem : public NetElement {
  public:
    explicit NetItem(Root& root);

    void initNetVersion(NetElementVersion const* version = nullptr) override;

    void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
    void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

    void enableNetInterpolation(float extrapolationHint = 0.0f) override;
    void disableNetInterpolation() override;
    void tickNetInterpolation(float dt) override;

    bool writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules = {}) const override;
    void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
    void blankNetDelta(float interpolationTime) override;

    std::shared_ptr<Item> const& get() const;
    void set(std::shared_ptr<Item> item);

    bool pullNewItem();

  private:
    void updateItemDescriptor();

    Root* m_root;
    NetElementData<ItemDescriptor> m_itemDescriptor;
    std::shared_ptr<Item> m_item;
    NetElementVersion const* m_netVersion = nullptr;
    bool m_netInterpolationEnabled = false;
    float m_netExtrapolationHint = 0;
    bool m_newItem = false;
    mutable DataStreamBuffer m_buffer;
  };

  void initPrimaryHandItem();
  void initAltHandItem();
  void uninitItem(std::shared_ptr<Item> const& item);

  void netElementsNeedLoad(bool full) override;
  void netElementsNeedStore() override;

  // Borrowed for this tool user's lifetime; catalog lookups remain reload-aware.
  Root* m_root;

  float m_beamGunRadius;
  unsigned m_beamGunGlowBorder;
  float m_objectPreviewInnerAlpha;
  float m_objectPreviewOuterAlpha;

  ToolUserEntity* m_user;

  NetItem m_primaryHandItem;
  NetItem m_altHandItem;

  bool m_fireMain;
  bool m_fireAlt;
  bool m_edgeTriggeredMain;
  bool m_edgeTriggeredAlt;
  bool m_edgeSuppressedMain;
  bool m_edgeSuppressedAlt;

  NetElementBool m_suppress;

  NetElementFloat m_primaryFireTimerNetState;
  NetElementFloat m_altFireTimerNetState;
  NetElementFloat m_primaryTimeFiringNetState;
  NetElementFloat m_altTimeFiringNetState;
  NetElementBool m_primaryItemActiveNetState;
  NetElementBool m_altItemActiveNetState;

  List<Drawable> m_cachedObjectPreview;
  Vec2I m_cachedObjectPreviewPosition;
  std::shared_ptr<Item> m_cachedObjectItem;
};

}

export module star.tool_user;

export namespace Star {
  using ::Star::ToolUserEntity;
  using ::Star::Item;
  using ::Star::World;
  using ::Star::EffectEmitter;
  using ::Star::ToolUser;
}
