module;
import std;
import star.string;
import star.list;
import star.format;
import star.maybe;
import star.json;
import star.image;
import star.data_stream;
import star.bi_map;
import star.game_types;
import star.random;
import star.thread;
import star.lexical_cast;
import star.ordered_map;
import star.lru_cache;
import star.set;


import star.item_descriptor;
import star.world_geometry;
import star.dungeon_generator;

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

    Json toJson() const;

    // Returns a new properties set where this properties object overrides
    // the properties parameter.
    Properties inherit(Json const& properties) const;
    Properties inherit(Properties const& properties) const;

    bool contains(String const& name) const;

    template <typename T>
    T get(String const& name) const;

    template <typename T>
    Maybe<T> opt(String const& name) const;

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

    std::shared_ptr<Tile const> const& getTile(std::size_t id, TileLayer layer) const;
    std::size_t size() const;

  private:
    List<std::shared_ptr<Tile const>> const& tiles(TileLayer layer) const;

    List<std::shared_ptr<Tile const>> m_tilesBack, m_tilesFront;
  };
}

class TilesetDatabase {
public:
  TilesetDatabase();

  std::shared_ptr<Tiled::Tileset const> get(String const& path) const;

private:
  static std::shared_ptr<Tiled::Tileset const> readTileset(String const& path);

  mutable Mutex m_cacheMutex;
  mutable HashLruCache<String, std::shared_ptr<Tiled::Tileset const>> m_tilesetCache;
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
    static Json to(String const& propertyValue);
    static String from(Json const& propertyValue);
  };

  template <>
  struct PropertyConverter<String> {
    static String to(String const& propertyValue);
    static String from(String const& propertyValue);
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

export module star.tileset_database;

export namespace Star {
  using ::Star::TilesetDatabase;
}

export namespace Star::Tiled {
  using ::Star::Tiled::LayerNames;
  using ::Star::Tiled::Properties;
  using ::Star::Tiled::Tile;
  using ::Star::Tiled::Tileset;
  using ::Star::Tiled::PropertyConverter;
  using ::Star::Tiled::getProperty;
  using ::Star::Tiled::optProperty;
  using ::Star::Tiled::setProperty;
}
