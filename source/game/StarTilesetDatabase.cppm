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

class TilesetDatabase;
using TilesetDatabasePtr = std::shared_ptr<TilesetDatabase>;
using TilesetDatabaseConstPtr = std::shared_ptr<const TilesetDatabase>;
using TilesetDatabaseWeakPtr = std::weak_ptr<TilesetDatabase>;
using TilesetDatabaseConstWeakPtr = std::weak_ptr<const TilesetDatabase>;
using TilesetDatabaseUPtr = std::unique_ptr<TilesetDatabase>;
using TilesetDatabaseConstUPtr = std::unique_ptr<const TilesetDatabase>;

namespace Tiled {
  class Tile;
  using TilePtr = std::shared_ptr<Tile>;
  using TileConstPtr = std::shared_ptr<const Tile>;
  using TileWeakPtr = std::weak_ptr<Tile>;
  using TileConstWeakPtr = std::weak_ptr<const Tile>;
  using TileUPtr = std::unique_ptr<Tile>;
  using TileConstUPtr = std::unique_ptr<const Tile>;
  class Tileset;
  using TilesetPtr = std::shared_ptr<Tileset>;
  using TilesetConstPtr = std::shared_ptr<const Tileset>;
  using TilesetWeakPtr = std::weak_ptr<Tileset>;
  using TilesetConstWeakPtr = std::weak_ptr<const Tileset>;
  using TilesetUPtr = std::unique_ptr<Tileset>;
  using TilesetConstUPtr = std::unique_ptr<const Tileset>;

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

    TileConstPtr const& getTile(std::size_t id, TileLayer layer) const;
    std::size_t size() const;

  private:
    List<TileConstPtr> const& tiles(TileLayer layer) const;

    List<TileConstPtr> m_tilesBack, m_tilesFront;
  };
}

class TilesetDatabase {
public:
  TilesetDatabase();

  Tiled::TilesetConstPtr get(String const& path) const;

private:
  static Tiled::TilesetConstPtr readTileset(String const& path);

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
  using ::Star::TilesetDatabasePtr;
  using ::Star::TilesetDatabaseConstPtr;
  using ::Star::TilesetDatabaseWeakPtr;
  using ::Star::TilesetDatabaseConstWeakPtr;
  using ::Star::TilesetDatabaseUPtr;
  using ::Star::TilesetDatabaseConstUPtr;
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
  using ::Star::Tiled::TilePtr;
  using ::Star::Tiled::TileConstPtr;
  using ::Star::Tiled::TileWeakPtr;
  using ::Star::Tiled::TileConstWeakPtr;
  using ::Star::Tiled::TileUPtr;
  using ::Star::Tiled::TileConstUPtr;
  using ::Star::Tiled::TilesetPtr;
  using ::Star::Tiled::TilesetConstPtr;
  using ::Star::Tiled::TilesetWeakPtr;
  using ::Star::Tiled::TilesetConstWeakPtr;
  using ::Star::Tiled::TilesetUPtr;
  using ::Star::Tiled::TilesetConstUPtr;
}
