module;
import std;
import star.set;
import star.maybe;
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
#include "StarStrongTypedef.hpp"
import star.vector;
import star.variant;
#include "StarCasting.hpp"

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

#include "StarDataStreamDevices.hpp"
import star.net_element;
import star.net_element_basic_fields;
import star.net_element_float_fields;
import star.net_element_sync_group;

namespace Star {

class Humanoid;

class ToolUserEntity;
using ToolUserEntityPtr = std::shared_ptr<ToolUserEntity>;
using ToolUserEntityConstPtr = std::shared_ptr<const ToolUserEntity>;
using ToolUserEntityWeakPtr = std::weak_ptr<ToolUserEntity>;
using ToolUserEntityConstWeakPtr = std::weak_ptr<const ToolUserEntity>;
using ToolUserEntityUPtr = std::unique_ptr<ToolUserEntity>;
using ToolUserEntityConstUPtr = std::unique_ptr<const ToolUserEntity>;
class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<const Item>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<const Item>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<const Item>;
class World;
using WorldPtr = std::shared_ptr<World>;
using WorldConstPtr = std::shared_ptr<const World>;
using WorldWeakPtr = std::weak_ptr<World>;
using WorldConstWeakPtr = std::weak_ptr<const World>;
using WorldUPtr = std::unique_ptr<World>;
using WorldConstUPtr = std::unique_ptr<const World>;
class EffectEmitter;
using EffectEmitterPtr = std::shared_ptr<EffectEmitter>;
using EffectEmitterConstPtr = std::shared_ptr<const EffectEmitter>;
using EffectEmitterWeakPtr = std::weak_ptr<EffectEmitter>;
using EffectEmitterConstWeakPtr = std::weak_ptr<const EffectEmitter>;
using EffectEmitterUPtr = std::unique_ptr<EffectEmitter>;
using EffectEmitterConstUPtr = std::unique_ptr<const EffectEmitter>;

class ToolUser;
using ToolUserPtr = std::shared_ptr<ToolUser>;
using ToolUserConstPtr = std::shared_ptr<const ToolUser>;
using ToolUserWeakPtr = std::weak_ptr<ToolUser>;
using ToolUserConstWeakPtr = std::weak_ptr<const ToolUser>;
using ToolUserUPtr = std::unique_ptr<ToolUser>;
using ToolUserConstUPtr = std::unique_ptr<const ToolUser>;

class ToolUser : public NetElementSyncGroup {
public:
  ToolUser();

  Json diskStore() const;
  void diskLoad(Json const& diskStore);

  void init(ToolUserEntity* user);
  void uninit();

  ItemPtr primaryHandItem() const;
  ItemPtr altHandItem() const;
  ItemDescriptor primaryHandItemDescriptor() const;
  ItemDescriptor altHandItemDescriptor() const;

  List<LightSource> lightSources() const;
  void effects(EffectEmitter& emitter) const;
  List<PersistentStatusEffect> statusEffects() const;

  Maybe<float> toolRadius() const;
  // FIXME: There is a render method in ToolUser, why can't this be rendered
  // with the rest of everything else, there are TILE previews and OBJECT
  // previews, but of course one has to go through the render method and the
  // other has to be rendered separately.
  List<Drawable> renderObjectPreviews(Vec2F aimPosition, Direction walkingDirection, bool inToolRange, Color favoriteColor);
  // Returns the facing override direciton if there is one
  Maybe<Direction> setupHumanoidHandItems(Humanoid& humanoid, Vec2F position, Vec2F aimPosition) const;
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

  void setItems(ItemPtr primaryHandItem, ItemPtr altHandItem);

  void suppressItems(bool suppress);

  Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});

  float beamGunRadius() const;

private:
  class NetItem : public NetElement {
  public:
    void initNetVersion(NetElementVersion const* version = nullptr) override;

    void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
    void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

    void enableNetInterpolation(float extrapolationHint = 0.0f) override;
    void disableNetInterpolation() override;
    void tickNetInterpolation(float dt) override;

    bool writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules = {}) const override;
    void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
    void blankNetDelta(float interpolationTime) override;

    ItemPtr const& get() const;
    void set(ItemPtr item);

    bool pullNewItem();

  private:
    void updateItemDescriptor();

    NetElementData<ItemDescriptor> m_itemDescriptor;
    ItemPtr m_item;
    NetElementVersion const* m_netVersion = nullptr;
    bool m_netInterpolationEnabled = false;
    float m_netExtrapolationHint = 0;
    bool m_newItem = false;
    mutable DataStreamBuffer m_buffer;
  };

  void initPrimaryHandItem();
  void initAltHandItem();
  void uninitItem(ItemPtr const& item);

  void netElementsNeedLoad(bool full) override;
  void netElementsNeedStore() override;

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
  ItemPtr m_cachedObjectItem;
};

}

export module star.tool_user;

export namespace Star {
  using ::Star::ToolUserEntity;
  using ::Star::ToolUserEntityPtr;
  using ::Star::ToolUserEntityConstPtr;
  using ::Star::ToolUserEntityWeakPtr;
  using ::Star::ToolUserEntityConstWeakPtr;
  using ::Star::ToolUserEntityUPtr;
  using ::Star::ToolUserEntityConstUPtr;
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::World;
  using ::Star::WorldPtr;
  using ::Star::WorldConstPtr;
  using ::Star::WorldWeakPtr;
  using ::Star::WorldConstWeakPtr;
  using ::Star::WorldUPtr;
  using ::Star::WorldConstUPtr;
  using ::Star::EffectEmitter;
  using ::Star::EffectEmitterPtr;
  using ::Star::EffectEmitterConstPtr;
  using ::Star::EffectEmitterWeakPtr;
  using ::Star::EffectEmitterConstWeakPtr;
  using ::Star::EffectEmitterUPtr;
  using ::Star::EffectEmitterConstUPtr;
  using ::Star::ToolUser;
  using ::Star::ToolUserPtr;
  using ::Star::ToolUserConstPtr;
  using ::Star::ToolUserWeakPtr;
  using ::Star::ToolUserConstWeakPtr;
  using ::Star::ToolUserUPtr;
  using ::Star::ToolUserConstUPtr;
}
