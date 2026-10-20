import star.json;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.casting;
import star.logging;
import star.listener;
import std;
import star.string;
import star.maybe;
import star.format;
import star.version;
import star.image;
import star.game_types;
import star.material_types;
import star.random;
import star.lexical_cast;
import star.lru_cache;
import star.set;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.item_descriptor;
import star.world_geometry;
import star.dungeon_generator;

import star.tileset_database;

namespace Star {

namespace Tiled {
  using namespace Dungeon;

  extern EnumMap<TileLayer> const LayerNames{{TileLayer::Foreground, "front"}, {TileLayer::Background, "back"}};

  Properties::Properties() : m_properties(JsonObject{}) {}

  Properties::Properties(Json const& json) : m_properties(json) {}

  Json Properties::toJson() const {
    return m_properties;
  }

  Properties Properties::inherit(Json const& properties) const {
    return jsonMerge(properties, m_properties);
  }

  Properties Properties::inherit(Properties const& properties) const {
    return jsonMerge(properties.m_properties, m_properties);
  }

  bool Properties::contains(String const& name) const {
    return m_properties.contains(name);
  }

  Maybe<std::shared_ptr<Dungeon::Brush const>> getClearBrush(bool value, Tiled::Properties&) {
    if (value)
      return Maybe<std::shared_ptr<Dungeon::Brush const>>(as<Brush>(std::make_shared<const ClearBrush>()));
    return {};
  }

  std::shared_ptr<Dungeon::Brush const> getFrontBrush(String const& materialName, Tiled::Properties& properties) {
    Maybe<float> hueshift = properties.opt<float>("hueshift");
    Maybe<MaterialColorVariant> colorVariant = properties.opt<std::size_t>("colorVariant");
    Maybe<String> mod = properties.opt<String>("mod");
    Maybe<float> modhueshift = properties.opt<float>("modhueshift");

    return std::make_shared<const FrontBrush>(materialName, mod, hueshift, modhueshift, colorVariant);
  }

  std::shared_ptr<Dungeon::Brush const> getBackBrush(String const& materialName, Tiled::Properties& properties) {
    Maybe<float> hueshift = properties.opt<float>("hueshift");
    Maybe<MaterialColorVariant> colorVariant = properties.opt<std::size_t>("colorVariant");
    Maybe<String> mod = properties.opt<String>("mod");
    Maybe<float> modhueshift = properties.opt<float>("modhueshift");

    return std::make_shared<const BackBrush>(materialName, mod, hueshift, modhueshift, colorVariant);
  }

  std::shared_ptr<Dungeon::Brush const> getMaterialBrush(String const& materialName, Tiled::Properties& properties) {
    TileLayer layer = LayerNames.getLeft(properties.get<String>("layer"));

    if (layer == TileLayer::Background) {
      return getBackBrush(materialName, properties);
    } else {
      if constexpr (Star::DebugEnabled) {
        Star::starAssertFailed((layer == TileLayer::Foreground));
      }
      return getFrontBrush(materialName, properties);
    }
  }

  std::shared_ptr<Dungeon::Brush const> getPlayerStartBrush(String const&, Tiled::Properties&) {
    return std::make_shared<PlayerStartBrush>();
  }

  std::shared_ptr<Dungeon::Brush const> getObjectBrush(String const& objectName, Tiled::Properties& properties) {
    Star::Direction direction = Star::Direction::Right;
    Json parameters;

    if (properties.contains("tilesetDirection"))
      direction = DirectionNames.getLeft(properties.get<String>("tilesetDirection"));
    if (properties.contains("flipX"))
      direction = -direction;

    if (properties.contains("parameters")) {
      parameters = properties.get<Json>("parameters");
    }

    parameters = parameters.opt().value(JsonObject{});

    return std::make_shared<const ObjectBrush>(objectName, direction, parameters);
  }

  std::shared_ptr<Dungeon::Brush const> getVehicleBrush(String const& vehicleName, Tiled::Properties& properties) {
    Json parameters = JsonObject{};
    if (properties.contains("parameters")) {
      parameters = properties.get<Json>("parameters");
    }
    return std::make_shared<const VehicleBrush>(vehicleName, parameters);
  }

