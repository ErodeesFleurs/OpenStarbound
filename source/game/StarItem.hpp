#pragma once

#include "StarConfig.hpp"
#include "StarDrawable.hpp"
#include "StarException.hpp"
#include "StarGameTypes.hpp"
#include "StarItemDescriptor.hpp"
#include "StarQuestDescriptor.hpp"

import std;

namespace Star {

using ItemException = ExceptionDerived<"ItemException">;

class Item {
public:
  // Config here is the configuration loaded directly from assets, directory is
  // the asset path this config was found in, that other assets should be
  // loaded relative to.
  Item(Json config, String directory, Json parameters = JsonObject());

  // For items which do not come from files
  Item();

  virtual ~Item();

  [[nodiscard]] virtual auto clone() const -> Ptr<Item> = 0;

  // Unique identifying item name
  [[nodiscard]] auto name() const -> String;

  // Number of this item that is available.
  [[nodiscard]] auto count() const -> std::uint64_t;
  // Sets the new item count, up to a max of the maximum stack size.  If this
  // value is over stack size, returns the overflow.  If 'overfill' is set to
  // true, then will fill past max stack level.
  auto setCount(std::uint64_t count, bool overfill = false) -> std::uint64_t;

  // Is this item type stackable with the given item type at all?  Base class
  // implementation compares name(), and m_parameters fields and returns true
  // if they are both the same, similarly to matches.
  [[nodiscard]] virtual auto stackableWith(ConstPtr<Item> const& item) const -> bool;
  [[nodiscard]] auto maxStack() const -> std::uint64_t;

  // Return how many of the given item could be shifted into this item, taking
  // into acount whether the item is stackable at all, as well as maxStack and
  // the count available.
  [[nodiscard]] auto couldStack(ConstPtr<Item> const& item) const -> std::uint64_t;

  // If the given item is stackable with this one, takes as many from the given
  // item as possible and shifts it into this item's count.  Returns true if
  // any items at all were shifted.
  auto stackWith(Ptr<Item> const& item) -> bool;

  // Does this item match the given item or itemDescriptor
  [[nodiscard]] auto matches(ItemDescriptor const& descriptor, bool exactMatch = false) const -> bool;
  [[nodiscard]] auto matches(ConstPtr<Item> const& other, bool exactMatch = false) const -> bool;

  // List of itemdescriptors for which the current item could be used in the
  // place of
  // in recipes and the like.
  [[nodiscard]] auto matchingDescriptors() const -> List<ItemDescriptor>;

  // If the given number of this item is available, consumes that number and
  // returns true, otherwise returns false.
  auto consume(std::uint64_t count) -> bool;

  // Take as many of this item as possible up to the given max (default is all)
  // and return the new set.  Implementation uses clone() method.
  auto take(std::uint64_t max = std::numeric_limits<std::size_t>::max()) -> Ptr<Item>;

  // count() is 0
  [[nodiscard]] auto empty() const -> bool;

  // Builds a descriptor out of name(), count(), and m_parameters
  [[nodiscard]] auto descriptor() const -> ItemDescriptor;

  [[nodiscard]] auto description() const -> String;
  [[nodiscard]] auto friendlyName() const -> String;

  [[nodiscard]] auto rarity() const -> Rarity;
  [[nodiscard]] auto price() const -> std::uint64_t;

  [[nodiscard]] virtual auto iconDrawables() const -> List<Drawable>;
  [[nodiscard]] virtual auto secondaryDrawables() const -> std::optional<List<Drawable>>;
  [[nodiscard]] virtual auto hasSecondaryDrawables() const -> bool;

  [[nodiscard]] virtual auto dropDrawables() const -> List<Drawable>;
  [[nodiscard]] auto largeImage() const -> String;

  [[nodiscard]] auto tooltipKind() const -> String;
  [[nodiscard]] virtual auto category() const -> String;

  [[nodiscard]] virtual auto pickupSound() const -> String;

  [[nodiscard]] auto twoHanded() const -> bool;
  [[nodiscard]] auto timeToLive() const -> float;

  [[nodiscard]] auto learnBlueprintsOnPickup() const -> List<ItemDescriptor>;
  [[nodiscard]] auto collectablesOnPickup() const -> StringMap<String>;

  [[nodiscard]] auto pickupQuestTemplates() const -> List<QuestArcDescriptor>;
  [[nodiscard]] auto itemTags() const -> StringSet;
  [[nodiscard]] auto hasItemTag(String const& itemTag) const -> bool;

  // Return either a parameter given to the item or a config value, if no such
  // parameter exists.
  [[nodiscard]] auto instanceValue(String const& name, Json const& def = Json()) const -> Json;
  [[nodiscard]] auto instanceValueOfType(String const& name, Json::Type type, Json const& def = Json()) const -> Json;

  // Returns the full set of configuration values merged with parameters
  [[nodiscard]] auto instanceValues() const -> Json;

  // Returns just the base config
  [[nodiscard]] auto config() const -> Json;

  // Returns just the dynamic parameters
  [[nodiscard]] auto parameters() const -> Json;

  static auto itemsEqual(ConstPtr<Item> const& a, ConstPtr<Item> const& b) -> bool;

protected:
  void setMaxStack(std::uint64_t maxStack);
  void setDescription(String const& description);
  void setShortDescription(String const& description);

  void setRarity(Rarity rarity);
  void setPrice(std::uint64_t price);
  // icon drawables are pixels, not tile, based
  void setIconDrawables(List<Drawable> drawables);
  void setSecondaryIconDrawables(std::optional<List<Drawable>> drawables);
  void setTwoHanded(bool twoHanded);
  void setTimeToLive(float timeToLive);

  void setInstanceValue(String const& name, Json const& val);

  [[nodiscard]] auto directory() const -> String const&;

private:
  Json m_config;
  String m_directory;

  String m_name;
  std::uint64_t m_count;
  Json m_parameters;

  std::uint64_t m_maxStack;
  String m_shortDescription;
  String m_description;
  Rarity m_rarity;
  List<Drawable> m_iconDrawables;
  std::optional<List<Drawable>> m_secondaryIconDrawables;
  bool m_twoHanded;
  float m_timeToLive;
  std::uint64_t m_price;
  String m_tooltipKind;
  String m_largeImage;
  String m_category;
  StringSet m_pickupSounds;

  List<ItemDescriptor> m_matchingDescriptors;
  List<ItemDescriptor> m_learnBlueprintsOnPickup;
  StringMap<String> m_collectablesOnPickup;
};

class GenericItem : public Item {
public:
  GenericItem(Json const& config, String const& directory, Json const& parameters);
  [[nodiscard]] auto clone() const -> Ptr<Item> override;
};

inline auto itemSafeCount(Ptr<Item> const& item) -> std::uint64_t {
  return item ? item->count() : 0;
}

inline auto itemSafeTwoHanded(Ptr<Item> const& item) -> bool {
  return item && item->twoHanded();
}

inline auto itemSafeOneHanded(Ptr<Item> const& item) -> bool {
  return item && !item->twoHanded();
}

inline auto itemSafeDescriptor(Ptr<Item> const& item) -> ItemDescriptor {
  return item ? item->descriptor() : ItemDescriptor();
}
}// namespace Star
