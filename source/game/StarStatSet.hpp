#pragma once

#include "StarStatusTypes.hpp"

import std;

namespace Star {

// Manages a collection of Stats and Resources.
//
// Stats are named floating point values of any base value, with an arbitrary
// number of "stat modifiers" attached to them.  Stat modifiers can be added
// and removed in groups, and they can either raise or lower stats by a
// constant value or a percentage of the stat value without any other
// percentage modifications applied.  The effective stat value is always the
// value with all mods applied.  If a modifier is created for a stat that does
// not exist, there will be an effective stat value for the modified stat, but
// NO base stat.  If the modifier is a base percentage modifier, it will have
// no effect because it is assumed that base stats that do not exist are zero.
//
// Resources are also named floating point values, but are in a different
// namespaced and are intended to be used as values that change regularly.
// They are always >= 0.0f, and optionally have a maximum value based on a
// given value or stat.  In addition to a max value, they can also have a
// "delta" value or stat, which automatically adds or removes that delta to the
// resource every second.
//
// If a resource has a maximum value, then rather than trying to keep the
// *value* of the resource constant, this class will instead attempt to keep
// the *percentage* of the resource constant across stat changes.  For example,
// if "health" is a stat with a max of 100, and the current health value is 50,
// and the max health stat is changed to 200 through any means, the health
// value will automatically update to 100.
class StatSet {
public:
  void addStat(String statName, float baseValue = 0.0f);
  void removeStat(String const& statName);

  // Only lists base stats added with addStat, not stats that come only from
  // modifiers
  [[nodiscard]] auto baseStatNames() const -> StringList;
  [[nodiscard]] auto isBaseStat(String const& statName) const -> bool;

  // Throws when the stat is not a base stat that is added via addStat.
  [[nodiscard]] auto statBaseValue(String const& statName) const -> float;
  void setStatBaseValue(String const& statName, float value);

  [[nodiscard]] auto statModifierGroupIds() const -> List<StatModifierGroupId>;
  [[nodiscard]] auto statModifierGroup(StatModifierGroupId modifierGroupId) const -> List<StatModifier>;

  auto addStatModifierGroup(List<StatModifier> modifiers = {}) -> StatModifierGroupId;
  void addStatModifierGroup(StatModifierGroupId groupId, List<StatModifier> modifiers);
  auto setStatModifierGroup(StatModifierGroupId groupId, List<StatModifier> modifiers) -> bool;
  auto removeStatModifierGroup(StatModifierGroupId modifierGroupId) -> bool;
  void clearStatModifiers();

  [[nodiscard]] auto allStatModifierGroups() const -> StatModifierGroupMap const&;
  void setAllStatModifierGroups(StatModifierGroupMap map);

  [[nodiscard]] auto effectiveStatNames() const -> StringList;

  // Does this stat exist either from the base stats or the modifiers
  [[nodiscard]] auto isEffectiveStat(String const& statName) const -> bool;

  // Will never throw, returns either the base stat value, or the modified
  // stat value if a modifier is applied, or 0.0.  This is to support stats that
  // may come only from modifiers and have no base value.
  [[nodiscard]] auto statEffectiveValue(String const& statName) const -> float;

  void addResource(String resourceName, MVariant<String, float> max = {}, MVariant<String, float> delta = {});
  void removeResource(String const& resourceName);

  [[nodiscard]] auto resourceMax(String const& resourceName) const -> MVariant<String, float>;
  [[nodiscard]] auto resourceDelta(String const& resourceName) const -> MVariant<String, float>;

  [[nodiscard]] auto resourceNames() const -> StringList;
  [[nodiscard]] auto isResource(String const& resourceName) const -> bool;

  // Will never throw, returns either the resource value, or 0.0 for a missing
  // resource
  [[nodiscard]] auto resourceValue(String const& resourceName) const -> float;

  auto setResourceValue(String const& resourceName, float value) -> float;
  auto modifyResourceValue(String const& resourceName, float amount) -> float;

  // Similar to consumeResource, will add the given amount to a resource if
  // it exists. Returns the amount by which the resource was actually increased.
  auto giveResourceValue(String const& resourceName, float amount) -> float;

  // If a resource exists and has more than the given amount available, and the
  // resource is not locked, then subtracts this amount from the resource and
  // returns true.  Otherwise, does nothing and returns false.  Will only throw
  // if 'amount' is less than zero, will simply return false on missing
  // resource.
  auto consumeResourceValue(String const& resourceName, float amount) -> bool;

  // Like consumeResource, but always succeeds if the resource is unlocked and
  // the amount is nonzero.  If the amount is greater than the available
  // resource, then the resource will be consumed to zero.
  auto overConsumeResourceValue(String const& resourceName, float amount) -> bool;

  // A locked resource cannot be consumed in any way.
  [[nodiscard]] auto resourceLocked(String const& resourceName) const -> bool;
  void setResourceLocked(String const& resourceName, bool locked);

  // If a resource has a maximum value, this will return it.
  [[nodiscard]] auto resourceMaxValue(String const& resourceName) const -> std::optional<float>;
  // Returns the resource percentage if the resource has a max value.
  [[nodiscard]] auto resourcePercentage(String const& resourceName) const -> std::optional<float>;
  // If the resource has a max value, then modifies the value percentage,
  // otherwise this is nonsense so throws.
  auto setResourcePercentage(String const& resourceName, float resourcePercentage) -> float;
  auto modifyResourcePercentage(String const& resourceName, float resourcePercentage) -> float;

  void update(float dt);

private:
  struct EffectiveStat {
    float baseValue;
    // Value with just the base percent modifiers applied and the value
    // modifiers
    float baseModifiedValue;
    // Final modified value that includes the effective modifiers.
    float effectiveModifiedValue;
  };

  struct Resource {
    MVariant<String, float> max;
    MVariant<String, float> delta;
    bool locked;
    float value;
    std::optional<float> maxValue;

    // Sets value and clamps between [0.0, maxStatValue] or just >= 0.0 if
    // maxStatValue is not given.
    auto setValue(float v) -> float;
  };

  [[nodiscard]] auto getResource(String const& resourceName) const -> Resource const&;
  auto getResource(String const& resourceName) -> Resource&;

  auto consumeResourceValue(String const& resourceName, float amount, bool allowOverConsume) -> bool;

  StringMap<float> m_baseStats;
  StringMap<EffectiveStat> m_effectiveStats;
  StatModifierGroupMap m_statModifierGroups;
  StringMap<Resource> m_resources;
};

}// namespace Star
