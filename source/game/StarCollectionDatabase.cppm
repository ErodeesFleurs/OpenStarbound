module;

#include "StarBiMap.hpp"

namespace Star {

class Json;

struct CollectionDatabaseExceptionTag {
  static constexpr char const* name() { return "CollectionDatabaseException"; }
};
using CollectionDatabaseException = StarError<CollectionDatabaseExceptionTag, StarException>;

STAR_CLASS(CollectionDatabase);

enum class CollectionType : uint16_t {
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
  CollectionDatabase();

  List<Collection> collections() const;
  Collection collection(String const& collectionName) const;
  List<Collectable> collectables(String const& collectionName) const;
  Collectable collectable(String const& collectionName, String const& collectableName) const;

  bool hasCollectable(String const& collectionName, String const& collectableName) const;

private:
  Collectable parseGenericCollectable(String const& name, Json const& config) const;
  Collectable parseMonsterCollectable(String const& name, Json const& config) const;
  Collectable parseItemCollectable(String const& name, Json const& config) const;

  StringMap<Collection> m_collections;
  StringMap<StringMap<Collectable>> m_collectables;
};


}

export module star.collection_database;

export namespace Star {
  using ::Star::CollectionDatabaseExceptionTag;
  using ::Star::CollectionDatabaseException;
  using ::Star::CollectionDatabase;
  using ::Star::CollectionDatabasePtr;
  using ::Star::CollectionDatabaseConstPtr;
  using ::Star::CollectionType;
  using ::Star::CollectionTypeNames;
  using ::Star::Collectable;
  using ::Star::Collection;
}