  std::shared_ptr<Dungeon::Brush const> getWireBrush(String const& group, Tiled::Properties& properties) {
    bool local = properties.opt<bool>("local").value(true);

    return std::make_shared<const WireBrush>(group, local);
  }

  Json getSeed(Tiled::Properties& properties) {
    String seed = properties.get<String>("seed");
    if (seed == "stable")
      return seed;
    return lexicalCast<std::uint64_t>(seed);
  }

  std::shared_ptr<Dungeon::Brush const> getNpcBrush(Root& root, String const& species, Tiled::Properties& properties) {
    JsonObject brush;
    brush["kind"] = "npc";
    brush["species"] = species; // this may be a single species or a comma
    // separated list to be parsed later
    if (properties.contains("seed")) {
      brush["seed"] = getSeed(properties);
    }
    if (properties.contains("typeName"))
      brush["typeName"] = properties.get<String>("typeName");
    brush["parameters"] = properties.opt<Json>("parameters").value(JsonObject{});
    return std::make_shared<const NpcBrush>(root, brush);
  }

  std::shared_ptr<Dungeon::Brush const> getMonsterBrush(Root& root, String const& typeName, Tiled::Properties& properties) {
    JsonObject brush;
    brush["kind"] = "monster";
    brush["typeName"] = typeName;
    if (properties.contains("seed")) {
      brush["seed"] = getSeed(properties);
    }
    brush["parameters"] = properties.opt<Json>("parameters").value(JsonObject{});
    return std::make_shared<const NpcBrush>(root, brush);
  }

  std::shared_ptr<Dungeon::Brush const> getStagehandBrush(String const& typeName, Tiled::Properties& properties) {
    JsonObject brush;
    brush["type"] = typeName;
    brush["parameters"] = properties.opt<Json>("parameters").value(JsonObject{});
    if (properties.contains("broadcastArea"))
      brush["parameters"] = brush["parameters"].set("broadcastArea", properties.get<Json>("broadcastArea"));
    if (typeName == "radiomessage" && properties.contains("radioMessage"))
      brush["parameters"] = brush["parameters"].set("radioMessage", properties.get<Json>("radioMessage"));
    return std::make_shared<const StagehandBrush>(brush);
  }

  std::shared_ptr<Dungeon::Brush const> getDungeonIdBrush(String const& dungeonId, Tiled::Properties&) {
    return std::make_shared<const DungeonIdBrush>(maybeLexicalCast<DungeonId>(dungeonId).value(NoDungeonId));
  }

  std::shared_ptr<Dungeon::Brush const> getBiomeItemsBrush(String const&, Tiled::Properties&) {
    return std::make_shared<const BiomeItemsBrush>();
  }

  std::shared_ptr<Dungeon::Brush const> getBiomeTreeBrush(String const&, Tiled::Properties&) {
    return std::make_shared<const BiomeTreeBrush>();
  }

  std::shared_ptr<Dungeon::Brush const> getItemBrush(String const& itemName, Tiled::Properties& properties) {
    std::size_t count = properties.opt<std::size_t>("count").value(1);
    Json parameters = properties.opt<Json>("parameters").value(JsonObject{});
    ItemDescriptor item(itemName, count, parameters);
    return std::make_shared<const ItemBrush>(item);
  }

  std::shared_ptr<Dungeon::Brush const> getSurfaceBrush(String const& variantStr, Tiled::Properties& properties) {
    TileLayer layer = LayerNames.getLeft(properties.get<String>("layer"));
    Maybe<int> variant = maybeLexicalCast<int>(variantStr);
    Maybe<String> mod = properties.opt<String>("mod");

    if (layer == TileLayer::Background)
      return std::make_shared<const SurfaceBackgroundBrush>(variant, mod);
    return std::make_shared<const SurfaceBrush>(variant, mod);
  }

  std::shared_ptr<Dungeon::Brush const> getLiquidBrush(String const& liquidName, Tiled::Properties& properties) {
    float quantity = properties.opt<float>("quantity").value(1.0f);
    bool source = properties.opt<bool>("source").value(false);
    return std::make_shared<const LiquidBrush>(liquidName, quantity, source);
  }

