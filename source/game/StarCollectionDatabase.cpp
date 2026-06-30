#include "StarCollectionDatabase.hpp"
#include "StarMonsterDatabase.hpp"
#include "StarItemDatabase.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

EnumMap<CollectionType> const CollectionTypeNames {
  {CollectionType::Generic, "generic"},
  {CollectionType::Item, "item"},
  {CollectionType::Monster, "monster"}
};

Collection::Collection() = default;

Collection::Collection(String const& name, CollectionType type, String const& title) : name(name), title(title), type(type) {}

Collectable::Collectable() = default;

Collectable::Collectable(String const& name, int order, String const& title, String const& description, String const& icon)
  : name(name), order(order), title(title), description(description), icon(icon) {};

CollectionDatabase::CollectionDatabase(AssetsConstPtr assets, MonsterDatabaseConstPtr monsterDatabase, ItemDatabaseConstPtr itemDatabase)
  : m_monsterDatabase(requireDependencyValueAs<CollectionDatabaseException>(std::move(monsterDatabase), "CollectionDatabase", "monster database")),
    m_itemDatabase(requireDependencyValueAs<CollectionDatabaseException>(std::move(itemDatabase), "CollectionDatabase", "item database")) {
  assets = requireServiceValueAs<CollectionDatabaseException>(std::move(assets), "CollectionDatabase", "assets");

  auto& files = assets->scanExtension("collection");
  assets->queueJsons(files);
  for (auto& file : files) {
    auto config = assets->json(file);

    Collection collection;

    collection.name = config.getString("name");
    collection.title = config.getString("title", collection.name);
    collection.type = CollectionTypeNames.getLeft(config.getString("type", "generic"));

    m_collectables[collection.name] = {};
    for (auto const& [collectableName, collectableConfig] : config.get("collectables").iterateObject()) {
      Collectable collectable;
      if (collection.type == CollectionType::Monster)
        collectable = parseMonsterCollectable(collectableName, collectableConfig);
      else if (collection.type == CollectionType::Item)
        collectable = parseItemCollectable(collectableName, collectableConfig);
      else
        collectable = parseGenericCollectable(collectableName, collectableConfig);

      m_collectables[collection.name][collectable.name] = collectable;
    }

    m_collections[collection.name] = collection;
  }
}

List<Collection> CollectionDatabase::collections() const {
  return m_collections.values();
}

Collection CollectionDatabase::collection(String const& collectionName) const {
  try {
    return m_collections.get(collectionName); 
  } catch (MapException const& e) {
    throw CollectionDatabaseException(strf("Collection '{}' not found", collectionName), e);
  }
}

List<Collectable> CollectionDatabase::collectables(String const& collectionName) const {
  try {
    return m_collectables.get(collectionName).values();
  } catch (MapException const& e) {
    throw CollectionDatabaseException(strf("Collection '{}' not found", collectionName), e);
  }
}

Collectable CollectionDatabase::collectable(String const& collectionName, String const& collectableName) const {
  try {
    return m_collectables.get(collectionName).get(collectableName);
  } catch (MapException const& e) {
    throw CollectionDatabaseException(strf("Collectable '{}' not found in collection '{}'", collectableName, collectionName), e);
  }
}

bool CollectionDatabase::hasCollectable(String const& collectionName, String const& collectableName) const {
  return (m_collections.contains(collectionName) && m_collectables.get(collectionName).contains(collectableName));
}

Collectable CollectionDatabase::parseGenericCollectable(String const& name, Json const& config) const {
  Collectable collectable;
  collectable.name = name;
  collectable.order = config.getInt("order", 0);

  collectable.title = config.getString("title", "");
  collectable.description = config.getString("description", "");
  collectable.icon = config.getString("icon", "");

  return collectable;
}

Collectable CollectionDatabase::parseMonsterCollectable(String const& name, Json const& config) const {
  Collectable collectable = parseGenericCollectable(name, config);
  auto seed = 0; // use a static seed to utilize caching
  auto variant = m_monsterDatabase->monsterVariant(config.getString("monsterType"), seed);

  collectable.title = variant.shortDescription.value("");
  collectable.description = variant.description.value("");

  return collectable;
}

Collectable CollectionDatabase::parseItemCollectable(String const& name, Json const& config) const {
  Collectable collectable = parseGenericCollectable(name, config);
  auto item = m_itemDatabase->itemShared(ItemDescriptor(config.getString("item")));

  collectable.title = item->friendlyName();
  collectable.description = item->description();

  if (config.contains("icon")) {
    collectable.icon = config.getString("icon");
  } else {
    auto inventoryIcon = item->instanceValue("inventoryIcon", "");
    if (inventoryIcon.isType(Json::Type::String))
      collectable.icon = AssetPath::relativeTo(m_itemDatabase->itemConfig(item->name(), JsonObject()).directory, inventoryIcon.toString());
  }

  return collectable;
}

}
