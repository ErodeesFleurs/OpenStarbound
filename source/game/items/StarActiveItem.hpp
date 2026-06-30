#pragma once

#include "StarAssets.hpp"
#include "StarDurabilityItem.hpp"
#include "StarItem.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarToolUserItem.hpp"

namespace Star {

class AudioInstance;
using AudioInstancePtr = SharedPtr<AudioInstance>;
class ParticleDatabase;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;
class ActiveItem;

class ActiveItem : public Item,
                   public DurabilityItem,
                   public virtual ToolUserItem,
                   public virtual NetElementGroup {
public:
  ActiveItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, ParticleDatabaseConstPtr particleDatabase, Json const& config, String const& directory, Json const& parameters = JsonObject());
  ActiveItem(ActiveItem const& rhs);

  [[nodiscard]] ItemPtr clone() const override;

  void init(ToolUserEntity& owner, ToolHand hand) override;
  void uninit() override;

  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  [[nodiscard]] List<DamageSource> damageSources() const override;
  [[nodiscard]] List<PolyF> shieldPolys() const override;

  [[nodiscard]] List<PhysicsForceRegion> forceRegions() const override;

  [[nodiscard]] bool holdingItem() const;
  [[nodiscard]] Maybe<String> backArmFrame() const;
  [[nodiscard]] Maybe<String> frontArmFrame() const;
  [[nodiscard]] bool twoHandedGrip() const;
  [[nodiscard]] bool recoil() const;
  [[nodiscard]] bool outsideOfHand() const;

  [[nodiscard]] float armAngle() const;
  [[nodiscard]] Maybe<Direction> facingDirection() const;

  // Hand drawables are in hand-space, everything else is in world space.
  [[nodiscard]] List<Drawable> handDrawables() const;
  [[nodiscard]] List<pair<Drawable, Maybe<EntityRenderLayer>>> entityDrawables() const;
  [[nodiscard]] List<LightSource> lights() const;
  [[nodiscard]] List<AudioInstancePtr> pullNewAudios();
  [[nodiscard]] List<Particle> pullNewParticles();

  [[nodiscard]] Maybe<String> cursor() const;

  [[nodiscard]] Maybe<Json> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});

  [[nodiscard]] float durabilityStatus() override;

private:
  [[nodiscard]] Vec2F armPosition(Vec2F const& offset) const;
  [[nodiscard]] Vec2F handPosition(Vec2F const& offset) const;

  [[nodiscard]] LuaCallbacks makeActiveItemCallbacks();
  [[nodiscard]] LuaCallbacks makeScriptedAnimationCallbacks();

  AssetsConstPtr m_assets;
  ParticleDatabaseConstPtr m_particleDatabase;

  mutable LuaMessageHandlingComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaStorableComponent<LuaWorldComponent<LuaBaseComponent>>>>> m_script;

  NetworkedAnimator m_itemAnimator;
  NetworkedAnimator::DynamicTarget m_itemAnimatorDynamicTarget;

  mutable LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;

  HashMap<AudioInstancePtr, Vec2F> m_activeAudio;

  FireMode m_currentFireMode;
  Maybe<String> m_cursor;

  NetElementBool m_holdingItem;
  NetElementData<Maybe<String>> m_backArmFrame;
  NetElementData<Maybe<String>> m_frontArmFrame;
  NetElementBool m_twoHandedGrip;
  NetElementBool m_recoil;
  NetElementBool m_outsideOfHand;
  NetElementFloat m_armAngle;
  NetElementData<Maybe<Direction>> m_facingDirection;
  NetElementData<List<DamageSource>> m_damageSources;
  NetElementData<List<DamageSource>> m_itemDamageSources;
  NetElementData<List<PolyF>> m_shieldPolys;
  NetElementData<List<PolyF>> m_itemShieldPolys;
  NetElementData<List<PhysicsForceRegion>> m_forceRegions;
  NetElementData<List<PhysicsForceRegion>> m_itemForceRegions;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
};

}// namespace Star
