#pragma once

#include "StarAssets.hpp"
#include "StarDungeonGenerator.hpp"
#include "StarJson.hpp"
#include "StarLexicalCast.hpp"
#include "StarLruCache.hpp"
#include "StarSet.hpp"

namespace Star {

class TilesetDatabase;
using TilesetDatabasePtr = SharedPtr<TilesetDatabase>;
using TilesetDatabaseConstPtr = SharedPtr<TilesetDatabase const>;

namespace Tiled {
  class Tile;
  using TileConstPtr = SharedPtr<Tile const>;
  class Tileset;
  using TilesetConstPtr = SharedPtr<Tileset const>;

  extern EnumMap<TileLayer> const LayerNames;

  // Tiled properties are all String values (due to its original format being
  // XML). This class wraps and converts the String properties into more useful
  // types, parsing them as Json for instance.
  class Properties {
  public:
    Properties() = default;
    Properties(Json const& json);

    [[nodiscard]] Json toJson() const;

    // Returns a new properties set where this properties object overrides
    // the properties parameter.
    [[nodiscard]] Properties inherit(Json const& properties) const;
    [[nodiscard]] Properties inherit(Properties const& properties) const;

    [[nodiscard]] bool contains(String const& name) const;

    template <typename T>
    [[nodiscard]] T get(String const& name) const;

    template <typename T>
    [[nodiscard]] Maybe<T> opt(String const& name) const;

    template <typename T>
    void set(String const& name, T const& value);

  private:
    Json m_properties = JsonObject{};
  };

  class Tile : public Dungeon::Tile {
  public:
    Tile(Properties const& properties, TileLayer layer, bool flipX = false);

    Properties properties;
  };

  class Tileset {
  public:
    Tileset(Json const& json);

    [[nodiscard]] TileConstPtr const& getTile(size_t id, TileLayer layer) const;
    [[nodiscard]] size_t size() const;

  private:
    [[nodiscard]] List<TileConstPtr> const& tiles(TileLayer layer) const;

    List<TileConstPtr> m_tilesBack, m_tilesFront;
  };
}

class TilesetDatabase {
public:
  TilesetDatabase(AssetsConstPtr assets);

  Tiled::TilesetConstPtr get(String const& path) const;

private:
  Tiled::TilesetConstPtr readTileset(String const& path) const;

  AssetsConstPtr m_assets;
  mutable Mutex m_cacheMutex;
  mutable HashLruCache<String, Tiled::TilesetConstPtr> m_tilesetCache;
};

namespace Tiled {
  template <typename T>
  struct PropertyConverter {
    static T to(String const& propertyValue) {
      return lexicalCast<T>(propertyValue);
    }
    static String from(T const& propertyValue) {
      return toString(propertyValue);
    }
  };

  template <>
  struct PropertyConverter<Json> {
    [[nodiscard]] static Json to(String const& propertyValue);
    [[nodiscard]] static String from(Json const& propertyValue);
  };

  template <>
  struct PropertyConverter<String> {
    [[nodiscard]] static String to(String const& propertyValue);
    [[nodiscard]] static String from(String const& propertyValue);
  };

  template <typename T>
  T getProperty(Json const& properties, String const& propertyName) {
    return PropertyConverter<T>::to(properties.get(propertyName).toString());
  }

  template <typename T>
  Maybe<T> optProperty(Json const& properties, String const& propertyName) {
    if (Maybe<String> propertyValue = properties.optString(propertyName))
      return PropertyConverter<T>::to(*propertyValue);
    return {};
  }

  template <typename T>
  Json setProperty(Json const& properties, String const& propertyName, T const& propertyValue) {
    return properties.set(propertyName, PropertyConverter<T>::from(propertyValue));
  }

  template <typename T>
  T Properties::get(String const& name) const {
    return getProperty<T>(m_properties, name);
  }

  template <typename T>
  Maybe<T> Properties::opt(String const& name) const {
    return optProperty<T>(m_properties, name);
  }

  template <typename T>
  void Properties::set(String const& name, T const& propertyValue) {
    m_properties = setProperty(m_properties, name, propertyValue);
  }
}

}