  Maybe<std::shared_ptr<Dungeon::Brush const>> getInvalidBrush(bool invalidValue, Tiled::Properties& properties) {
    if (!invalidValue)
      return {};

    return as<const Brush>(std::make_shared<const InvalidBrush>(properties.opt<String>("//name")));
  }

  std::shared_ptr<Dungeon::Rule const> getAirRule(String const&, Tiled::Properties& properties) {
    TileLayer layer = LayerNames.getLeft(properties.get<String>("layer"));
    return std::make_shared<const WorldGenMustContainAirRule>(layer);
  }

  std::shared_ptr<Dungeon::Rule const> getSolidRule(String const&, Tiled::Properties& properties) {
    TileLayer layer = LayerNames.getLeft(properties.get<String>("layer"));
    return std::make_shared<const WorldGenMustContainSolidRule>(layer);
  }

  std::shared_ptr<Dungeon::Rule const> getLiquidRule(String const&, Tiled::Properties&) {
    return std::make_shared<const WorldGenMustContainLiquidRule>();
  }

  std::shared_ptr<Dungeon::Rule const> getNotLiquidRule(String const&, Tiled::Properties&) {
    return std::make_shared<const WorldGenMustNotContainLiquidRule>();
  }

  std::shared_ptr<Dungeon::Rule const> getAllowOverdrawingRule(String const&, Tiled::Properties&) {
    return std::make_shared<const AllowOverdrawingRule>();
  }

  template <typename T>
  class PropertyReader {
  public:
    template <typename PropertyType,
        typename GetterReturn = T,
        typename Getter = std::function<GetterReturn(PropertyType, Tiled::Properties&)>>
    void optRead(List<T>& list, String const& propertyName, Getter getter, Tiled::Properties& properties) {
      auto appendFn = std::bind(&List<T>::append, &list, std::placeholders::_1);
      read<PropertyType, Getter>(propertyName, getter, properties).exec(appendFn);
    }

  private:
    template <typename PropertyType, typename Getter>
    Maybe<T> read(String const& propertyName, Getter getter, Tiled::Properties& properties) {
      Maybe<PropertyType> propertyValue = properties.opt<PropertyType>(propertyName);
      if (propertyValue.isValid())
        return getter(*propertyValue, properties);
      return {};
    }
  };

  Tile::Tile(Root& root, Properties const& tileProperties, TileLayer layer, bool flipX)
    : Dungeon::Tile(), properties(tileProperties) {
    JsonObject computedProperties;
    if (!properties.contains("layer")) {
      computedProperties["layer"] = LayerNames.getRight(layer);
    } else {
      layer = LayerNames.getLeft(properties.get<String>("layer"));
    }

    if (flipX)
      computedProperties["flipX"] = "true";

    if (layer == TileLayer::Background && !properties.contains("clear"))
      // The magic pink tile/brush has the clear property set to "false". All
      // other tiles default to clear="true".
      computedProperties["clear"] = "true";

    properties = properties.inherit(computedProperties);

    PropertyReader<std::shared_ptr<Dungeon::Brush const>> br;
    br.optRead<bool, Maybe<std::shared_ptr<Dungeon::Brush const>>>(brushes, "clear", getClearBrush, properties);
    br.optRead<String>(brushes, "material", getMaterialBrush, properties);
    br.optRead<String>(brushes, "front", getFrontBrush, properties);
    br.optRead<String>(brushes, "back", getBackBrush, properties);
    br.optRead<String>(brushes, "playerstart", getPlayerStartBrush, properties);
    br.optRead<String>(brushes, "object", getObjectBrush, properties);
    br.optRead<String>(brushes, "vehicle", getVehicleBrush, properties);
    br.optRead<String>(brushes, "wire", getWireBrush, properties);
    br.optRead<String>(brushes, "npc", std::bind_front(getNpcBrush, std::ref(root)), properties);
    br.optRead<String>(brushes, "monster", std::bind_front(getMonsterBrush, std::ref(root)), properties);
    br.optRead<String>(brushes, "stagehand", getStagehandBrush, properties);
    br.optRead<String>(brushes, "dungeonid", getDungeonIdBrush, properties);
    br.optRead<String>(brushes, "biomeitems", getBiomeItemsBrush, properties);
    br.optRead<String>(brushes, "biometree", getBiomeTreeBrush, properties);
    br.optRead<String>(brushes, "item", getItemBrush, properties);
    br.optRead<String>(brushes, "surface", getSurfaceBrush, properties);
    br.optRead<String>(brushes, "liquid", getLiquidBrush, properties);
    br.optRead<bool, Maybe<std::shared_ptr<Dungeon::Brush const>>>(brushes, "invalid", getInvalidBrush, properties);

    PropertyReader<std::shared_ptr<Dungeon::Rule const>> rr;
    rr.optRead<String>(rules, "worldGenMustContainAir", getAirRule, properties);
    rr.optRead<String>(rules, "worldGenMustContainSolid", getSolidRule, properties);
    rr.optRead<String>(rules, "worldGenMustContainLiquid", getLiquidRule, properties);
    rr.optRead<String>(rules, "worldGenMustNotContainLiquid", getNotLiquidRule, properties);
    rr.optRead<String>(rules, "allowOverdrawing", getAllowOverdrawingRule, properties);

    if (auto connectorName = properties.opt<String>("connector")) {
      auto newConnector = TileConnector();

      newConnector.value = *connectorName;

      auto connectForwardOnly = properties.opt<bool>("connectForwardOnly");
      newConnector.forwardOnly = connectForwardOnly.value(false);

      if (auto connectDirection = properties.opt<String>("connectDirection"))
        newConnector.direction = DungeonDirectionNames.getLeft(*connectDirection);

      connector = newConnector;
    }
  }

