#include "StarItem.hpp"

#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarLogging.hpp"
#include "StarRandom.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

Item::Item(Json config, String directory, Json parameters) {
  m_config = std::move(config);
  m_directory = std::move(directory);
  m_parameters = std::move(parameters);
  m_name = m_config.getString("itemName");
  m_count = 1;

  m_maxStack = instanceValue("maxStack", Root::singleton().assets()->json("/items/defaultParameters.config:defaultMaxStack").toInt()).toInt();
  m_shortDescription = instanceValue("shortdescription", "").toString();
  m_description = instanceValue("description", "").toString();

  m_rarity = RarityNames.getLeft(instanceValue("rarity").toString());

  auto inventoryIcon = instanceValue("inventoryIcon", Root::singleton().assets()->json("/items/defaultParameters.config:missingIcon"));
  if (inventoryIcon.type() == Json::Type::Array) {
    setIconDrawables(inventoryIcon.toArray().transformed([&](Json config) -> Drawable {
      if (auto image = config.optString("image"))
        return Drawable(config.set("image", AssetPath::relativeTo(m_directory, *image)));
      return Drawable(config);
    }));
  } else {
    auto image = AssetPath::relativeTo(m_directory, inventoryIcon.toString());
    setIconDrawables({Drawable::makeImage(image, 1.0f, true, Vec2F())});
  }
  auto secondaryIcon = instanceValue("secondaryIcon", Json());
  if (secondaryIcon.type() == Json::Type::Array) {
    setSecondaryIconDrawables(secondaryIcon.toArray().transformed([&](Json config) -> Drawable {
      if (auto image = config.optString("image"))
        return Drawable(config.set("image", AssetPath::relativeTo(m_directory, *image)));
      return Drawable(config);
    }));
  } else if (secondaryIcon.type() == Json::Type::String) {
    auto image = AssetPath::relativeTo(m_directory, secondaryIcon.toString());
    setSecondaryIconDrawables(std::optional<List<Drawable>>({Drawable::makeImage(image, 1.0f, true, Vec2F())}));
  } else {
    setSecondaryIconDrawables(std::optional<List<Drawable>>());
  }

  auto assets = Root::singleton().assets();
  m_twoHanded = instanceValue("twoHanded", false).toBool();
  m_price = instanceValue("price", assets->json("/items/defaultParameters.config:defaultPrice")).toInt();
  m_tooltipKind = instanceValue("tooltipKind", "").toString();
  auto largeImage = instanceValue("largeImage");
  if (!largeImage.isNull())
    m_largeImage = AssetPath::relativeTo(m_directory, largeImage.toString());

  m_category = instanceValue("category", "").toString();
  m_pickupSounds = jsonToStringSet(m_config.get("pickupSounds", JsonArray{}));
  if (!m_pickupSounds.size())
    m_pickupSounds = jsonToStringSet(assets->json("/items/defaultParameters.config:pickupSounds"));

  m_timeToLive = instanceValue("timeToLive", Root::singleton().assets()->json("/items/defaultParameters.config:defaultTimeToLive").toFloat()).toFloat();

  for (auto b : jsonToStringList(instanceValue("learnBlueprintsOnPickup", JsonArray{})))
    m_learnBlueprintsOnPickup.append(ItemDescriptor(b));

  for (auto pair : instanceValue("collectablesOnPickup", JsonObject{}).iterateObject())
    m_collectablesOnPickup[pair.first] = pair.second.toString();
}

Item::~Item() = default;

auto Item::name() const -> String {
  return m_name;
}

auto Item::count() const -> std::uint64_t {
  return m_count;
}

auto Item::setCount(std::uint64_t count, bool overfill) -> std::uint64_t {
  if (overfill)
    m_count = count;
  else
    m_count = std::min(count, m_maxStack);
  return count - m_count;
}

auto Item::stackableWith(ConstPtr<Item> const& item) const -> bool {
  return item && name() == item->name() && parameters() == item->parameters();
}

auto Item::maxStack() const -> std::uint64_t {
  return m_maxStack;
}

auto Item::couldStack(ConstPtr<Item> const& item) const -> std::uint64_t {
  if (stackableWith(item) && m_count < m_maxStack) {
    std::uint64_t take = m_maxStack - m_count;
    return std::min(take, item->count());
  } else {
    return 0;
  }
}

auto Item::stackWith(Ptr<Item> const& item) -> bool {
  std::uint64_t take = couldStack(item);

  if (take > 0 && item->consume(take)) {
    m_count += take;
    return true;
  } else {
    return false;
  }
}

auto Item::matches(ItemDescriptor const& descriptor, bool exactMatch) const -> bool {
  return descriptor.name() == m_name && (!exactMatch || descriptor.parameters() == m_parameters);
}

auto Item::matches(ConstPtr<Item> const& other, bool exactMatch) const -> bool {
  return other->name() == m_name && (!exactMatch || other->parameters() == m_parameters);
}

auto Item::consume(std::uint64_t count) -> bool {
  if (m_count >= count) {
    m_count -= count;
    return true;
  } else {
    return false;
  }
}

auto Item::take(std::uint64_t max) -> Ptr<Item> {
  std::uint64_t takeCount = std::min(m_count, max);
  if (takeCount != 0) {
    if (auto newItems = clone()) {
      m_count -= takeCount;
      newItems->setCount(takeCount);
      return newItems;
    } else {
      Logger::warn("Could not clone {}, not moving {} items as requested.", friendlyName(), takeCount);
    }
  }

  return {};
}

