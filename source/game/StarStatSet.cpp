#include "StarStatSet.hpp"

#include "StarMathCommon.hpp"

import std;

namespace Star {

void StatSet::addStat(String statName, float baseValue) {
  if (!m_baseStats.insert(std::move(statName), baseValue).second)
    throw StatusException::format("Added duplicate stat named '{}' in StatSet", statName);
  update(0.0f);
}

void StatSet::removeStat(String const& statName) {
  if (!m_baseStats.remove(statName))
    throw StatusException::format("No such base stat '{}' in StatSet", statName);
  update(0.0f);
}

auto StatSet::baseStatNames() const -> StringList {
  return m_baseStats.keys();
}

auto StatSet::isBaseStat(String const& statName) const -> bool {
  return m_baseStats.contains(statName);
}

auto StatSet::statBaseValue(String const& statName) const -> float {
  if (auto s = m_baseStats.ptr(statName))
    return *s;
  throw StatusException::format("No such base stat '{}' in StatSet", statName);
}

void StatSet::setStatBaseValue(String const& statName, float value) {
  if (auto s = m_baseStats.ptr(statName)) {
    if (*s != value) {
      *s = value;
      update(0.0f);
    }
  } else {
    throw StatusException::format("No such base stat '{}' in StatSet", statName);
  }
}

auto StatSet::addStatModifierGroup(List<StatModifier> modifiers) -> StatModifierGroupId {
  bool empty = modifiers.empty();
  auto id = m_statModifierGroups.add(std::move(modifiers));
  if (!empty)
    update(0.0f);
  return id;
}

auto StatSet::statModifierGroupIds() const -> List<StatModifierGroupId> {
  return m_statModifierGroups.keys();
}

auto StatSet::statModifierGroup(StatModifierGroupId modifierGroupId) const -> List<StatModifier> {
  return m_statModifierGroups.get(modifierGroupId);
}

void StatSet::addStatModifierGroup(StatModifierGroupId groupId, List<StatModifier> modifiers) {
  bool empty = modifiers.empty();
  m_statModifierGroups.add(groupId, std::move(modifiers));
  if (!empty)
    update(0.0f);
}

auto StatSet::setStatModifierGroup(StatModifierGroupId groupId, List<StatModifier> modifiers) -> bool {
  auto& list = m_statModifierGroups.get(groupId);
  if (list != modifiers) {
    list = std::move(modifiers);
    update(0.0f);
    return true;
  }

  return false;
}

auto StatSet::removeStatModifierGroup(StatModifierGroupId modifierSetId) -> bool {
  if (m_statModifierGroups.remove(modifierSetId)) {
    update(0.0f);
    return true;
  }
  return false;
}

void StatSet::clearStatModifiers() {
  if (!m_statModifierGroups.empty()) {
    m_statModifierGroups.clear();
    update(0.0f);
  }
}

auto StatSet::allStatModifierGroups() const -> StatModifierGroupMap const& {
  return m_statModifierGroups;
}

void StatSet::setAllStatModifierGroups(StatModifierGroupMap map) {
  if (m_statModifierGroups != map) {
    m_statModifierGroups = std::move(map);
    update(0.0f);
  }
}

auto StatSet::effectiveStatNames() const -> StringList {
  return m_effectiveStats.keys();
}

auto StatSet::isEffectiveStat(String const& statName) const -> bool {
  return m_effectiveStats.contains(statName);
}

auto StatSet::statEffectiveValue(String const& statName) const -> float {
  // All stat values will be added to m_effectiveStats regardless of whether a
  // modifier is applied for it.
  if (auto modified = m_effectiveStats.ptr(statName))
    return modified->effectiveModifiedValue;
  else
    return 0.0f;
}

void StatSet::addResource(String resourceName, MVariant<String, float> max, MVariant<String, float> delta) {
  auto pair = m_resources.insert({std::move(resourceName), Resource{.max = std::move(max), .delta = std::move(delta), .locked = false, .value = 0.0f, .maxValue = {}}});
  if (!pair.second)
    throw StatusException::format("Added duplicate resource named '{}' in StatSet", resourceName);
  update(0.0f);
}

void StatSet::removeResource(String const& resourceName) {
  if (!m_resources.remove(resourceName))
    throw StatusException::format("No such resource named '{}' in StatSet", resourceName);
}

auto StatSet::resourceNames() const -> StringList {
  return m_resources.keys();
}

auto StatSet::resourceMax(String const& resourceName) const -> MVariant<String, float> {
  return getResource(resourceName).max;
}

auto StatSet::resourceDelta(String const& resourceName) const -> MVariant<String, float> {
  return getResource(resourceName).delta;
}

auto StatSet::isResource(String const& resourceName) const -> bool {
  return m_resources.contains(resourceName);
}

auto StatSet::resourceValue(String const& resourceName) const -> float {
  if (auto r = m_resources.ptr(resourceName))
    return r->value;
  return 0.0f;
}

auto StatSet::setResourceValue(String const& resourceName, float value) -> float {
  return getResource(resourceName).setValue(value);
}

auto StatSet::modifyResourceValue(String const& resourceName, float amount) -> float {
  auto& resource = getResource(resourceName);
  return resource.setValue(resource.value + amount);
}

auto StatSet::giveResourceValue(String const& resourceName, float amount) -> float {
  if (auto r = m_resources.ptr(resourceName)) {
    float previousValue = r->value;
    r->setValue(r->value + amount);
    return r->value - previousValue;
  }
  return 0;
}

auto StatSet::consumeResourceValue(String const& resourceName, float amount) -> bool {
  return consumeResourceValue(resourceName, amount, false);
}

auto StatSet::overConsumeResourceValue(String const& resourceName, float amount) -> bool {
  return consumeResourceValue(resourceName, amount, true);
}

auto StatSet::resourceLocked(String const& resourceName) const -> bool {
  return getResource(resourceName).locked;
}

void StatSet::setResourceLocked(String const& resourceName, bool locked) {
  getResource(resourceName).locked = locked;
}

auto StatSet::resourceMaxValue(String const& resourceName) const -> std::optional<float> {
  return getResource(resourceName).maxValue;
}

auto StatSet::resourcePercentage(String const& resourceName) const -> std::optional<float> {
  auto const& resource = getResource(resourceName);
  if (!resource.maxValue)
    return {};
  return resource.value / *resource.maxValue;
}

auto StatSet::setResourcePercentage(String const& resourceName, float resourcePercentage) -> float {
  auto& resource = getResource(resourceName);
  if (!resource.maxValue)
    throw StatusException::format("setResourcePersentage called on resource '{}' which has no maximum", resourceName);
  return resource.setValue(resourcePercentage * *resource.maxValue);
}

auto StatSet::modifyResourcePercentage(String const& resourceName, float resourcePercentage) -> float {
  auto& resource = getResource(resourceName);
  if (!resource.maxValue)
    throw StatusException::format(
      "modifyResourcePercentage called on resource '{}' which has no maximum", resourceName);
  return resource.setValue(resource.value + resourcePercentage * *resource.maxValue);
}

void StatSet::update(float dt) {
  // We use two intermediate values for calculating the effective stat value.
  // The baseModifiedValue represents the application of the base percentage
  // modifiers and the value modifiers, which only depend on the baseValue.
  // The effectiveModifiedValue is the application of all effective percentage
  // modifiers successively on the baseModifiedValue, causing them to stack with
  // each other in addition to base multipliers and value modifiers

  // First, clear the modified values to get rid of temporary stats applied
  // from modifiers that may no longer be there
  m_effectiveStats.clear();

  // Then we do all the StatValueModifiers and StatBaseMultipliers and
  // compute the baseModifiedValue

  for (auto& p : m_baseStats) {
    auto& stat = m_effectiveStats[p.first];
    stat.baseValue = p.second;
    stat.baseModifiedValue = stat.baseValue;
  }

  for (auto const& p : m_statModifierGroups) {
    for (auto const& modifier : p.second) {
      if (auto baseMultiplier = modifier.ptr<StatBaseMultiplier>()) {
        auto& stat = m_effectiveStats[baseMultiplier->statName];
        stat.baseModifiedValue += (baseMultiplier->baseMultiplier - 1.0f) * stat.baseValue;
      } else if (auto valueModifier = modifier.ptr<StatValueModifier>()) {
        auto& stat = m_effectiveStats[valueModifier->statName];
        stat.baseModifiedValue += valueModifier->value;
      }
    }
  }

  // Then we do all the StatEffectiveMultipliers and compute the
  // final effectiveModifiedValue

  for (auto& p : m_effectiveStats)
    p.second.effectiveModifiedValue = p.second.baseModifiedValue;

  for (auto const& p : m_statModifierGroups) {
    for (auto const& modifier : p.second) {
      if (auto effectiveMultiplier = modifier.ptr<StatEffectiveMultiplier>()) {
        auto& stat = m_effectiveStats[effectiveMultiplier->statName];
        stat.effectiveModifiedValue *= effectiveMultiplier->effectiveMultiplier;
      }
    }
  }

  // Then update all the resources due to charging and percentage tracking,
  // after updating the stats.

  for (auto& p : m_resources) {
    std::optional<float> newMaxValue;
    if (p.second.max.is<String>())
      newMaxValue = statEffectiveValue(p.second.max.get<String>());
    else if (p.second.max.is<float>())
      newMaxValue = p.second.max.get<float>();

    // If the resource has a maximum value, rather than keeping the absolute
    // value of the resource the same between updates, the resource value
    // should instead track the percentage.
    if (p.second.maxValue && newMaxValue && *p.second.maxValue > 0.0f)
      p.second.value *= *newMaxValue / *p.second.maxValue;

    p.second.maxValue = newMaxValue;
    if (p.second.maxValue)
      p.second.value = clamp(p.second.value, 0.0f, *p.second.maxValue);

    if (dt != 0.0f) {
      float delta = 0.0f;
      if (p.second.delta.is<String>())
        delta = statEffectiveValue(p.second.delta.get<String>());
      else if (p.second.delta.is<float>())
        delta = p.second.delta.get<float>();
      p.second.setValue(p.second.value + delta * dt);
    }
  }
}

auto StatSet::Resource::setValue(float v) -> float {
  if (maxValue)
    value = std::clamp(v, 0.0f, *maxValue);
  else
    value = std::max(v, 0.0f);
  return value;
}

auto StatSet::getResource(String const& resourceName) const -> StatSet::Resource const& {
  if (auto r = m_resources.ptr(resourceName))
    return *r;
  throw StatusException::format("No such resource '{}' in StatSet", resourceName);
}

auto StatSet::getResource(String const& resourceName) -> StatSet::Resource& {
  if (auto r = m_resources.ptr(resourceName))
    return *r;
  throw StatusException::format("No such resource '{}' in StatSet", resourceName);
}

auto StatSet::consumeResourceValue(String const& resourceName, float amount, bool allowOverConsume) -> bool {
  if (amount < 0.0f)
    throw StatusException::format("StatSet, consumeResource called with negative amount '{}' {}", resourceName, amount);

  if (auto r = m_resources.ptr(resourceName)) {
    if (r->locked)
      return false;

    if (r->value >= amount) {
      r->setValue(r->value - amount);
      return true;
    } else if (r->value > 0.0f && allowOverConsume) {
      r->setValue(0.0f);
      return true;
    }
  }
  return false;
}

}// namespace Star
