#include "StarFireableItem.hpp"
#include "StarCasting.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarFireableItemLuaBindings.hpp"
#include "StarItem.hpp"
#include "StarItemLuaBindings.hpp"
#include "StarJsonExtra.hpp"

import std;

namespace Star {

FireableItem::FireableItem()
    : m_fireTimer(0),
      m_cooldownTime(10),
      m_windupTime(0),
      m_fireWhenReady(false),
      m_startWhenReady(false),
      m_cooldown(false),
      m_alreadyInit(false),
      m_requireEdgeTrigger(false),
      m_attemptedFire(false),
      m_fireOnRelease(false),
      m_timeFiring(0.0f),
      m_startTimingFire(false),
      m_inUse(false),
      m_walkWhileFiring(false),
      m_stopWhileFiring(false),
      m_mode(FireMode::None) {}

FireableItem::FireableItem(Json const& params) : FireableItem() {
  setParams(params);
  m_fireableParams = params;
}

FireableItem::FireableItem(FireableItem const& rhs) : ToolUserItem(rhs), StatusEffectItem(rhs) {
  m_fireTimer = rhs.m_fireTimer;
  m_cooldownTime = rhs.m_cooldownTime;
  m_windupTime = rhs.m_windupTime;
  m_fireWhenReady = rhs.m_fireWhenReady;
  m_startWhenReady = rhs.m_startWhenReady;
  m_cooldown = rhs.m_cooldown;
  m_alreadyInit = rhs.m_alreadyInit;
  m_requireEdgeTrigger = rhs.m_requireEdgeTrigger;
  m_attemptedFire = rhs.m_attemptedFire;
  m_fireOnRelease = rhs.m_fireOnRelease;
  m_timeFiring = rhs.m_timeFiring;
  m_startTimingFire = rhs.m_startTimingFire;
  m_inUse = rhs.m_inUse;
  m_walkWhileFiring = rhs.m_walkWhileFiring;
  m_stopWhileFiring = rhs.m_stopWhileFiring;
  m_fireableParams = rhs.m_fireableParams;
  m_handPosition = rhs.m_handPosition;
  m_mode = rhs.m_mode;
}

void FireableItem::init(ToolUserEntity* owner, ToolHand hand) {
  ToolUserItem::init(owner, hand);

  m_fireWhenReady = false;
  m_startWhenReady = false;

  auto scripts = m_fireableParams.opt("scripts").transform(jsonToStringList);
  if (entityMode() == EntityMode::Master && scripts) {
    if (!m_scriptComponent) {
      m_scriptComponent.emplace();
      m_scriptComponent->setScripts(*scripts);
    }
    m_scriptComponent->addCallbacks(
      "config", LuaBindings::makeConfigCallbacks([capture0 = as<Item>(this)](auto&& PH1, auto&& PH2) -> auto { return capture0->instanceValue(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }));
    m_scriptComponent->addCallbacks("fireableItem", LuaBindings::makeFireableItemCallbacks(this));
    m_scriptComponent->addCallbacks("item", LuaBindings::makeItemCallbacks(as<Item>(this)));
    m_scriptComponent->init(world());
  }
}

void FireableItem::uninit() {
  if (m_scriptComponent) {
    m_scriptComponent->uninit();
    m_scriptComponent->removeCallbacks("config");
    m_scriptComponent->removeCallbacks("fireableItem");
    m_scriptComponent->removeCallbacks("item");
  }

  ToolUserItem::uninit();
}

void FireableItem::fire(FireMode mode, bool, bool edgeTriggered) {
  m_attemptedFire = true;
  if (ready()) {
    m_inUse = true;
    m_startTimingFire = true;
    m_mode = mode;
    if (!m_requireEdgeTrigger || edgeTriggered) {
      setFireTimer(windupTime() + cooldownTime());
      if (!m_fireOnRelease) {
        m_fireWhenReady = true;
        m_startWhenReady = true;
      }
    }
  }

  if (m_scriptComponent)
    m_scriptComponent->invoke("attemptedFire");
}

void FireableItem::endFire(FireMode mode, bool) {
  if (m_scriptComponent)
    m_scriptComponent->invoke("endFire");

  m_attemptedFire = false;
  if (m_fireOnRelease && m_timeFiring) {
    m_mode = mode;
    triggerCooldown();
    fireTriggered();
  }
}

auto FireableItem::fireMode() const -> FireMode {
  return m_mode;
}

auto FireableItem::cooldownTime() const -> float {
  return m_cooldownTime;
}

void FireableItem::setCooldownTime(float cooldownTime) {
  m_cooldownTime = cooldownTime;
}

auto FireableItem::fireTimer() const -> float {
  return m_fireTimer;
}

void FireableItem::setFireTimer(float fireTimer) {
  m_fireTimer = fireTimer;
}

auto FireableItem::ready() const -> bool {
  return fireTimer() <= 0;
}

auto FireableItem::firing() const -> bool {
  return m_timeFiring > 0;
}

auto FireableItem::inUse() const -> bool {
  return m_inUse;
}

auto FireableItem::walkWhileFiring() const -> bool {
  return m_walkWhileFiring;
}

auto FireableItem::stopWhileFiring() const -> bool {
  return m_stopWhileFiring;
}

auto FireableItem::windup() const -> bool {
  if (ready())
    return false;

  if (m_scriptComponent)
    m_scriptComponent->invoke("triggerWindup");

  return fireTimer() > cooldownTime();
}

void FireableItem::update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const&) {
  if (m_scriptComponent)
    m_scriptComponent->invoke("update", dt, FireModeNames.getRight(fireMode), shifting);

  if (m_attemptedFire) {
    if (m_startTimingFire) {
      m_timeFiring += dt;
      if (m_scriptComponent)
        m_scriptComponent->invoke("continueFire", dt);
    }
  } else {
    m_timeFiring = 0.0f;
    m_startTimingFire = false;
  }
  m_attemptedFire = false;

  if (entityMode() == EntityMode::Master) {
    if (fireTimer() > 0.0f) {
      setFireTimer(fireTimer() - dt);
      if (fireTimer() < 0.0f) {
        setFireTimer(0.0f);
        m_inUse = false;
      }
    }
    if (fireTimer() <= 0) {
      m_cooldown = false;
    }
    if (m_startWhenReady) {
      m_startWhenReady = false;
      startTriggered();
    }
    if (m_fireWhenReady) {
      if (fireTimer() <= cooldownTime()) {
        m_fireWhenReady = false;
        fireTriggered();
      }
    }
  }
}

void FireableItem::triggerCooldown() {
  setFireTimer(cooldownTime());
  m_cooldown = true;
  if (m_scriptComponent)
    m_scriptComponent->invoke("triggerCooldown");
}

auto FireableItem::coolingDown() const -> bool {
  return m_cooldown;
}

void FireableItem::setCoolingDown(bool coolingdown) {
  m_cooldown = coolingdown;
}

auto FireableItem::timeFiring() const -> float {
  return m_timeFiring;
}

void FireableItem::setTimeFiring(float timeFiring) {
  m_timeFiring = timeFiring;
}

auto FireableItem::handPosition() const -> Vec2F {
  return m_handPosition;
}

auto FireableItem::firePosition() const -> Vec2F {
  return {};
}

auto FireableItem::fireableParam(String const& key) const -> Json {
  return m_fireableParams.get(key);
}

auto FireableItem::fireableParam(String const& key, Json const& defaultVal) const -> Json {
  return m_fireableParams.get(key, defaultVal);
}

auto FireableItem::validAimPos(Vec2F const&) -> bool {
  return true;
}

void FireableItem::setParams(Json const& params) {
  if (!m_alreadyInit) {
    // cannot use setWindupTime or setCooldownTime here, because object is not fully constructed
    m_windupTime = params.getFloat("windupTime", 0.0f);
    m_cooldownTime = params.getFloat("cooldown", params.getFloat("fireTime", 0.15f) - m_windupTime);
    if (params.contains("handPosition")) {
      m_handPosition = jsonToVec2F(params.get("handPosition"));
    }
    m_requireEdgeTrigger = params.getBool("edgeTrigger", false);
    m_fireOnRelease = params.getBool("fireOnRelease", false);
    m_walkWhileFiring = params.getBool("walkWhileFiring", false);
    m_stopWhileFiring = params.getBool("stopWhileFiring", false);
    m_alreadyInit = true;
  }
}

void FireableItem::setFireableParam(String const& key, Json const& value) {
  m_fireableParams = m_fireableParams.set(key, value);
}

void FireableItem::startTriggered() {
  if (m_scriptComponent)
    m_scriptComponent->invoke("startTriggered");
}

void FireableItem::fireTriggered() {
  if (m_scriptComponent)
    m_scriptComponent->invoke("fireTriggered");
}

auto FireableItem::ownerFirePosition() const -> Vec2F {
  if (!initialized())
    throw StarException("FireableItem uninitialized in ownerFirePosition");

  return owner()->handPosition(hand(), (this->firePosition() - handPosition()) / TilePixels);
}

auto FireableItem::windupTime() const -> float {
  return m_windupTime;
}

void FireableItem::setWindupTime(float time) {
  m_windupTime = time;
}

auto FireableItem::statusEffects() const -> List<PersistentStatusEffect> {
  return {};
}

}// namespace Star
