#pragma once

#include "StarAssets.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"

namespace Star {

struct CollectionDatabaseExceptionTag { static constexpr char const* typeName = "CollectionDatabaseException"; };
using CollectionDatabaseException = TypedException<StarException, CollectionDatabaseExceptionTag>;

class CollectionDatabase;
using CollectionDatabasePtr = SharedPtr<CollectionDatabase>;
using CollectionDatabaseConstPtr = SharedPtr<CollectionDatabase const>;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class MonsterDatabase;
using MonsterDatabaseConstPtr = SharedPtr<MonsterDatabase const>;

enum class CollectionType : uint16_t {
  Generic,
  Item,
  Monster
};
extern EnumMap<CollectionType> const CollectionTypeNames;

struct Collectable {
  Collectable() = default;
  Collectable(String const& name, int order, String const& title, String const& description, String const& icon);

  String name;
  int order = 0;
  String title;
  String description;
  String icon;
};

struct Collection {
  Collection() = default;
  Collection(String const& name, CollectionType type, String const& icon);

  String name;
  String title;
  CollectionType type = CollectionType::Generic;
};

class CollectionDatabase {
public:
  CollectionDatabase(AssetsConstPtr assets, MonsterDatabaseConstPtr monsterDatabase, ItemDatabaseConstPtr itemDatabase);

  List<Collection> collections() const;
  Collection collection(String const& collectionName) const;
  List<Collectable> collectables(String const& collectionName) const;
  Collectable collectable(String const& collectionName, String const& collectableName) const;

  bool hasCollectable(String const& collectionName, String const& collectableName) const;

private:
  Collectable parseGenericCollectable(String const& name, Json const& config) const;
  Collectable parseMonsterCollectable(String const& name, Json const& config) const;
  Collectable parseItemCollectable(String const& name, Json const& config) const;

  MonsterDatabaseConstPtr m_monsterDatabase;
  ItemDatabaseConstPtr m_itemDatabase;
  StringMap<Collection> m_collections;
  StringMap<StringMap<Collectable>> m_collectables;
};


}
