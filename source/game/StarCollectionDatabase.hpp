#pragma once

#include "StarBiMap.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

using CollectionDatabaseException = ExceptionDerived<"CollectionDatabaseException">;

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
  CollectionDatabase();

  [[nodiscard]] auto collections() const -> List<Collection>;
  [[nodiscard]] auto collection(String const& collectionName) const -> Collection;
  [[nodiscard]] auto collectables(String const& collectionName) const -> List<Collectable>;
  [[nodiscard]] auto collectable(String const& collectionName, String const& collectableName) const -> Collectable;

  [[nodiscard]] auto hasCollectable(String const& collectionName, String const& collectableName) const -> bool;

private:
  [[nodiscard]] auto parseGenericCollectable(String const& name, Json const& config) const -> Collectable;
  [[nodiscard]] auto parseMonsterCollectable(String const& name, Json const& config) const -> Collectable;
  [[nodiscard]] auto parseItemCollectable(String const& name, Json const& config) const -> Collectable;

  StringMap<Collection> m_collections;
  StringMap<StringMap<Collectable>> m_collectables;
};

}// namespace Star
