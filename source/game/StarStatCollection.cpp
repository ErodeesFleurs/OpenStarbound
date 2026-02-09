#include "StarStatCollection.hpp"

import std;

namespace Star {

StatCollection::StatCollection(Json const& config) {
  for (auto const& stat : config.getObject("stats", {}))
    m_stats.addStat(stat.first, stat.second.getFloat("baseValue", 0.0));

  for (auto const& resource : config.getObject("resources", {})) {
    auto statOrValue = [&resource](String const& statName, String const& valueName, MVariant<String, float> def = {}) -> MVariant<String, float> {
      if (auto maxStat = resource.second.optString(statName))
        return *maxStat;
      else if (auto maxValue = resource.second.optFloat(valueName))
        return *maxValue;
      else
        return def;
    };

    MVariant<String, float> resourceMax = statOrValue("maxStat", "maxValue");
    MVariant<String, float> resourceDelta = statOrValue("deltaStat", "deltaValue");
    m_stats.addResource(resource.first, resourceMax, resourceDelta);

    if (auto initialValue = resource.second.optFloat("initialValue")) {
      m_stats.setResourceValue(resource.first, *initialValue);
      m_defaultResourceValues[resource.first] = makeLeft(*initialValue);
    } else if (auto percentage = resource.second.optFloat("initialPercentage")) {
      m_stats.setResourcePercentage(resource.first, *percentage);
      m_defaultResourceValues[resource.first] = makeRight(*percentage);
    } else {
      if (m_stats.resourceMax(resource.first)) {
        m_stats.setResourcePercentage(resource.first, 1.0f);
        m_defaultResourceValues[resource.first] = makeRight(1.0f);
      } else {
        m_stats.setResourceValue(resource.first, 0.0f);
        m_defaultResourceValues[resource.first] = makeLeft(0.0f);
      }
    }
  }

  addNetElement(&m_statModifiersNetState);

  // Sort resource names alphabetically to ensure the same order on master and
  // slaves.
  for (auto const& resource : m_stats.resourceNames().sorted()) {
    auto& resourceNetState = m_resourceValuesNetStates[resource];
    addNetElement(&resourceNetState);
    addNetElement(&m_resourceLockedNetStates[resource]);
  }
}

auto StatCollection::statNames() const -> StringList {
  return m_stats.effectiveStatNames();
}

auto StatCollection::stat(String const& statName) const -> float {
  return m_stats.statEffectiveValue(statName);
}

auto StatCollection::statPositive(String const& statName) const -> bool {
  return stat(statName) > 0.0f;
}

auto StatCollection::resourceNames() const -> StringList {
  return m_stats.resourceNames();
}

auto StatCollection::isResource(String const& resourceName) const -> bool {
  return m_stats.isResource(resourceName);
}

auto StatCollection::resource(String const& resourceName) const -> float {
  return m_stats.resourceValue(resourceName);
}

auto StatCollection::resourcePositive(String const& resourceName) const -> bool {
  return resource(resourceName) > 0.0f;
}

void StatCollection::setResource(String const& resourceName, float value) {
  m_stats.setResourceValue(resourceName, value);
}

void StatCollection::modifyResource(String const& resourceName, float amount) {
  m_stats.modifyResourceValue(resourceName, amount);
}

auto StatCollection::giveResource(String const& resourceName, float amount) -> float {
  return m_stats.giveResourceValue(resourceName, amount);
}

auto StatCollection::consumeResource(String const& resourceName, float amount) -> bool {
  return m_stats.consumeResourceValue(resourceName, amount);
}

auto StatCollection::overConsumeResource(String const& resourceName, float amount) -> bool {
  return m_stats.overConsumeResourceValue(resourceName, amount);
}

auto StatCollection::resourceLocked(String const& resourceName) const -> bool {
  return m_stats.resourceLocked(resourceName);
}

void StatCollection::setResourceLocked(String const& resourceName, bool locked) {
  m_stats.setResourceLocked(resourceName, locked);
}

void StatCollection::resetResource(String const& resourceName) {
  m_stats.setResourceLocked(resourceName, false);
  auto def = m_defaultResourceValues.get(resourceName);
  if (def.isLeft())
    m_stats.setResourceValue(resourceName, def.left());
  else
    m_stats.setResourcePercentage(resourceName, def.right());
}

void StatCollection::resetAllResources() {
  for (auto const& resourceName : m_stats.resourceNames())
    resetResource(resourceName);
}

auto StatCollection::resourceMax(String const& resourceName) const -> std::optional<float> {
  return m_stats.resourceMaxValue(resourceName);
}

auto StatCollection::resourcePercentage(String const& resourceName) const -> std::optional<float> {
  return m_stats.resourcePercentage(resourceName);
}

auto StatCollection::setResourcePercentage(String const& resourceName, float resourcePercentage) -> float {
  return m_stats.setResourcePercentage(resourceName, resourcePercentage);
}

auto StatCollection::modifyResourcePercentage(String const& resourceName, float resourcePercentage) -> float {
  return m_stats.modifyResourcePercentage(resourceName, resourcePercentage);
}

auto StatCollection::addStatModifierGroup(List<StatModifier> modifiers) -> StatModifierGroupId {
  return m_stats.addStatModifierGroup(modifiers);
}

void StatCollection::setStatModifierGroup(StatModifierGroupId modifierGroupId, List<StatModifier> modifiers) {
  m_stats.setStatModifierGroup(modifierGroupId, modifiers);
}

void StatCollection::removeStatModifierGroup(StatModifierGroupId modifierGroupId) {
  m_stats.removeStatModifierGroup(modifierGroupId);
}

void StatCollection::clearStatModifiers() {
  m_stats.clearStatModifiers();
}

void StatCollection::tickMaster(float dt) {
  m_stats.update(dt);
}

void StatCollection::tickSlave(float) {
  m_stats.update(0.0f);
}

void StatCollection::netElementsNeedLoad(bool) {
  if (m_statModifiersNetState.pullUpdated()) {
    StatModifierGroupMap allModifiers;
    for (auto const& p : m_statModifiersNetState)
      allModifiers.add(p.first, p.second);
    m_stats.setAllStatModifierGroups(std::move(allModifiers));
  }

  for (auto const& pair : m_resourceValuesNetStates)
    m_stats.setResourceValue(pair.first, pair.second.get());

  for (auto& pair : m_resourceLockedNetStates)
    m_stats.setResourceLocked(pair.first, pair.second.get());
}

void StatCollection::netElementsNeedStore() {
  m_statModifiersNetState.setContents(m_stats.allStatModifierGroups());

  for (auto& pair : m_resourceValuesNetStates)
    pair.second.set(m_stats.resourceValue(pair.first));

  for (auto& pair : m_resourceLockedNetStates)
    pair.second.set(m_stats.resourceLocked(pair.first));
}

}// namespace Star
