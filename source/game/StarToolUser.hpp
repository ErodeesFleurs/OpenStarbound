#pragma once

#include "StarConfig.hpp"
#include "StarDamage.hpp"
#include "StarEffectEmitter.hpp"
#include "StarEntityRenderingTypes.hpp"
#include "StarForceRegions.hpp"
#include "StarHumanoid.hpp"
#include "StarItemDescriptor.hpp"
#include "StarLightSource.hpp"
#include "StarStatusTypes.hpp"

namespace Star {

class ToolUserEntity;
class ObjectItem;

class ToolUser : public NetElementSyncGroup {
public:
  ToolUser();

  auto diskStore() const -> Json;
  void diskLoad(Json const& diskStore);

  void init(ToolUserEntity* user);
  void uninit();

  auto primaryHandItem() const -> Ptr<Item>;
  auto altHandItem() const -> Ptr<Item>;
  auto primaryHandItemDescriptor() const -> ItemDescriptor;
  auto altHandItemDescriptor() const -> ItemDescriptor;

  auto lightSources() const -> List<LightSource>;
  void effects(EffectEmitter& emitter) const;
  auto statusEffects() const -> List<PersistentStatusEffect>;

  auto toolRadius() const -> std::optional<float>;
  // FIXME: There is a render method in ToolUser, why can't this be rendered
  // with the rest of everything else, there are TILE previews and OBJECT
  // previews, but of course one has to go through the render method and the
  // other has to be rendered separately.
  auto renderObjectPreviews(Vec2F aimPosition, Direction walkingDirection, bool inToolRange, Color favoriteColor) -> List<Drawable>;
  // Returns the facing override direciton if there is one
  auto setupHumanoidHandItems(Humanoid& humanoid, Vec2F position, Vec2F aimPosition) const -> std::optional<Direction>;
  void setupHumanoidHandItemDrawables(Humanoid& humanoid) const;

  auto armPosition(Humanoid const& humanoid, ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset) const -> Vec2F;
  auto handOffset(Humanoid const& humanoid, ToolHand hand, Direction facingDirection) const -> Vec2F;
  auto handPosition(ToolHand hand, Humanoid const& humanoid, Vec2F const& handOffset) const -> Vec2F;
  auto queryShieldHit(DamageSource const& source) const -> bool;

  void tick(float dt, bool shifting, HashSet<MoveControlType> const& moves);

  void beginPrimaryFire();
  void beginAltFire();
  void endPrimaryFire();
  void endAltFire();

  auto firingPrimary() const -> bool;
  auto firingAlt() const -> bool;

  auto damageSources() const -> List<DamageSource>;
  auto forceRegions() const -> List<PhysicsForceRegion>;

  void render(RenderCallback* renderCallback, bool inToolRange, bool shifting, EntityRenderLayer renderLayer);

  void setItems(Ptr<Item> primaryHandItem, Ptr<Item> altHandItem);

  void suppressItems(bool suppress);

  auto receiveMessage(String const& message, bool localMessage, JsonArray const& args = {}) -> std::optional<Json>;

  auto beamGunRadius() const -> float;

private:
  class NetItem : public NetElement {
  public:
    void initNetVersion(NetElementVersion const* version = nullptr) override;

    void netStore(DataStream& ds, NetCompatibilityRules rules = {}) const override;
    void netLoad(DataStream& ds, NetCompatibilityRules rules) override;

    void enableNetInterpolation(float extrapolationHint = 0.0f) override;
    void disableNetInterpolation() override;
    void tickNetInterpolation(float dt) override;

    auto writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules = {}) const -> bool override;
    void readNetDelta(DataStream& ds, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
    void blankNetDelta(float interpolationTime) override;

    auto get() const -> Ptr<Item> const&;
    void set(Ptr<Item> item);

    auto pullNewItem() -> bool;

  private:
    void updateItemDescriptor();

    NetElementData<ItemDescriptor> m_itemDescriptor;
    Ptr<Item> m_item;
    NetElementVersion const* m_netVersion = nullptr;
    bool m_netInterpolationEnabled = false;
    float m_netExtrapolationHint = 0;
    bool m_newItem = false;
    mutable DataStreamBuffer m_buffer;
  };

  void initPrimaryHandItem();
  void initAltHandItem();
  void uninitItem(Ptr<Item> const& item);

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
  Ptr<ObjectItem> m_cachedObjectItem;
};

}// namespace Star
