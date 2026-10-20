module;

import std;
import star.string;
import star.list;
import star.format;
import star.bi_map;
import star.json;
import star.thread;

namespace Star {

class Json;
class Root;
class ItemDatabase;

struct CollectionDatabaseExceptionTag {
  static constexpr char const* name() { return "CollectionDatabaseException"; }
};
using CollectionDatabaseException = StarError<CollectionDatabaseExceptionTag, StarException>;

enum class CollectionType : std::uint16_t {
  Generic,
  Item,
  Monster
};
extern EnumMap<CollectionType> const CollectionTypeNames;

struct Collectable {
  Collectable();
  Collectable(String const& name, int order, String const& title, String const& description, String const& icon);

  String name;
  int order;
  String title;
  String description;
  String icon;
};

struct Collection {
  Collection();
  Collection(String const& name, CollectionType type, String const& icon);

  String name;
  String title;
  CollectionType type;
};

class CollectionDatabase {
public:
  explicit CollectionDatabase(Root& root);

  List<Collection> collections() const;
  Collection collection(String const& collectionName) const;
  List<Collectable> collectables(String const& collectionName) const;
  List<Collectable> collectables(String const& collectionName, ItemDatabase const* itemCatalog) const;
  Collectable collectable(String const& collectionName, String const& collectableName) const;
  Collectable collectable(String const& collectionName, String const& collectableName, ItemDatabase const* itemCatalog) const;

  bool hasCollectable(String const& collectionName, String const& collectableName) const;

private:
  Collectable parseGenericCollectable(String const& name, Json const& config) const;
  Collectable parseMonsterCollectable(String const& name, Json const& config) const;
  Collectable parseItemCollectable(String const& name, Json const& config, ItemDatabase const* itemCatalog) const;

  Root& m_root;
  StringMap<Collection> m_collections;
  StringMap<StringMap<Json>> m_collectableConfigs;
  mutable Mutex m_collectablesMutex;
  mutable StringMap<StringMap<Collectable>> m_collectables;
};


}

export module star.collection_database;

export namespace Star {
  using ::Star::CollectionDatabaseExceptionTag;
  using ::Star::CollectionDatabaseException;
  using ::Star::CollectionDatabase;
  using ::Star::CollectionType;
  using ::Star::CollectionTypeNames;
  using ::Star::Collectable;
  using ::Star::Collection;
}
