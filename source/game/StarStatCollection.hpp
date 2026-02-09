#pragma once

#include "StarEither.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementContainers.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSyncGroup.hpp"
#include "StarStatSet.hpp"

import std;

namespace Star {

// Extension of StatSet that can easily be set up from config, and is network
// capable.
class StatCollection : public NetElementSyncGroup {
public:
  explicit StatCollection(Json const& config);

  auto statNames() const -> StringList;
  auto stat(String const& statName) const -> float;
  // Returns true if the stat is strictly greater than zero
  auto statPositive(String const& statName) const -> bool;

  auto resourceNames() const -> StringList;
  auto isResource(String const& resourceName) const -> bool;
  auto resource(String const& resourceName) const -> float;
  // Returns true if the resource is strictly greater than zero
  auto resourcePositive(String const& resourceName) const -> bool;

  void setResource(String const& resourceName, float value);
  void modifyResource(String const& resourceName, float amount);

  auto giveResource(String const& resourceName, float amount) -> float;

  auto consumeResource(String const& resourceName, float amount) -> bool;
  auto overConsumeResource(String const& resourceName, float amount) -> bool;

  auto resourceLocked(String const& resourceName) const -> bool;
  void setResourceLocked(String const& resourceName, bool locked);

  // Resetting a resource also clears any locked states
  void resetResource(String const& resourceName);
  void resetAllResources();

  auto resourceMax(String const& resourceName) const -> std::optional<float>;
  auto resourcePercentage(String const& resourceName) const -> std::optional<float>;
  auto setResourcePercentage(String const& resourceName, float resourcePercentage) -> float;
  auto modifyResourcePercentage(String const& resourceName, float resourcePercentage) -> float;

  auto addStatModifierGroup(List<StatModifier> modifiers = {}) -> StatModifierGroupId;
  void setStatModifierGroup(StatModifierGroupId modifierGroupId, List<StatModifier> modifiers);
  void removeStatModifierGroup(StatModifierGroupId modifierGroupId);
  void clearStatModifiers();

  void tickMaster(float dt);
  void tickSlave(float dt);

private:
  void netElementsNeedLoad(bool full) override;
  void netElementsNeedStore() override;

  StatSet m_stats;
  // Left value is a raw value, right value is a percentage.
  StringMap<Either<float, float>> m_defaultResourceValues;

  NetElementMap<StatModifierGroupId, List<StatModifier>> m_statModifiersNetState;
  StableStringMap<NetElementFloat> m_resourceValuesNetStates;
  StableStringMap<NetElementBool> m_resourceLockedNetStates;
};

}// namespace Star