  Tileset::Tileset(Root& root, Json const& json) {
    Properties tilesetProperties(json.opt("properties").value(JsonObject{}));
    Json tileProperties = json.opt("tileproperties").value(JsonObject{});

    m_tilesBack.resize(json.getInt("tilecount"));
    m_tilesFront.resize(json.getInt("tilecount"));

    for (auto const& entry : tileProperties.iterateObject()) {
      std::size_t index = lexicalCast<std::size_t>(entry.first);
      Properties properties = Properties(entry.second).inherit(tilesetProperties);

      m_tilesBack[index] = std::make_shared<Tile>(root, properties, TileLayer::Background, false);
      m_tilesFront[index] = std::make_shared<Tile>(root, properties, TileLayer::Foreground, false);
    }
  }

  std::shared_ptr<Tile const> const& Tileset::getTile(std::size_t id, TileLayer layer) const {
    List<std::shared_ptr<Tile const>> const& tileset = tiles(layer);
    return tileset[id];
  }

  std::size_t Tileset::size() const {
    if constexpr (Star::DebugEnabled) {
      Star::starAssertFailed((m_tilesBack.size() == m_tilesFront.size()));
    }
    return m_tilesBack.size();
  }

  List<std::shared_ptr<Tile const>> const& Tileset::tiles(TileLayer layer) const {
    if (layer == TileLayer::Background)
      return m_tilesBack;
    if constexpr (Star::DebugEnabled) {
      Star::starAssertFailed((layer == TileLayer::Foreground));
    }
    return m_tilesFront;
  }
}

TilesetDatabase::TilesetDatabase(Root& root) : m_root(root), m_cacheMutex(), m_tilesetCache() {}

std::shared_ptr<Tiled::Tileset const> TilesetDatabase::get(String const& path) const {
  MutexLocker locker(m_cacheMutex);
  return m_tilesetCache.get(path, [this](String const& path) { return readTileset(path); });
}

std::shared_ptr<Tiled::Tileset const> TilesetDatabase::readTileset(String const& path) const {
  auto assets = m_root.assets();
  return std::make_shared<Tiled::Tileset>(m_root, assets->json(path));
}

namespace Tiled {
  Json PropertyConverter<Json>::to(String const& propertyValue) {
    try {
      return Json::parseJson(propertyValue);
    } catch (JsonParsingException const& e) {
      throw StarException::format("Error parsing Tiled property as Json: {}", outputException(e, false));
    }
  }

  String PropertyConverter<Json>::from(Json const& propertyValue) {
    return propertyValue.repr();
  }

  String PropertyConverter<String>::to(String const& propertyValue) {
    return propertyValue;
  }

  String PropertyConverter<String>::from(String const& propertyValue) {
    return propertyValue;
  }
}

}
