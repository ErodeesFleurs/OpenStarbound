#pragma once

#include "StarLuaComponents.hpp"
#include "StarStatusEffectItem.hpp"
#include "StarToolUserItem.hpp"

import std;

namespace Star {

class FireableItem : public virtual ToolUserItem, public virtual StatusEffectItem {
public:
  FireableItem();
  FireableItem(Json const& params);
  ~FireableItem() override = default;

  FireableItem(FireableItem const& fireableItem);

  virtual void fire(FireMode mode, bool shifting, bool edgeTriggered);
  virtual void endFire(FireMode mode, bool shifting);
  virtual auto fireMode() const -> FireMode;
  virtual auto fireTimer() const -> float;
  virtual void setFireTimer(float fireTimer);
  virtual auto cooldownTime() const -> float;
  virtual void setCooldownTime(float cooldownTime);
  virtual auto windupTime() const -> float;
  virtual void setWindupTime(float time);
  virtual auto ready() const -> bool;
  virtual auto firing() const -> bool;
  virtual auto inUse() const -> bool;
  virtual auto walkWhileFiring() const -> bool;
  virtual auto stopWhileFiring() const -> bool;
  virtual auto windup() const -> bool;
  virtual void triggerCooldown();
  virtual auto coolingDown() const -> bool;
  virtual void setCoolingDown(bool coolingdown);
  virtual auto timeFiring() const -> float;
  virtual void setTimeFiring(float timeFiring);
  virtual auto firePosition() const -> Vec2F;
  virtual auto handPosition() const -> Vec2F;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void uninit() override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  auto statusEffects() const -> List<PersistentStatusEffect> override;

  virtual auto validAimPos(Vec2F const& aimPos) -> bool;

  auto fireableParam(String const& key) const -> Json;
  auto fireableParam(String const& key, Json const& defaultVal) const -> Json;

protected:
  void setParams(Json const& params);
  void setFireableParam(String const& key, Json const& value);
  virtual void startTriggered();
  virtual void fireTriggered();

  // firePosition translated by the hand in the owner's space
  auto ownerFirePosition() const -> Vec2F;

  float m_fireTimer;
  float m_cooldownTime;
  float m_windupTime;
  bool m_fireWhenReady;
  bool m_startWhenReady;
  bool m_cooldown;
  bool m_alreadyInit;
  bool m_requireEdgeTrigger;

  bool m_attemptedFire;
  bool m_fireOnRelease;
  float m_timeFiring;
  bool m_startTimingFire;
  bool m_inUse;
  bool m_walkWhileFiring;
  bool m_stopWhileFiring;

  mutable std::optional<LuaWorldComponent<LuaBaseComponent>> m_scriptComponent;

  Json m_fireableParams;

  Vec2F m_handPosition;

  FireMode m_mode;
};

}// namespace Star
