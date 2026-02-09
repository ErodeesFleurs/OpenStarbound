#pragma once

#include "StarConfig.hpp"
#include "StarDungeonGenerator.hpp"
#include "StarJson.hpp"
#include "StarLexicalCast.hpp"
#include "StarLruCache.hpp"

import std;

namespace Star {

namespace Tiled {

extern EnumMap<TileLayer> const LayerNames;

// Tiled properties are all String values (due to its original format being
// XML). This class wraps and converts the String properties into more useful
// types, parsing them as Json for instance.
class Properties {
public:
  Properties();
  Properties(Json const& json);

  [[nodiscard]] auto toJson() const -> Json;

  // Returns a new properties set where this properties object overrides
  // the properties parameter.
  [[nodiscard]] auto inherit(Json const& properties) const -> Properties;
  [[nodiscard]] auto inherit(Properties const& properties) const -> Properties;

  [[nodiscard]] auto contains(String const& name) const -> bool;

  template <typename T>
  auto get(String const& name) const -> T;

  template <typename T>
  auto opt(String const& name) const -> std::optional<T>;

  template <typename T>
  void set(String const& name, T const& value);

private:
  Json m_properties;
};

class Tile : public Dungeon::Tile {
public:
  Tile(Properties const& properties, TileLayer layer, bool flipX = false);

  Properties properties;
};

class Tileset {
public:
  Tileset(Json const& json);

  [[nodiscard]] auto getTile(std::size_t id, TileLayer layer) const -> ConstPtr<Tile> const&;
  [[nodiscard]] auto size() const -> std::size_t;

private:
  [[nodiscard]] auto tiles(TileLayer layer) const -> List<ConstPtr<Tile>> const&;

  List<ConstPtr<Tile>> m_tilesBack, m_tilesFront;
};
}// namespace Tiled

class TilesetDatabase {
public:
  TilesetDatabase();

  auto get(String const& path) const -> ConstPtr<Tiled::Tileset>;

private:
  static auto readTileset(String const& path) -> ConstPtr<Tiled::Tileset>;

  mutable Mutex m_cacheMutex;
  mutable HashLruCache<String, ConstPtr<Tiled::Tileset>> m_tilesetCache;
};

namespace Tiled {
template <typename T>
struct PropertyConverter {
  static auto to(String const& propertyValue) -> T {
    return lexicalCast<T>(propertyValue);
  }
  static auto from(T const& propertyValue) -> String {
    return toString(propertyValue);
  }
};

template <>
struct PropertyConverter<Json> {
  static auto to(String const& propertyValue) -> Json;
  static auto from(Json const& propertyValue) -> String;
};

template <>
struct PropertyConverter<String> {
  static auto to(String const& propertyValue) -> String;
  static auto from(String const& propertyValue) -> String;
};

template <typename T>
auto getProperty(Json const& properties, String const& propertyName) -> T {
  return PropertyConverter<T>::to(properties.get(propertyName).toString());
}

template <typename T>
auto optProperty(Json const& properties, String const& propertyName) -> std::optional<T> {
  if (auto propertyValue = properties.optString(propertyName))
    return PropertyConverter<T>::to(*propertyValue);
  return std::nullopt;
}

template <typename T>
auto setProperty(Json const& properties, String const& propertyName, T const& propertyValue) -> Json {
  return properties.set(propertyName, PropertyConverter<T>::from(propertyValue));
}

template <typename T>
auto Properties::get(String const& name) const -> T {
  return getProperty<T>(m_properties, name);
}

template <typename T>
auto Properties::opt(String const& name) const -> std::optional<T> {
  return optProperty<T>(m_properties, name);
}

template <typename T>
void Properties::set(String const& name, T const& propertyValue) {
  m_properties = setProperty(m_properties, name, propertyValue);
}
}// namespace Tiled

}// namespace Star
