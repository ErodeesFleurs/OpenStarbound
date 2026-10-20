import star.json;
import star.set;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.strong_typedef;
import star.bi_map;
import star.game_types;
import star.game_types;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.map;
import star.format;
import star.version;

import star.drawable;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;
import star.item_descriptor;
import star.item_recipe;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.collection_database;
import star.monster_database;
import star.item_database;


namespace Star {

extern EnumMap<CollectionType> const CollectionTypeNames {
  {CollectionType::Generic, "generic"},
  {CollectionType::Item, "item"},
  {CollectionType::Monster, "monster"}
};

Collection::Collection() : name(), title(), type() {}

Collection::Collection(String const& name, CollectionType type, String const& title) : name(name), title(title), type(type) {}

Collectable::Collectable() : name(), order(), title(), description(), icon() {}

Collectable::Collectable(String const& name, int order, String const& title, String const& description, String const& icon)
  : name(name), order(order), title(title), description(description), icon(icon) {};

CollectionDatabase::CollectionDatabase(Root& root) : m_root(root) {
  auto assets = m_root.assets();
  auto& files = assets->scanExtension("collection");
  assets->queueJsons(files);
  for (auto& file : files) {
    auto config = assets->json(file);

    Collection collection;

    collection.name = config.getString("name");
    collection.title = config.getString("title", collection.name);
    collection.type = CollectionTypeNames.getLeft(config.getString("type", "generic"));

    m_collectables[collection.name] = {};
    m_collectableConfigs[collection.name] = {};
    for (auto pair : config.get("collectables").iterateObject()) {
      m_collectableConfigs[collection.name][pair.first] = pair.second;
      if (collection.type == CollectionType::Generic)
        m_collectables[collection.name][pair.first] = parseGenericCollectable(pair.first, pair.second);
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
  return collectables(collectionName, nullptr);
}

List<Collectable> CollectionDatabase::collectables(String const& collectionName, ItemDatabase const* itemCatalog) const {
  try {
    List<Collectable> result;
    auto const& configs = m_collectableConfigs.get(collectionName);
    result.reserve(configs.size());
    for (auto const& entry : configs)
      result.append(collectable(collectionName, entry.first, itemCatalog));
    return result;
  } catch (MapException const& e) {
    throw CollectionDatabaseException(strf("Collection '{}' not found", collectionName), e);
  }
}

Collectable CollectionDatabase::collectable(String const& collectionName, String const& collectableName) const {
  return collectable(collectionName, collectableName, nullptr);
}

Collectable CollectionDatabase::collectable(String const& collectionName, String const& collectableName, ItemDatabase const* itemCatalog) const {
  try {
    auto const& config = m_collectableConfigs.get(collectionName).get(collectableName);
    {
      MutexLocker locker(m_collectablesMutex);
      if (auto cached = m_collectables.get(collectionName).maybe(collectableName))
        return *cached;
    }
    Collectable result;
    switch (m_collections.get(collectionName).type) {
      case CollectionType::Generic:
        result = parseGenericCollectable(collectableName, config);
        break;
      case CollectionType::Monster:
        result = parseMonsterCollectable(collectableName, config);
        break;
      case CollectionType::Item:
        result = parseItemCollectable(collectableName, config, itemCatalog);
        break;
    }
    MutexLocker locker(m_collectablesMutex);
    if (auto cached = m_collectables.get(collectionName).maybe(collectableName))
      return *cached;
    m_collectables[collectionName][collectableName] = result;
    return result;
  } catch (MapException const& e) {
    throw CollectionDatabaseException(strf("Collectable '{}' not found in collection '{}'", collectableName, collectionName), e);
  }
}

bool CollectionDatabase::hasCollectable(String const& collectionName, String const& collectableName) const {
  return (m_collections.contains(collectionName) && m_collectableConfigs.get(collectionName).contains(collectableName));
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
  auto variant = m_root.monsterDatabase()->monsterVariant(config.getString("monsterType"), seed);

  collectable.title = variant.shortDescription.value("");
  collectable.description = variant.description.value("");

  return collectable;
}

Collectable CollectionDatabase::parseItemCollectable(String const& name, Json const& config, ItemDatabase const* itemCatalog) const {
  Collectable collectable = parseGenericCollectable(name, config);
  auto catalogOwner = itemCatalog ? std::shared_ptr<ItemDatabase const>{} : m_root.itemDatabase();
  if (!itemCatalog)
    itemCatalog = catalogOwner.get();
  auto item = itemCatalog->itemShared(ItemDescriptor(config.getString("item")));

  collectable.title = item->friendlyName();
  collectable.description = item->description();

  if (config.contains("icon")) {
    collectable.icon = config.getString("icon");
  } else {
    auto inventoryIcon = item->instanceValue("inventoryIcon", "");
    if (inventoryIcon.isType(Json::Type::String))
      collectable.icon = AssetPath::relativeTo(itemCatalog->itemConfig(item->name(), JsonObject()).directory, inventoryIcon.toString());
  }

  return collectable;
}

}
