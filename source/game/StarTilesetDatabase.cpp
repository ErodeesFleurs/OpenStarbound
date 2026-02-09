#include "StarTilesetDatabase.hpp"

#include "StarCasting.hpp"
#include "StarConfig.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

namespace Tiled {
using namespace Dungeon;

EnumMap<TileLayer> const LayerNames{{TileLayer::Foreground, "front"}, {TileLayer::Background, "back"}};

Properties::Properties() : m_properties(JsonObject{}) {}

Properties::Properties(Json const& json) : m_properties(std::move(json)) {}

auto Properties::toJson() const -> Json {
  return m_properties;
}

auto Properties::inherit(Json const& properties) const -> Properties {
  return jsonMerge(properties, m_properties);
}

auto Properties::inherit(Properties const& properties) const -> Properties {
  return jsonMerge(properties.m_properties, m_properties);
}

auto Properties::contains(String const& name) const -> bool {
  return m_properties.contains(name);
}

auto getClearBrush(bool value, Tiled::Properties&) -> std::optional<ConstPtr<Brush>> {
  if (value)
    return as<Brush>(std::make_shared<const ClearBrush>());
  return {};
}

auto getFrontBrush(String const& materialName, Tiled::Properties& properties) -> ConstPtr<Brush> {
  std::optional<float> hueshift = properties.opt<float>("hueshift");
  std::optional<MaterialColorVariant> colorVariant = properties.opt<size_t>("colorVariant");
  std::optional<String> mod = properties.opt<String>("mod");
  std::optional<float> modhueshift = properties.opt<float>("modhueshift");

  return make_shared<const FrontBrush>(materialName, mod, hueshift, modhueshift, colorVariant);
}

auto getBackBrush(String const& materialName, Tiled::Properties& properties) -> ConstPtr<Brush> {
  std::optional<float> hueshift = properties.opt<float>("hueshift");
  std::optional<MaterialColorVariant> colorVariant = properties.opt<size_t>("colorVariant");
  std::optional<String> mod = properties.opt<String>("mod");
  std::optional<float> modhueshift = properties.opt<float>("modhueshift");

  return make_shared<const BackBrush>(materialName, mod, hueshift, modhueshift, colorVariant);
}

auto getMaterialBrush(String const& materialName, Tiled::Properties& properties) -> ConstPtr<Brush> {
  TileLayer layer = LayerNames.getLeft(properties.get<String>("layer"));

  if (layer == TileLayer::Background) {
    return getBackBrush(materialName, properties);
  } else {
    return getFrontBrush(materialName, properties);
  }
}

auto getPlayerStartBrush(String const&, Tiled::Properties&) -> ConstPtr<Brush> {
  return std::make_shared<PlayerStartBrush>();
}

auto getObjectBrush(String const& objectName, Tiled::Properties& properties) -> ConstPtr<Brush> {
  Star::Direction direction = Star::Direction::Right;
  Json parameters;

  if (properties.contains("tilesetDirection"))
    direction = DirectionNames.getLeft(properties.get<String>("tilesetDirection"));
  if (properties.contains("flipX"))
    direction = -direction;

  if (properties.contains("parameters")) {
    parameters = properties.get<Json>("parameters");
  }

  parameters = parameters.opt().value_or(JsonObject{});

  return std::make_shared<const ObjectBrush>(objectName, direction, parameters);
}

auto getVehicleBrush(String const& vehicleName, Tiled::Properties& properties) -> ConstPtr<Brush> {
  Json parameters = JsonObject{};
  if (properties.contains("parameters")) {
    parameters = properties.get<Json>("parameters");
  }
  return std::make_shared<const VehicleBrush>(vehicleName, parameters);
}

auto getWireBrush(String const& group, Tiled::Properties& properties) -> ConstPtr<Brush> {
  bool local = properties.opt<bool>("local").value_or(true);

  return std::make_shared<const WireBrush>(group, local);
}

auto getSeed(Tiled::Properties& properties) -> Json {
  auto seed = properties.get<String>("seed");
  if (seed == "stable")
    return seed;
  return lexicalCast<std::uint64_t>(seed);
}

auto getNpcBrush(String const& species, Tiled::Properties& properties) -> ConstPtr<Brush> {
  JsonObject brush;
  brush["kind"] = "npc";
  brush["species"] = species;// this may be a single species or a comma
  // separated list to be parsed later
  if (properties.contains("seed")) {
    brush["seed"] = getSeed(properties);
  }
  if (properties.contains("typeName"))
    brush["typeName"] = properties.get<String>("typeName");
  brush["parameters"] = properties.opt<Json>("parameters").value_or(JsonObject{});
  return make_shared<const NpcBrush>(brush);
}

auto getMonsterBrush(String const& typeName, Tiled::Properties& properties) -> ConstPtr<Brush> {
  JsonObject brush;
  brush["kind"] = "monster";
  brush["typeName"] = typeName;
  if (properties.contains("seed")) {
    brush["seed"] = getSeed(properties);
  }
  brush["parameters"] = properties.opt<Json>("parameters").value_or(JsonObject{});
  return make_shared<const NpcBrush>(brush);
}

auto getStagehandBrush(String const& typeName, Tiled::Properties& properties) -> ConstPtr<Brush> {
  JsonObject brush;
  brush["type"] = typeName;
  brush["parameters"] = properties.opt<Json>("parameters").value_or(JsonObject{});
  if (properties.contains("broadcastArea"))
    brush["parameters"] = brush["parameters"].set("broadcastArea", properties.get<Json>("broadcastArea"));
  if (typeName == "radiomessage" && properties.contains("radioMessage"))
    brush["parameters"] = brush["parameters"].set("radioMessage", properties.get<Json>("radioMessage"));
  return std::make_shared<const StagehandBrush>(brush);
}

auto getDungeonIdBrush(String const& dungeonId, Tiled::Properties&) -> ConstPtr<Brush> {
  return std::make_shared<const DungeonIdBrush>(maybeLexicalCast<DungeonId>(dungeonId).value_or(NoDungeonId));
}

auto getBiomeItemsBrush(String const&, Tiled::Properties&) -> ConstPtr<Brush> {
  return std::make_shared<const BiomeItemsBrush>();
}

auto getBiomeTreeBrush(String const&, Tiled::Properties&) -> ConstPtr<Brush> {
  return std::make_shared<const BiomeTreeBrush>();
}

auto getItemBrush(String const& itemName, Tiled::Properties& properties) -> ConstPtr<Brush> {
  size_t count = properties.opt<size_t>("count").value_or(1);
  Json parameters = properties.opt<Json>("parameters").value_or(JsonObject{});
  ItemDescriptor item(itemName, count, parameters);
  return std::make_shared<const ItemBrush>(item);
}

auto getSurfaceBrush(String const& variantStr, Tiled::Properties& properties) -> ConstPtr<Brush> {
  TileLayer layer = LayerNames.getLeft(properties.get<String>("layer"));
  std::optional<int> variant = maybeLexicalCast<int>(variantStr);
  std::optional<String> mod = properties.opt<String>("mod");

  if (layer == TileLayer::Background)
    return std::make_shared<const SurfaceBackgroundBrush>(variant, mod);
  return std::make_shared<const SurfaceBrush>(variant, mod);
}

auto getLiquidBrush(String const& liquidName, Tiled::Properties& properties) -> ConstPtr<Brush> {
  float quantity = properties.opt<float>("quantity").value_or(1.0f);
  bool source = properties.opt<bool>("source").value_or(false);
  return std::make_shared<const LiquidBrush>(liquidName, quantity, source);
}

auto getInvalidBrush(bool invalidValue, Tiled::Properties& properties) -> std::optional<ConstPtr<Brush>> {
  if (!invalidValue)
    return {};

  return as<const Brush>(make_shared<const InvalidBrush>(properties.opt<String>("//name")));
}

auto getAirRule(String const&, Tiled::Properties& properties) -> ConstPtr<Rule> {
  TileLayer layer = LayerNames.getLeft(properties.get<String>("layer"));
  return std::make_shared<const WorldGenMustContainAirRule>(layer);
}

auto getSolidRule(String const&, Tiled::Properties& properties) -> ConstPtr<Rule> {
  TileLayer layer = LayerNames.getLeft(properties.get<String>("layer"));
  return std::make_shared<const WorldGenMustContainSolidRule>(layer);
}

auto getLiquidRule(String const&, Tiled::Properties&) -> ConstPtr<Rule> {
  return std::make_shared<const WorldGenMustContainLiquidRule>();
}

auto getNotLiquidRule(String const&, Tiled::Properties&) -> ConstPtr<Rule> {
  return std::make_shared<const WorldGenMustNotContainLiquidRule>();
}

auto getAllowOverdrawingRule(String const&, Tiled::Properties&) -> ConstPtr<Rule> {
  return std::make_shared<const AllowOverdrawingRule>();
}

template <typename T>
class PropertyReader {
public:
  template <typename PropertyType,
            typename GetterReturn = T,
            typename Getter = std::function<GetterReturn(PropertyType, Tiled::Properties&)>>
  void optRead(List<T>& list, String const& propertyName, Getter getter, Tiled::Properties& properties) {
    if (auto propertyValue = read<PropertyType, Getter>(propertyName, getter, properties))
      list.append(std::move(*propertyValue));
  }

private:
  template <typename PropertyType, typename Getter>
  auto read(String const& propertyName, Getter getter, Tiled::Properties& properties) -> std::optional<T> {
    std::optional<PropertyType> propertyValue = properties.opt<PropertyType>(propertyName);
    if (propertyValue.has_value())
      return getter(*propertyValue, properties);
    return {};
  }
};

Tile::Tile(Properties const& tileProperties, TileLayer layer, bool flipX)
    : Dungeon::Tile(), properties(std::move(tileProperties)) {
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

  PropertyReader<ConstPtr<Brush>> br;
  br.optRead<bool, std::optional<ConstPtr<Brush>>>(brushes, "clear", getClearBrush, properties);
  br.optRead<String>(brushes, "material", getMaterialBrush, properties);
  br.optRead<String>(brushes, "front", getFrontBrush, properties);
  br.optRead<String>(brushes, "back", getBackBrush, properties);
  br.optRead<String>(brushes, "playerstart", getPlayerStartBrush, properties);
  br.optRead<String>(brushes, "object", getObjectBrush, properties);
  br.optRead<String>(brushes, "vehicle", getVehicleBrush, properties);
  br.optRead<String>(brushes, "wire", getWireBrush, properties);
  br.optRead<String>(brushes, "npc", getNpcBrush, properties);
  br.optRead<String>(brushes, "monster", getMonsterBrush, properties);
  br.optRead<String>(brushes, "stagehand", getStagehandBrush, properties);
  br.optRead<String>(brushes, "dungeonid", getDungeonIdBrush, properties);
  br.optRead<String>(brushes, "biomeitems", getBiomeItemsBrush, properties);
  br.optRead<String>(brushes, "biometree", getBiomeTreeBrush, properties);
  br.optRead<String>(brushes, "item", getItemBrush, properties);
  br.optRead<String>(brushes, "surface", getSurfaceBrush, properties);
  br.optRead<String>(brushes, "liquid", getLiquidBrush, properties);
  br.optRead<bool, std::optional<ConstPtr<Brush>>>(brushes, "invalid", getInvalidBrush, properties);

  PropertyReader<ConstPtr<Rule>> rr;
  rr.optRead<String>(rules, "worldGenMustContainAir", getAirRule, properties);
  rr.optRead<String>(rules, "worldGenMustContainSolid", getSolidRule, properties);
  rr.optRead<String>(rules, "worldGenMustContainLiquid", getLiquidRule, properties);
  rr.optRead<String>(rules, "worldGenMustNotContainLiquid", getNotLiquidRule, properties);
  rr.optRead<String>(rules, "allowOverdrawing", getAllowOverdrawingRule, properties);

  if (auto connectorName = properties.opt<String>("connector")) {
    auto newConnector = TileConnector();

    newConnector.value = *connectorName;

    auto connectForwardOnly = properties.opt<bool>("connectForwardOnly");
    newConnector.forwardOnly = connectForwardOnly.value_or(false);

    if (auto connectDirection = properties.opt<String>("connectDirection"))
      newConnector.direction = DungeonDirectionNames.getLeft(*connectDirection);

    connector = newConnector;
  }
}

Tileset::Tileset(Json const& json) {
  Properties tilesetProperties(json.opt("properties").value_or(JsonObject{}));
  Json tileProperties = json.opt("tileproperties").value_or(JsonObject{});

  m_tilesBack.resize(json.getInt("tilecount"));
  m_tilesFront.resize(json.getInt("tilecount"));

  for (auto const& entry : tileProperties.iterateObject()) {
    auto index = lexicalCast<size_t>(entry.first);
    Properties properties = Properties(entry.second).inherit(tilesetProperties);

    m_tilesBack[index] = std::make_shared<Tile>(properties, TileLayer::Background);
    m_tilesFront[index] = std::make_shared<Tile>(properties, TileLayer::Foreground);
  }
}

auto Tileset::getTile(size_t id, TileLayer layer) const -> ConstPtr<Tile> const& {
  List<ConstPtr<Tile>> const& tileset = tiles(layer);
  return tileset[id];
}

auto Tileset::size() const -> size_t {
  return m_tilesBack.size();
}

auto Tileset::tiles(TileLayer layer) const -> List<ConstPtr<Tile>> const& {
  if (layer == TileLayer::Background)
    return m_tilesBack;
  return m_tilesFront;
}
}// namespace Tiled

TilesetDatabase::TilesetDatabase() : m_cacheMutex(), m_tilesetCache() {}

auto TilesetDatabase::get(String const& path) const -> ConstPtr<Tiled::Tileset> {
  MutexLocker locker(m_cacheMutex);
  return m_tilesetCache.get(path, TilesetDatabase::readTileset);
}

auto TilesetDatabase::readTileset(String const& path) -> ConstPtr<Tiled::Tileset> {
  auto assets = Root::singleton().assets();
  return std::make_shared<Tiled::Tileset>(assets->json(path));
}

namespace Tiled {
auto PropertyConverter<Json>::to(String const& propertyValue) -> Json {
  try {
    return Json::parseJson(propertyValue);
  } catch (JsonParsingException const& e) {
    throw StarException::format("Error parsing Tiled property as Json: {}", outputException(e, false));
  }
}

auto PropertyConverter<Json>::from(Json const& propertyValue) -> String {
  return propertyValue.repr();
}

auto PropertyConverter<String>::to(String const& propertyValue) -> String {
  return propertyValue;
}

auto PropertyConverter<String>::from(String const& propertyValue) -> String {
  return propertyValue;
}
}// namespace Tiled

}// namespace Star