auto Item::empty() const -> bool {
  return m_count == 0;
}

auto Item::descriptor() const -> ItemDescriptor {
  return {m_name, m_count, m_parameters};
}

auto Item::description() const -> String {
  return m_description;
}

auto Item::friendlyName() const -> String {
  return m_shortDescription;
}

auto Item::rarity() const -> Rarity {
  return m_rarity;
}

auto Item::iconDrawables() const -> List<Drawable> {
  return m_iconDrawables;
}

auto Item::secondaryDrawables() const -> std::optional<List<Drawable>> {
  return m_secondaryIconDrawables;
}

auto Item::hasSecondaryDrawables() const -> bool {
  return m_secondaryIconDrawables.has_value();
}

auto Item::dropDrawables() const -> List<Drawable> {
  auto drawables = iconDrawables();
  Drawable::scaleAll(drawables, 1.0f / TilePixels);
  return drawables;
}

auto Item::twoHanded() const -> bool {
  return m_twoHanded;
}

auto Item::timeToLive() const -> float {
  return m_timeToLive;
}

auto Item::price() const -> std::uint64_t {
  return m_price * count();
}

auto Item::tooltipKind() const -> String {
  return m_tooltipKind;
}

auto Item::largeImage() const -> String {
  return m_largeImage;
}

auto Item::category() const -> String {
  return m_category;
}

auto Item::pickupSound() const -> String {
  return Random::randFrom(m_pickupSounds);
}

void Item::setMaxStack(std::uint64_t maxStack) {
  m_maxStack = maxStack;
}

void Item::setDescription(String const& description) {
  m_description = description;
}

void Item::setShortDescription(String const& description) {
  m_shortDescription = description;
}

void Item::setRarity(Rarity rarity) {
  m_rarity = rarity;
}

void Item::setPrice(std::uint64_t price) {
  m_price = price;
}

void Item::setIconDrawables(List<Drawable> drawables) {
  m_iconDrawables = std::move(drawables);
  auto boundBox = Drawable::boundBoxAll(m_iconDrawables, true);
  if (!boundBox.isEmpty()) {
    for (auto& drawable : m_iconDrawables)
      drawable.translate(-boundBox.center());
    // TODO: Why 16?  Is this the size of the icon container?  Shouldn't this
    // be configurable?
    float zoom = 16.0f / std::max(boundBox.width(), boundBox.height());
    if (zoom < 1) {
      for (auto& drawable : m_iconDrawables)
        drawable.scale(zoom);
    }
  }
}

void Item::setSecondaryIconDrawables(std::optional<List<Drawable>> drawables) {
  m_secondaryIconDrawables = std::move(drawables);
  if (!m_secondaryIconDrawables)
    return;

  auto boundBox = Drawable::boundBoxAll(*m_secondaryIconDrawables, true);
  if (!boundBox.isEmpty()) {
    for (auto& drawable : *m_secondaryIconDrawables)
      drawable.translate(-boundBox.center());
    // TODO: Why 16?  Is this the size of the icon container?  Shouldn't this
    // be configurable?
    float zoom = 16.0f / std::max(boundBox.width(), boundBox.height());
    if (zoom < 1) {
      for (auto& drawable : *m_secondaryIconDrawables)
        drawable.scale(zoom);
    }
  }
}

void Item::setTwoHanded(bool twoHanded) {
  m_twoHanded = twoHanded;
}

void Item::setTimeToLive(float timeToLive) {
  m_timeToLive = timeToLive;
}

auto Item::pickupQuestTemplates() const -> List<QuestArcDescriptor> {
  return instanceValue("pickupQuestTemplates", JsonArray{}).toArray().transformed(&QuestArcDescriptor::fromJson);
}

auto Item::itemTags() const -> StringSet {
  return jsonToStringSet(m_config.get("itemTags", JsonArray{}));
}

auto Item::hasItemTag(String const& itemTag) const -> bool {
  return itemTags().contains(itemTag);
}

auto Item::instanceValue(String const& name, Json const& def) const -> Json {
  return jsonMergeQueryDef(name, def, m_config, m_parameters);
}

auto Item::instanceValueOfType(String const& name, Json::Type type, Json const& def) const -> Json {
  auto value = instanceValue(name, def);
  if (value.isType(type))
    return value;
  return def;
}

auto Item::instanceValues() const -> Json {
  return m_config.setAll(m_parameters.toObject());
}

auto Item::config() const -> Json {
  return m_config;
}

auto Item::parameters() const -> Json {
  return m_parameters;
}

void Item::setInstanceValue(String const& name, Json const& value) {
  if (m_parameters.get(name, {}) != value)
    m_parameters = m_parameters.setAll(JsonObject{{name, value}});
}

auto Item::directory() const -> String const& {
  return m_directory;
}

auto Item::learnBlueprintsOnPickup() const -> List<ItemDescriptor> {
  return m_learnBlueprintsOnPickup;
}

auto Item::collectablesOnPickup() const -> StringMap<String> {
  return m_collectablesOnPickup;
}

GenericItem::GenericItem(Json const& config, String const& directory, Json const& parameters)
    : Item(config, directory, parameters) {}

auto GenericItem::clone() const -> Ptr<Item> {
  return std::make_shared<GenericItem>(*this);
}

auto Item::itemsEqual(ConstPtr<Item> const& a, ConstPtr<Item> const& b) -> bool {
  if (!a && !b)// Both are null
    return true;
  if (a && b)// Both aren't null, compare
    return a->stackableWith(b);
  else// One is null, so not equal
    return false;
}

}// namespace Star
