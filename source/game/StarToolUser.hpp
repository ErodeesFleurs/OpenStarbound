#pragma once

#include "StarHumanoid.hpp"
#include "StarNetElementSystem.hpp"
#include "StarItemDescriptor.hpp"
#include "StarStatusTypes.hpp"
#include "StarLightSource.hpp"
#include "StarDamage.hpp"
#include "StarEffectEmitter.hpp"
#include "StarEntityRenderingTypes.hpp"
#include "StarPhysicsEntity.hpp"
#include "StarAssets.hpp"
#include "StarItemDatabase.hpp"
#include "StarObjectDatabase.hpp"

namespace Star {

class ObjectItem;
using ObjectItemPtr = SharedPtr<ObjectItem>;
class ToolUserEntity;
class Item;
using ItemPtr = SharedPtr<Item>;
class ToolUser;
using ToolUserPtr = SharedPtr<ToolUser>;

class ToolUser : public NetElementSyncGroup {
public:
  ToolUser(AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase);

  ToolUser(AssetsConstPtr assets, ToolUserEntity& user, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase);

  [[nodiscard]] Json diskStore() const;
  void diskLoad(Json const& diskStore);

  void init(ToolUserEntity& user);
  void uninit();

  [[nodiscard]] ItemPtr primaryHandItem() const;
  [[nodiscard]] ItemPtr altHandItem() const;
  [[nodiscard]] ItemDescriptor primaryHandItemDescriptor() const;
  [[nodiscard]] ItemDescriptor altHandItemDescriptor() const;

  [[nodiscard]] List<LightSource> lightSources() const;
  void effects(EffectEmitter& emitter) const;
  [[nodiscard]] List<PersistentStatusEffect> statusEffects() const;

  [[nodiscard]] Maybe<float> toolRadius() const;
  // FIXME: There is a render method in ToolUser, why can't this be rendered
  // with the rest of everything else, there are TILE previews and OBJECT
  // previews, but of course one has to go through the render method and the
  // other has to be rendered separately.
  [[nodiscard]] List<Drawable> renderObjectPreviews(Vec2F aimPosition, Direction walkingDirection, bool inToolRange, Color favoriteColor);
  // Returns the facing override direciton if there is one
  [[nodiscard]] Maybe<Direction> setupHumanoidHandItems(Humanoid& humanoid, Vec2F position, Vec2F aimPosition) const;
  void setupHumanoidHandItemDrawables(Humanoid& humanoid) const;

  [[nodiscard]] Vec2F armPosition(Humanoid const& humanoid, ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset) const;
  [[nodiscard]] Vec2F handOffset(Humanoid const& humanoid, ToolHand hand, Direction facingDirection) const;
  [[nodiscard]] Vec2F handPosition(ToolHand hand, Humanoid const& humanoid, Vec2F const& handOffset) const;
  [[nodiscard]] bool queryShieldHit(DamageSource const& source) const;

  void tick(float dt, bool shifting, HashSet<MoveControlType> const& moves);

  void beginPrimaryFire();
  void beginAltFire();
  void endPrimaryFire();
  void endAltFire();

  [[nodiscard]] bool firingPrimary() const;
  [[nodiscard]] bool firingAlt() const;

  [[nodiscard]] List<DamageSource> damageSources() const;
  [[nodiscard]] List<PhysicsForceRegion> forceRegions() const;

  void render(RenderCallback* renderCallback, bool inToolRange, bool shifting, EntityRenderLayer renderLayer);

  void setItems(ItemPtr primaryHandItem, ItemPtr altHandItem);

  void suppressItems(bool suppress);

  [[nodiscard]] Maybe<Json> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});

  [[nodiscard]] float beamGunRadius() const;

private:
  class NetItem : public NetElement {
  public:
    NetItem(ItemDatabaseConstPtr itemDatabase = {});

    void initNetVersion(NetElementVersion const* version = nullptr) override;

    void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
    void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

    void enableNetInterpolation(float extrapolationHint = 0.0f) override;
    void disableNetInterpolation() override;
    void tickNetInterpolation(float dt) override;

    [[nodiscard]] bool writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules = {}) const override;
    void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
    void blankNetDelta(float interpolationTime) override;

    [[nodiscard]] ItemPtr const& get() const;
    void set(ItemPtr item);

    [[nodiscard]] bool pullNewItem();

  private:
    void updateItemDescriptor();

    NetElementData<ItemDescriptor> m_itemDescriptor;
    ItemPtr m_item;
    ItemDatabaseConstPtr m_itemDatabase;
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

  float m_beamGunRadius = 0;
  unsigned m_beamGunGlowBorder = 0;
  float m_objectPreviewInnerAlpha = 0;
  float m_objectPreviewOuterAlpha = 0;

  ToolUserEntity* m_user = nullptr;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;

  NetItem m_primaryHandItem;
  NetItem m_altHandItem;

  bool m_fireMain = false;
  bool m_fireAlt = false;
  bool m_edgeTriggeredMain = false;
  bool m_edgeTriggeredAlt = false;
  bool m_edgeSuppressedMain = false;
  bool m_edgeSuppressedAlt = false;

  NetElementBool m_suppress;

  NetElementFloat m_primaryFireTimerNetState;
  NetElementFloat m_altFireTimerNetState;
  NetElementFloat m_primaryTimeFiringNetState;
  NetElementFloat m_altTimeFiringNetState;
  NetElementBool m_primaryItemActiveNetState;
  NetElementBool m_altItemActiveNetState;
  bool m_primaryHandItemInitialized = false;
  bool m_altHandItemInitialized = false;

  List<Drawable> m_cachedObjectPreview;
  Vec2I m_cachedObjectPreviewPosition;
  ObjectItemPtr m_cachedObjectItem;
};

}
