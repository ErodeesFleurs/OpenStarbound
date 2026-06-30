#pragma once

#include "StarSet.hpp"
#include "StarDrawable.hpp"
#include "StarItemDescriptor.hpp"
#include "StarQuests.hpp"
#include "StarAssets.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {

class Item;
using ItemPtr = SharedPtr<Item>;
using ItemConstPtr = SharedPtr<Item const>;
class GenericItem;

struct ItemExceptionTag { static constexpr char const* typeName = "ItemException"; };
using ItemException = TypedException<StarException, ItemExceptionTag>;

class Item {
public:
  // Config here is the configuration loaded directly from assets, directory is
  // the asset path this config was found in, that other assets should be
  // loaded relative to.
  Item(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json config, String directory, Json parameters = JsonObject());

  // For items which do not come from files
  Item();

  virtual ~Item();

  [[nodiscard]] virtual ItemPtr clone() const = 0;

  // Unique identifying item name
  [[nodiscard]] String name() const;

  // Number of this item that is available.
  [[nodiscard]] uint64_t count() const;
  // Sets the new item count, up to a max of the maximum stack size.  If this
  // value is over stack size, returns the overflow.  If 'overfill' is set to
  // true, then will fill past max stack level.
  [[nodiscard]] uint64_t setCount(uint64_t count, bool overfill = false);

  // Is this item type stackable with the given item type at all?  Base class
  // implementation compares name(), and m_parameters fields and returns true
  // if they are both the same, similarly to matches.
  [[nodiscard]] virtual bool stackableWith(ItemConstPtr const& item) const;
  [[nodiscard]] uint64_t maxStack() const;

  // Return how many of the given item could be shifted into this item, taking
  // into acount whether the item is stackable at all, as well as maxStack and
  // the count available.
  [[nodiscard]] uint64_t couldStack(ItemConstPtr const& item) const;

  // If the given item is stackable with this one, takes as many from the given
  // item as possible and shifts it into this item's count.  Returns true if
  // any items at all were shifted.
  [[nodiscard]] bool stackWith(ItemPtr const& item);

  // Does this item match the given item or itemDescriptor
  [[nodiscard]] bool matches(ItemDescriptor const& descriptor, bool exactMatch = false) const;
  [[nodiscard]] bool matches(ItemConstPtr const& other, bool exactMatch = false) const;

  // List of itemdescriptors for which the current item could be used in the
  // place of
  // in recipes and the like.
  [[nodiscard]] List<ItemDescriptor> matchingDescriptors() const;

  // If the given number of this item is available, consumes that number and
  // returns true, otherwise returns false.
  [[nodiscard]] bool consume(uint64_t count);

  // Take as many of this item as possible up to the given max (default is all)
  // and return the new set.  Implementation uses clone() method.
  [[nodiscard]] ItemPtr take(uint64_t max = NPos);

  // count() is 0
  [[nodiscard]] bool empty() const;

  // Builds a descriptor out of name(), count(), and m_parameters
  [[nodiscard]] ItemDescriptor descriptor() const;

  [[nodiscard]] String description() const;
  [[nodiscard]] String friendlyName() const;

  [[nodiscard]] Rarity rarity() const;
  [[nodiscard]] uint64_t price() const;

  [[nodiscard]] virtual List<Drawable> iconDrawables() const;
  [[nodiscard]] virtual Maybe<List<Drawable>> secondaryDrawables() const;
  [[nodiscard]] virtual bool hasSecondaryDrawables() const;

  [[nodiscard]] virtual List<Drawable> dropDrawables() const;
  [[nodiscard]] String largeImage() const;

  [[nodiscard]] String tooltipKind() const;
  [[nodiscard]] virtual String category() const;

  [[nodiscard]] virtual String pickupSound() const;

  [[nodiscard]] bool twoHanded() const;
  [[nodiscard]] float timeToLive() const;

  [[nodiscard]] List<ItemDescriptor> learnBlueprintsOnPickup() const;
  [[nodiscard]] StringMap<String> collectablesOnPickup() const;

  [[nodiscard]] List<QuestArcDescriptor> pickupQuestTemplates() const;
  [[nodiscard]] StringSet itemTags() const;
  [[nodiscard]] bool hasItemTag(String const& itemTag) const;

  // Return either a parameter given to the item or a config value, if no such
  // parameter exists.
  [[nodiscard]] Json instanceValue(String const& name, Json const& def = Json()) const;
  [[nodiscard]] Json instanceValueOfType(String const& name, Json::Type type, Json const& def = Json()) const;

  // Returns the full set of configuration values merged with parameters
  [[nodiscard]] Json instanceValues() const;

  // Returns just the base config
  [[nodiscard]] Json config() const;

  // Returns just the dynamic parameters
  [[nodiscard]] Json parameters() const;

  [[nodiscard]] static bool itemsEqual(ItemConstPtr const& a, ItemConstPtr const& b);

protected:
  void setMaxStack(uint64_t maxStack);
  void setDescription(String const& description);
  void setShortDescription(String const& description);

  void setRarity(Rarity rarity);
  void setPrice(uint64_t price);
  // icon drawables are pixels, not tile, based
  void setIconDrawables(List<Drawable> drawables);
  void setSecondaryIconDrawables(Maybe<List<Drawable>> drawables);
  [[nodiscard]] List<Drawable> iconDrawablesFromJson(Json const& icon) const;
  [[nodiscard]] Maybe<List<Drawable>> secondaryIconDrawablesFromJson(Json const& icon) const;
  void setTwoHanded(bool twoHanded);
  void setTimeToLive(float timeToLive);

  void setInstanceValue(String const& name, Json const& val);

  [[nodiscard]] String const& directory() const;

private:
  void normalizeIconDrawables(List<Drawable>& drawables);

  Json m_config;
  String m_directory;
protected:
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;

private:
  String m_name;
  uint64_t m_count;
  Json m_parameters;

  uint64_t m_maxStack;
  String m_shortDescription;
  String m_description;
  Rarity m_rarity;
  List<Drawable> m_iconDrawables;
  Maybe<List<Drawable>> m_secondaryIconDrawables;
  bool m_twoHanded;
  float m_timeToLive;
  uint64_t m_price;
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
  GenericItem(AssetsConstPtr assets, Json const& config, String const& directory, Json const& parameters);
  GenericItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& parameters);
  [[nodiscard]] virtual ItemPtr clone() const;
};

[[nodiscard]] inline uint64_t itemSafeCount(ItemPtr const& item) {
  return item ? item->count() : 0;
}

[[nodiscard]] inline bool itemSafeTwoHanded(ItemPtr const& item) {
  return item && item->twoHanded();
}

[[nodiscard]] inline bool itemSafeOneHanded(ItemPtr const& item) {
  return item && !item->twoHanded();
}

[[nodiscard]] inline ItemDescriptor itemSafeDescriptor(ItemPtr const& item) {
  return item ? item->descriptor() : ItemDescriptor();
}
}
