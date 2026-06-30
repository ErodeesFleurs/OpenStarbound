#include "StarItem.hpp"
#include "StarAlgorithm.hpp"
#include "StarRoot.hpp"
#include "StarJsonExtra.hpp"
#include "StarRandom.hpp"
#include "StarLogging.hpp"
#include "StarWorldLuaBindings.hpp"

namespace Star {

Item::Item(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json config, String directory, Json parameters)
  : m_config(std::move(config))
  , m_directory(std::move(directory))
  , m_imageMetadataDatabase(requireServiceValueAs<ItemException>(std::move(imageMetadataDatabase), "Item", "image metadata database"))
  , m_name(m_config.getString("itemName"))
  , m_count(1)
  , m_parameters(std::move(parameters)) {
  assets = requireServiceValueAs<ItemException>(std::move(assets), "Item", "assets");

  m_maxStack = instanceValue("maxStack", assets->json("/items/defaultParameters.config:defaultMaxStack").toInt()).toInt();
  m_shortDescription = instanceValue("shortdescription", "").toString();
  m_description = instanceValue("description", "").toString();

  m_rarity = RarityNames.getLeft(instanceValue("rarity").toString());

  setIconDrawables(iconDrawablesFromJson(instanceValue("inventoryIcon", assets->json("/items/defaultParameters.config:missingIcon"))));
  setSecondaryIconDrawables(secondaryIconDrawablesFromJson(instanceValue("secondaryIcon", Json())));

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

  m_timeToLive = instanceValue("timeToLive", assets->json("/items/defaultParameters.config:defaultTimeToLive").toFloat()).toFloat();

  for (auto b : jsonToStringList(instanceValue("learnBlueprintsOnPickup", JsonArray{})))
    m_learnBlueprintsOnPickup.append(ItemDescriptor(b));

  for (auto const& [collectionName, collectableName] : instanceValue("collectablesOnPickup", JsonObject{}).iterateObject())
    m_collectablesOnPickup[collectionName] = collectableName.toString();
}

Item::~Item() = default;

List<Drawable> Item::iconDrawablesFromJson(Json const& icon) const {
  if (icon.type() == Json::Type::Array) {
    return icon.toArray().transformed([&](Json config) -> Drawable {
      if (auto image = config.optString("image"))
        return Drawable(config.set("image", AssetPath::relativeTo(m_directory, *image)), m_imageMetadataDatabase);
      return Drawable(config, m_imageMetadataDatabase);
    });
  }

  auto image = AssetPath::relativeTo(m_directory, icon.toString());
  return {Drawable::makeImage(image, 1.0f, true, Vec2F(), Color::White, m_imageMetadataDatabase)};
}

Maybe<List<Drawable>> Item::secondaryIconDrawablesFromJson(Json const& icon) const {
  if (icon.type() == Json::Type::Array || icon.type() == Json::Type::String)
    return iconDrawablesFromJson(icon);
  return Maybe<List<Drawable>>();
}

void Item::normalizeIconDrawables(List<Drawable>& drawables) {
  auto boundBox = Drawable::boundBoxAll(drawables, true, m_imageMetadataDatabase);
  if (!boundBox.isEmpty()) {
    for (auto& drawable : drawables)
      drawable.translate(-boundBox.center());
    // TODO: Why 16?  Is this the size of the icon container?  Shouldn't this
    // be configurable?
    float zoom = 16.0f / std::max(boundBox.width(), boundBox.height());
    if (zoom < 1) {
      for (auto& drawable : drawables)
        drawable.scale(zoom);
    }
  }
}

[[nodiscard]] String Item::name() const {
  return m_name;
}

[[nodiscard]] uint64_t Item::count() const {
  return m_count;
}

uint64_t Item::setCount(uint64_t count, bool overfill) {
  if (overfill)
    m_count = count;
  else
    m_count = std::min(count, m_maxStack);
  return count - m_count;
}

[[nodiscard]] bool Item::stackableWith(ItemConstPtr const& item) const {
  return item && name() == item->name() && parameters() == item->parameters();
}

[[nodiscard]] uint64_t Item::maxStack() const {
  return m_maxStack;
}

[[nodiscard]] uint64_t Item::couldStack(ItemConstPtr const& item) const {
  if (stackableWith(item) && m_count < m_maxStack) {
    uint64_t take = m_maxStack - m_count;
    return std::min(take, item->count());
  } else {
    return 0;
  }
}

bool Item::stackWith(ItemPtr const& item) {
  uint64_t take = couldStack(item);

  if (take > 0 && item->consume(take)) {
    m_count += take;
    return true;
  } else {
    return false;
  }
}

[[nodiscard]] bool Item::matches(ItemDescriptor const& descriptor, bool exactMatch) const {
  return descriptor.name() == m_name && (!exactMatch || descriptor.parameters() == m_parameters);
}

[[nodiscard]] bool Item::matches(ItemConstPtr const& other, bool exactMatch) const {
  return other->name() == m_name && (!exactMatch || other->parameters() == m_parameters);
}

bool Item::consume(uint64_t count) {
  if (m_count >= count) {
    m_count -= count;
    return true;
  } else {
    return false;
  }
}

ItemPtr Item::take(uint64_t max) {
  uint64_t takeCount = std::min(m_count, max);
  if (takeCount != 0) {
    if (auto newItems = clone()) {
      m_count -= takeCount;
      newItems->setCount(takeCount);
      return newItems;
    } else {
      Logger::warn(strf("Could not clone {}, not moving {} items as requested.", friendlyName(), takeCount).c_str());
    }
  }

  return {};
}

[[nodiscard]] bool Item::empty() const {
  return m_count == 0;
}

[[nodiscard]] ItemDescriptor Item::descriptor() const {
  return ItemDescriptor(m_name, m_count, m_parameters);
}

[[nodiscard]] String Item::description() const {
  return m_description;
}

[[nodiscard]] String Item::friendlyName() const {
  return m_shortDescription;
}

[[nodiscard]] Rarity Item::rarity() const {
  return m_rarity;
}

[[nodiscard]] List<Drawable> Item::iconDrawables() const {
  return m_iconDrawables;
}

[[nodiscard]] Maybe<List<Drawable>> Item::secondaryDrawables() const {
  return m_secondaryIconDrawables;
}

[[nodiscard]] bool Item::hasSecondaryDrawables() const {
  return m_secondaryIconDrawables.isValid();
}

[[nodiscard]] List<Drawable> Item::dropDrawables() const {
  auto drawables = iconDrawables();
  Drawable::scaleAll(drawables, 1.0f / TilePixels);
  return drawables;
}

[[nodiscard]] bool Item::twoHanded() const {
  return m_twoHanded;
}

[[nodiscard]] float Item::timeToLive() const {
  return m_timeToLive;
}

[[nodiscard]] uint64_t Item::price() const {
  return m_price * count();
}

[[nodiscard]] String Item::tooltipKind() const {
  return m_tooltipKind;
}

[[nodiscard]] String Item::largeImage() const {
  return m_largeImage;
}

[[nodiscard]] String Item::category() const {
  return m_category;
}

[[nodiscard]] String Item::pickupSound() const {
  return Random::randFrom(m_pickupSounds);
}

void Item::setMaxStack(uint64_t maxStack) {
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

void Item::setPrice(uint64_t price) {
  m_price = price;
}

void Item::setIconDrawables(List<Drawable> drawables) {
  m_iconDrawables = std::move(drawables);
  normalizeIconDrawables(m_iconDrawables);
}

void Item::setSecondaryIconDrawables(Maybe<List<Drawable>> drawables) {
  m_secondaryIconDrawables = std::move(drawables);
  if (m_secondaryIconDrawables.isNothing())
    return;

  normalizeIconDrawables(*m_secondaryIconDrawables);
}

void Item::setTwoHanded(bool twoHanded) {
  m_twoHanded = twoHanded;
}

void Item::setTimeToLive(float timeToLive) {
  m_timeToLive = timeToLive;
}

[[nodiscard]] List<QuestArcDescriptor> Item::pickupQuestTemplates() const {
  return instanceValue("pickupQuestTemplates", JsonArray{}).toArray().transformed(&QuestArcDescriptor::fromJson);
}

[[nodiscard]] StringSet Item::itemTags() const {
  return jsonToStringSet(m_config.get("itemTags", JsonArray{}));
}

[[nodiscard]] bool Item::hasItemTag(String const& itemTag) const {
  return itemTags().contains(itemTag);
}

[[nodiscard]] Json Item::instanceValue(String const& name, Json const& def) const {
  return jsonMergeQueryDef(name, def, m_config, m_parameters);
}

[[nodiscard]] Json Item::instanceValueOfType(String const& name, Json::Type type, Json const& def) const {
  auto value = instanceValue(name, def);
  if (value.isType(type))
    return value;
  return def;
}

[[nodiscard]] Json Item::instanceValues() const {
  return m_config.setAll(m_parameters.toObject());
}

[[nodiscard]] Json Item::config() const {
  return m_config;
}

[[nodiscard]] Json Item::parameters() const {
  return m_parameters;
}

void Item::setInstanceValue(String const& name, Json const& value) {
  if (m_parameters.get(name, {}) != value)
    m_parameters = m_parameters.setAll(JsonObject{{name, value}});
}

[[nodiscard]] String const& Item::directory() const {
  return m_directory;
}

[[nodiscard]] List<ItemDescriptor> Item::learnBlueprintsOnPickup() const {
  return m_learnBlueprintsOnPickup;
}

[[nodiscard]] StringMap<String> Item::collectablesOnPickup() const {
  return m_collectablesOnPickup;
}

GenericItem::GenericItem(AssetsConstPtr assets, Json const& config, String const& directory, Json const& parameters)
  : GenericItem(std::move(assets), {}, config, directory, parameters) {}

GenericItem::GenericItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& parameters)
  : Item(std::move(assets), std::move(imageMetadataDatabase), config, directory, parameters) {}

ItemPtr GenericItem::clone() const {
  return make_shared<GenericItem>(*this);
}

[[nodiscard]] bool Item::itemsEqual(ItemConstPtr const& a, ItemConstPtr const& b) {
  if (!a && !b) // Both are null
    return true;
  if (a && b) // Both aren't null, compare
    return a->stackableWith(b);
  else // One is null, so not equal
    return false;
}

}
