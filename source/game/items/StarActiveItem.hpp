#pragma once

#include "StarConfig.hpp"
#include "StarDurabilityItem.hpp"
#include "StarItem.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetworkedAnimator.hpp"
#include "StarToolUserItem.hpp"

import std;

namespace Star {

class ActiveItem : public Item,
                   public DurabilityItem,
                   public virtual ToolUserItem,
                   public virtual NetElementGroup {
public:
  ActiveItem(Json const& config, String const& directory, Json const& parameters = JsonObject());
  ActiveItem(ActiveItem const& rhs);

  auto clone() const -> Ptr<Item> override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void uninit() override;

  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  auto damageSources() const -> List<DamageSource> override;
  auto shieldPolys() const -> List<PolyF> override;

  auto forceRegions() const -> List<PhysicsForceRegion> override;

  auto holdingItem() const -> bool;
  auto backArmFrame() const -> std::optional<String>;
  auto frontArmFrame() const -> std::optional<String>;
  auto twoHandedGrip() const -> bool;
  auto recoil() const -> bool;
  auto outsideOfHand() const -> bool;

  auto armAngle() const -> float;
  auto facingDirection() const -> std::optional<Direction>;

  // Hand drawables are in hand-space, everything else is in world space.
  auto handDrawables() const -> List<Drawable>;
  auto entityDrawables() const -> List<std::pair<Drawable, std::optional<EntityRenderLayer>>>;
  auto lights() const -> List<LightSource>;
  auto pullNewAudios() -> List<Ptr<AudioInstance>>;
  auto pullNewParticles() -> List<Particle>;

  auto cursor() const -> std::optional<String>;

  auto receiveMessage(String const& message, bool localMessage, JsonArray const& args = {}) -> std::optional<Json>;

  auto durabilityStatus() -> float override;

private:
  auto armPosition(Vec2F const& offset) const -> Vec2F;
  auto handPosition(Vec2F const& offset) const -> Vec2F;

  auto makeActiveItemCallbacks() -> LuaCallbacks;
  auto makeScriptedAnimationCallbacks() -> LuaCallbacks;

  mutable LuaMessageHandlingComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaStorableComponent<LuaWorldComponent<LuaBaseComponent>>>>> m_script;

  NetworkedAnimator m_itemAnimator;
  NetworkedAnimator::DynamicTarget m_itemAnimatorDynamicTarget;

  mutable LuaAnimationComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>> m_scriptedAnimator;

  HashMap<Ptr<AudioInstance>, Vec2F> m_activeAudio;

  FireMode m_currentFireMode;
  std::optional<String> m_cursor;

  NetElementBool m_holdingItem;
  NetElementData<std::optional<String>> m_backArmFrame;
  NetElementData<std::optional<String>> m_frontArmFrame;
  NetElementBool m_twoHandedGrip;
  NetElementBool m_recoil;
  NetElementBool m_outsideOfHand;
  NetElementFloat m_armAngle;
  NetElementData<std::optional<Direction>> m_facingDirection;
  NetElementData<List<DamageSource>> m_damageSources;
  NetElementData<List<DamageSource>> m_itemDamageSources;
  NetElementData<List<PolyF>> m_shieldPolys;
  NetElementData<List<PolyF>> m_itemShieldPolys;
  NetElementData<List<PhysicsForceRegion>> m_forceRegions;
  NetElementData<List<PhysicsForceRegion>> m_itemForceRegions;
  NetElementHashMap<String, Json> m_scriptedAnimationParameters;
};

}// namespace Star
