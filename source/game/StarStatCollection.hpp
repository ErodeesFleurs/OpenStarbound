#pragma once

#include "StarEither.hpp"
#include "StarNetElementSystem.hpp"
#include "StarStatSet.hpp"

namespace Star {

// Extension of StatSet that can easily be set up from config, and is network
// capable.
class StatCollection : public NetElementSyncGroup {
public:
  explicit StatCollection(Json const& config);

  [[nodiscard]] StringList statNames() const;
  [[nodiscard]] float stat(String const& statName) const;
  // Returns true if the stat is strictly greater than zero
  [[nodiscard]] bool statPositive(String const& statName) const;

  [[nodiscard]] StringList resourceNames() const;
  [[nodiscard]] bool isResource(String const& resourceName) const;
  [[nodiscard]] float resource(String const& resourceName) const;
  // Returns true if the resource is strictly greater than zero
  [[nodiscard]] bool resourcePositive(String const& resourceName) const;

  void setResource(String const& resourceName, float value);
  void modifyResource(String const& resourceName, float amount);

  [[nodiscard]] float giveResource(String const& resourceName, float amount);

  [[nodiscard]] bool consumeResource(String const& resourceName, float amount);
  [[nodiscard]] bool overConsumeResource(String const& resourceName, float amount);

  [[nodiscard]] bool resourceLocked(String const& resourceName) const;
  void setResourceLocked(String const& resourceName, bool locked);

  // Resetting a resource also clears any locked states
  void resetResource(String const& resourceName);
  void resetAllResources();

  [[nodiscard]] Maybe<float> resourceMax(String const& resourceName) const;
  [[nodiscard]] Maybe<float> resourcePercentage(String const& resourceName) const;
  [[nodiscard]] float setResourcePercentage(String const& resourceName, float resourcePercentage);
  [[nodiscard]] float modifyResourcePercentage(String const& resourceName, float resourcePercentage);

  [[nodiscard]] StatModifierGroupId addStatModifierGroup(List<StatModifier> modifiers = {});
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

}
