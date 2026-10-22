module;

import star.format;
import std;
import star.string;
import star.list;
import star.json;
import star.strong_typedef;
import star.data_stream;
import star.id_map;
import star.status_types;

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
  [[nodiscard]] std::expected<void, StarException> addStat(String statName, float baseValue = 0.0f);
  [[nodiscard]] std::expected<void, StarException> removeStat(String const& statName);

  // Only lists base stats added with addStat, not stats that come only from
  // modifiers
  StringList baseStatNames() const;
  bool isBaseStat(String const& statName) const;

  // Reports a missing base stat.
  [[nodiscard]] std::expected<float, StarException> statBaseValue(String const& statName) const;
  [[nodiscard]] std::expected<void, StarException> setStatBaseValue(String const& statName, float value);

  List<StatModifierGroupId> statModifierGroupIds() const;
  [[nodiscard]] std::expected<List<StatModifier>, StarException> statModifierGroup(StatModifierGroupId modifierGroupId) const;

  [[nodiscard]] std::expected<StatModifierGroupId, StarException> addStatModifierGroup(List<StatModifier> modifiers = {});
  [[nodiscard]] std::expected<void, StarException> addStatModifierGroup(StatModifierGroupId groupId, List<StatModifier> modifiers);
  [[nodiscard]] std::expected<bool, StarException> setStatModifierGroup(StatModifierGroupId groupId, List<StatModifier> modifiers);
  bool removeStatModifierGroup(StatModifierGroupId modifierGroupId);
  void clearStatModifiers();

  StatModifierGroupMap const& allStatModifierGroups() const;
  void setAllStatModifierGroups(StatModifierGroupMap map);

  StringList effectiveStatNames() const;

  // Does this stat exist either from the base stats or the modifiers
  bool isEffectiveStat(String const& statName) const;

  // Returns either the base stat value, the modified stat value, or zero.
  // This supports stats that come only from modifiers and have no base value.
  float statEffectiveValue(String const& statName) const;

  [[nodiscard]] std::expected<void, StarException> addResource(String resourceName, std::variant<std::monostate, String, float> max = {}, std::variant<std::monostate, String, float> delta = {});
  [[nodiscard]] std::expected<void, StarException> removeResource(String const& resourceName);

  [[nodiscard]] std::expected<std::variant<std::monostate, String, float>, StarException> resourceMax(String const& resourceName) const;
  [[nodiscard]] std::expected<std::variant<std::monostate, String, float>, StarException> resourceDelta(String const& resourceName) const;

  StringList resourceNames() const;
  bool isResource(String const& resourceName) const;

  // Returns either the resource value, or zero for a missing resource.
  float resourceValue(String const& resourceName) const;

  [[nodiscard]] std::expected<float, StarException> setResourceValue(String const& resourceName, float value);
  [[nodiscard]] std::expected<float, StarException> modifyResourceValue(String const& resourceName, float amount);

  // Similar to consumeResource, will add the given amount to a resource if
  // it exists. Returns the amount by which the resource was actually increased.
  float giveResourceValue(String const& resourceName, float amount);

  // If a resource exists and has more than the given amount available, and the
  // resource is not locked, then subtracts this amount from the resource and
  // returns true. Otherwise, does nothing and returns false. A negative amount
  // is an error; a missing resource is an ordinary false outcome.
  [[nodiscard]] std::expected<bool, StarException> consumeResourceValue(String const& resourceName, float amount);

  // Like consumeResource, but always succeeds if the resource is unlocked and
  // the amount is nonzero.  If the amount is greater than the available
  // resource, then the resource will be consumed to zero.
  [[nodiscard]] std::expected<bool, StarException> overConsumeResourceValue(String const& resourceName, float amount);

  // A locked resource cannot be consumed in any way.
  [[nodiscard]] std::expected<bool, StarException> resourceLocked(String const& resourceName) const;
  [[nodiscard]] std::expected<void, StarException> setResourceLocked(String const& resourceName, bool locked);

  // A resource without a maximum has no percentage.
  [[nodiscard]] std::expected<std::optional<float>, StarException> resourceMaxValue(String const& resourceName) const;
  [[nodiscard]] std::expected<std::optional<float>, StarException> resourcePercentage(String const& resourceName) const;
  [[nodiscard]] std::expected<float, StarException> setResourcePercentage(String const& resourceName, float resourcePercentage);
  [[nodiscard]] std::expected<float, StarException> modifyResourcePercentage(String const& resourceName, float resourcePercentage);

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
    std::variant<std::monostate, String, float> max;
    std::variant<std::monostate, String, float> delta;
    bool locked;
    float value;
    std::optional<float> maxValue;

    // Sets value and clamps between [0.0, maxStatValue] or just >= 0.0 if
    // maxStatValue is not given.
    float setValue(float v);
  };

  [[nodiscard]] std::expected<Resource const*, StarException> getResource(String const& resourceName) const;
  [[nodiscard]] std::expected<Resource*, StarException> getResource(String const& resourceName);

  [[nodiscard]] std::expected<bool, StarException> consumeResourceValue(String const& resourceName, float amount, bool allowOverConsume);

  StringMap<float> m_baseStats;
  StringMap<EffectiveStat> m_effectiveStats;
  StatModifierGroupMap m_statModifierGroups;
  StringMap<Resource> m_resources;
};

}

export module star.stat_set;

export namespace Star {
  using ::Star::StatSet;
}
