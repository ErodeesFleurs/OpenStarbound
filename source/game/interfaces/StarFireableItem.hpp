#pragma once

#include "StarToolUserItem.hpp"
#include "StarStatusEffectItem.hpp"
#include "StarLuaComponents.hpp"

namespace Star {

class FireableItem;
using FireableItemPtr = SharedPtr<FireableItem>;

class FireableItem : public virtual ToolUserItem, public virtual StatusEffectItem {
public:
  FireableItem() = default;
  FireableItem(Json const& params);
  virtual ~FireableItem() = default;

  FireableItem(FireableItem const& fireableItem);

  virtual void fire(FireMode mode, bool shifting, bool edgeTriggered);
  virtual void endFire(FireMode mode, bool shifting);
  [[nodiscard]] virtual FireMode fireMode() const;
  [[nodiscard]] virtual float fireTimer() const;
  virtual void setFireTimer(float fireTimer);
  [[nodiscard]] virtual float cooldownTime() const;
  virtual void setCooldownTime(float cooldownTime);
  [[nodiscard]] virtual float windupTime() const;
  virtual void setWindupTime(float time);
  [[nodiscard]] virtual bool ready() const;
  [[nodiscard]] virtual bool firing() const;
  [[nodiscard]] virtual bool inUse() const;
  [[nodiscard]] virtual bool walkWhileFiring() const;
  [[nodiscard]] virtual bool stopWhileFiring() const;
  [[nodiscard]] virtual bool windup() const;
  virtual void triggerCooldown();
  [[nodiscard]] virtual bool coolingDown() const;
  virtual void setCoolingDown(bool coolingdown);
  [[nodiscard]] virtual float timeFiring() const;
  virtual void setTimeFiring(float timeFiring);
  [[nodiscard]] virtual Vec2F firePosition() const;
  [[nodiscard]] virtual Vec2F handPosition() const;

  void init(ToolUserEntity& owner, ToolHand hand) override;
  void uninit() override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  [[nodiscard]] List<PersistentStatusEffect> statusEffects() const override;

  [[nodiscard]] virtual bool validAimPos(Vec2F const& aimPos);

  [[nodiscard]] Json fireableParam(String const& key) const;
  [[nodiscard]] Json fireableParam(String const& key, Json const& defaultVal) const;

protected:
  void setParams(Json const& params);
  void setFireableParam(String const& key, Json const& value);
  virtual void startTriggered();
  virtual void fireTriggered();

  // firePosition translated by the hand in the owner's space
  [[nodiscard]] Vec2F ownerFirePosition() const;

  float m_fireTimer = 0;
  float m_cooldownTime = 10;
  float m_windupTime = 0;
  bool m_fireWhenReady = false;
  bool m_startWhenReady = false;
  bool m_cooldown = false;
  bool m_alreadyInit = false;
  bool m_requireEdgeTrigger = false;

  bool m_attemptedFire = false;
  bool m_fireOnRelease = false;
  float m_timeFiring = 0.0f;
  bool m_startTimingFire = false;
  bool m_inUse = false;
  bool m_walkWhileFiring = false;
  bool m_stopWhileFiring = false;

  mutable Maybe<LuaWorldComponent<LuaBaseComponent>> m_scriptComponent;

  Json m_fireableParams;

  Vec2F m_handPosition;

  FireMode m_mode = FireMode::None;
};

}
