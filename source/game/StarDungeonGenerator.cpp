#include "StarDungeonGenerator.hpp"

#include "StarCasting.hpp"
#include "StarDungeonImagePart.hpp"
#include "StarDungeonTMXPart.hpp"
#include "StarJsonExtra.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLogging.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarRandom.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

size_t const DefinitionsCacheSize = 20;

namespace Dungeon {

EnumMap<Dungeon::Direction> const DungeonDirectionNames{
  {Dungeon::Direction::Left, "left"},
  {Dungeon::Direction::Right, "right"},
  {Dungeon::Direction::Up, "up"},
  {Dungeon::Direction::Down, "down"},
  {Dungeon::Direction::Unknown, "unknown"},
  {Dungeon::Direction::Any, "any"},
};

auto flipDirection(Direction direction) -> Direction {
  if (direction == Direction::Left)
    return Direction::Right;
  if (direction == Direction::Right)
    return Direction::Left;
  if (direction == Direction::Up)
    return Direction::Down;
  if (direction == Direction::Down)
    return Direction::Up;
  if (direction == Direction::Any)
    return Direction::Any;
  throw DungeonException("Invalid direction");
}

auto biomeMaterialForJson(int variant) -> MaterialId {
  if (variant == 0)
    return BiomeMaterialId;
  if (variant == 1)
    return Biome1MaterialId;
  if (variant == 2)
    return Biome2MaterialId;
  if (variant == 3)
    return Biome3MaterialId;
  if (variant == 4)
    return Biome4MaterialId;
  return Biome5MaterialId;
}

auto chooseOption(List<ConstPtr<Connector>>& options, RandomSource& rnd) -> ConstPtr<Connector> {
  float distribution = 0;
  for (const auto& option : options)
    distribution += option->part()->chance();
  float pick = rnd.randf() * distribution;
  for (size_t i = 0; i < options.size(); i++) {
    pick -= options[i]->part()->chance();
    if (pick <= 0)
      return options.takeAt(i);
  }
  // float rounding is always fun
  return options.takeAt(options.size() - 1);
}

auto Rule::readRules(Json const& rules) -> List<ConstPtr<Rule>> {
  List<ConstPtr<Rule>> result;
  for (auto const& list : rules.iterateArray()) {
    std::optional<ConstPtr<Rule>> rule = Rule::parse(list);
    if (rule.has_value())
      result.push_back(*rule);
  }
  return result;
}

auto Brush::readBrushes(Json const& brushes) -> List<ConstPtr<Brush>> {
  List<ConstPtr<Brush>> result;
  for (auto const& list : brushes.iterateArray())
    result.push_back(Brush::parse(list));
  return result;
}

auto Rule::parse(Json const& rule) -> std::optional<ConstPtr<Rule>> {
  String key = rule.getString(0);
  if (key == "worldGenMustContainLiquid")
    return as<Rule>(std::make_shared<const WorldGenMustContainLiquidRule>());
  if (key == "worldGenMustNotContainLiquid")
    return as<Rule>(std::make_shared<const WorldGenMustNotContainLiquidRule>());

  if (key == "worldGenMustContainSolidForeground")
    return as<Rule>(std::make_shared<const WorldGenMustContainSolidRule>(TileLayer::Foreground));
  if (key == "worldGenMustContainAirForeground")
    return as<Rule>(std::make_shared<const WorldGenMustContainAirRule>(TileLayer::Foreground));

  if (key == "worldGenMustContainSolidBackground")
    return as<Rule>(std::make_shared<const WorldGenMustContainSolidRule>(TileLayer::Background));
  if (key == "worldGenMustContainAirBackground")
    return as<Rule>(std::make_shared<const WorldGenMustContainAirRule>(TileLayer::Background));

  if (key == "allowOverdrawing")
    return as<Rule>(std::make_shared<const AllowOverdrawingRule>());
  if (key == "ignorePartMaximumRule")
    return as<Rule>(std::make_shared<const IgnorePartMaximumRule>());
  if (key == "maxSpawnCount")
    return as<Rule>(std::make_shared<const MaxSpawnCountRule>(rule));
  if (key == "doNotConnectToPart")
    return as<Rule>(std::make_shared<const DoNotConnectToPartRule>(rule));
  if (key == "doNotCombineWith")
    return as<Rule>(std::make_shared<const DoNotCombineWithRule>(rule));

  Logger::error("Unknown dungeon rule: {}", key);
  return std::nullopt;
}

auto Rule::checkTileCanPlace(Vec2I, DungeonGeneratorWriter*) const -> bool {
  return true;
}

auto Rule::overdrawable() const -> bool {
  return false;
}

auto Rule::ignorePartMaximum() const -> bool {
  return false;
}

auto Rule::allowSpawnCount(int) const -> bool {
  return true;
}

auto Rule::doesNotConnectToPart(String const&) const -> bool {
  return false;
}

auto Rule::checkPartCombinationsAllowed(StringMap<int> const&) const -> bool {
  return true;
}

auto Rule::requiresOpen() const -> bool {
  return false;
}

auto Rule::requiresSolid() const -> bool {
  return false;
}

auto Rule::requiresLiquid() const -> bool {
  return false;
}

auto parseFrontBrush(Json const& brush) -> ConstPtr<Brush> {
  String material;
  std::optional<String> mod;
  std::optional<float> hueshift, modhueshift;
  std::optional<MaterialColorVariant> colorVariant;

  if (brush.isType(Json::Type::Object)) {
    material = brush.getString("material");
    mod = brush.optString("mod");
    hueshift = brush.optFloat("hueshift");
    modhueshift = brush.optFloat("modhueshift");
    colorVariant = brush.optFloat("colorVariant");
  } else {
    material = brush.getString(1);
    if (brush.size() > 2)
      mod = brush.getString(2);
  }
  return make_shared<const FrontBrush>(material, mod, hueshift, modhueshift, colorVariant);
}

auto parseBackBrush(Json const& brush) -> ConstPtr<Brush> {
  String material;
  std::optional<String> mod;
  std::optional<float> hueshift, modhueshift;
  std::optional<MaterialColorVariant> colorVariant;

  if (brush.isType(Json::Type::Object)) {
    material = brush.getString("material");
    mod = brush.optString("mod");
    hueshift = brush.optFloat("hueshift");
    modhueshift = brush.optFloat("modhueshift");
    colorVariant = brush.optFloat("colorVariant");
  } else {
    material = brush.getString(1);
    if (brush.size() > 2)
      mod = brush.getString(2);
  }
  return make_shared<const BackBrush>(material, mod, hueshift, modhueshift, colorVariant);
}

auto parseObjectBrush(Json const& brush) -> ConstPtr<Brush> {
  String object;
  Star::Direction direction;
  Json parameters;

  object = brush.getString(1);
  JsonObject settings;
  if (brush.size() > 2)
    settings = brush.getObject(2);
  if (settings.contains("direction"))
    direction = DirectionNames.getLeft(settings.get("direction").toString());
  else
    direction = Star::Direction::Left;

  if (settings.contains("parameters"))
    parameters = settings.get("parameters");
  return std::make_shared<const ObjectBrush>(object, direction, parameters);
}

auto parseSurfaceBrush(Json const& brush) -> ConstPtr<Brush> {
  Json settings = Json::ofType(Json::Type::Object);
  if (brush.size() > 1)
    settings = brush.get(1);
  return make_shared<const SurfaceBrush>(settings.optInt("variant"), settings.optString("mod"));
}

auto parseSurfaceBackgroundBrush(Json const& brush) -> ConstPtr<Brush> {
  Json settings = Json::ofType(Json::Type::Object);
  if (brush.size() > 1)
    settings = brush.get(1);
  return make_shared<const SurfaceBackgroundBrush>(settings.optInt("variant"), settings.optString("mod"));
}

auto parseWireBrush(Json const& brush) -> ConstPtr<Brush> {
  Json settings = brush.get(1);
  String group = settings.getString("group");
  bool local = settings.getBool("local", true);
  return std::make_shared<const WireBrush>(group, local);
}

auto parseItemBrush(Json const& brush) -> ConstPtr<Brush> {
  ItemDescriptor item(brush.getString(1), 1);
  return std::make_shared<const ItemBrush>(item);
}

auto Brush::parse(Json const& brush) -> ConstPtr<Brush> {
  String key = brush.getString(0);
  if (key == "clear")
    return as<const Brush>(std::make_shared<ClearBrush>());

  if (key == "front")
    return parseFrontBrush(brush);
  if (key == "back")
    return parseBackBrush(brush);
  if (key == "object")
    return parseObjectBrush(brush);
  if (key == "biomeitems")
    return as<Brush>(std::make_shared<BiomeItemsBrush>());
  if (key == "biometree")
    return as<Brush>(std::make_shared<BiomeTreeBrush>());
  if (key == "item")
    return parseItemBrush(brush);
  if (key == "npc")
    return as<Brush>(std::make_shared<NpcBrush>(brush.get(1)));
  if (key == "stagehand")
    return as<Brush>(std::make_shared<StagehandBrush>(brush.get(1)));
  if (key == "random")
    return as<Brush>(std::make_shared<RandomBrush>(brush));
  if (key == "surface")
    return parseSurfaceBrush(brush);
  if (key == "surfacebackground")
    return parseSurfaceBackgroundBrush(brush);
  if (key == "liquid")
    return as<Brush>(std::make_shared<LiquidBrush>(brush.getString(1), 1.0f, brush.getBool(2, false)));
  if (key == "wire")
    return parseWireBrush(brush);
  if (key == "playerstart")
    return as<Brush>(std::make_shared<PlayerStartBrush>());
  throw DungeonException::format("Unknown dungeon brush: {}", key);
}

RandomBrush::RandomBrush(Json const& brush) {
  JsonArray options = brush.getArray(1);
  for (auto const& option : options)
    m_brushes.append(Brush::parse(option));
  m_seed = Random::randi64();
}

void RandomBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  auto rnd = (size_t)staticRandomI32(m_seed, position[0], position[1]);
  m_brushes[rnd % m_brushes.size()]->paint(position, phase, writer);
}

void ClearBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::ClearPhase)
    return;

  // TODO: delete objects too?
  writer->setLiquid(position, LiquidStore(EmptyLiquidId, 0.0f, 0.0f, false));
  writer->setForegroundMaterial(position, EmptyMaterialId, 0, DefaultMaterialColorVariant);
  writer->setBackgroundMaterial(position, EmptyMaterialId, 0, DefaultMaterialColorVariant);
  writer->setForegroundMod(position, NoModId, 0);
  writer->setBackgroundMod(position, NoModId, 0);
}

FrontBrush::FrontBrush(String const& material, std::optional<String> mod, std::optional<float> hueshift, std::optional<float> modhueshift, std::optional<MaterialColorVariant> colorVariant) {
  m_material = material;
  m_mod = mod;
  m_materialHue = hueshift.transform(materialHueFromDegrees).value_or(0);
  m_modHue = modhueshift.transform(materialHueFromDegrees).value_or(0);
  m_materialColorVariant = colorVariant.value_or(DefaultMaterialColorVariant);
}

void FrontBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::WallPhase)
    return;

  ConstPtr<MaterialDatabase> materialDatabase = Root::singleton().materialDatabase();
  MaterialId material = materialDatabase->materialId(m_material);

  ModId mod = NoModId;
  if (m_mod)
    mod = materialDatabase->modId(*m_mod);

  if (isSolidColliding(materialDatabase->materialCollisionKind(material)))
    writer->setLiquid(position, LiquidStore(EmptyLiquidId, 0.0f, 0.0f, false));
  writer->setForegroundMaterial(position, material, m_materialHue, m_materialColorVariant);
  if (isRealMod(mod)) {
    writer->setForegroundMod(position, mod, m_modHue);
  }
}

BackBrush::BackBrush(String const& material, std::optional<String> mod, std::optional<float> hueshift, std::optional<float> modhueshift, std::optional<MaterialColorVariant> colorVariant) {
  m_material = material;
  m_mod = mod;
  m_materialHue = hueshift.transform(materialHueFromDegrees).value_or(0);
  m_modHue = modhueshift.transform(materialHueFromDegrees).value_or(0);
  m_materialColorVariant = colorVariant.value_or(DefaultMaterialColorVariant);
}

void BackBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::WallPhase)
    return;

  auto materialDatabase = Root::singleton().materialDatabase();
  MaterialId material = materialDatabase->materialId(m_material);

  ModId mod = NoModId;
  if (m_mod)
    mod = materialDatabase->modId(*m_mod);

  writer->setBackgroundMaterial(position, material, m_materialHue, m_materialColorVariant);
  if (isRealMod(mod)) {
    writer->setBackgroundMod(position, mod, m_modHue);
  }
}

ObjectBrush::ObjectBrush(String const& object, Star::Direction direction, Json const& parameters) {
  m_object = object;
  m_direction = direction;
  m_parameters = parameters;
}

void ObjectBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::ObjectPhase)
    return;
  writer->placeObject(position, m_object, m_direction, m_parameters);
}

VehicleBrush::VehicleBrush(String const& vehicle, Json const& parameters) {
  m_vehicle = vehicle;
  m_parameters = parameters;
}

void VehicleBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::ObjectPhase)
    return;
  writer->placeVehicle(Vec2F(position), m_vehicle, m_parameters);
}

void BiomeItemsBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::BiomeItemsPhase)
    return;
  writer->placeSurfaceBiomeItems(position);
}

void BiomeTreeBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::BiomeTreesPhase)
    return;
  writer->placeBiomeTree(position);
}

ItemBrush::ItemBrush(ItemDescriptor const& item) : m_item(std::move(item)) {}

void ItemBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::ItemPhase)
    return;
  writer->addDrop(Vec2F(position), m_item);
}

NpcBrush::NpcBrush(Json const& brush) {
  m_npc = brush;
  auto map = m_npc.toObject();
  if (map.value("seed") == Json("stable"))
    map["seed"] = Random::randu64();
  m_npc = map;
}

void NpcBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::NpcPhase)
    return;

  if (m_npc.contains("species")) {
    // interpret species as a comma separated list of unquoted strings
    StringList speciesOptions = m_npc.get("species").toString().replace(" ", "").split(",");
    writer->spawnNpc(Vec2F(position), m_npc.set("species", Random::randFrom(speciesOptions)));
  } else {
    writer->spawnNpc(Vec2F(position), m_npc);
  }
}

StagehandBrush::StagehandBrush(Json const& definition) {
  m_definition = definition;
}

void StagehandBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::NpcPhase)
    return;
  writer->spawnStagehand(Vec2F(position), m_definition);
}

DungeonIdBrush::DungeonIdBrush(DungeonId dungeonId) {
  m_dungeonId = dungeonId;
}

void DungeonIdBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::DungeonIdPhase)
    return;
  writer->setDungeonId(position, m_dungeonId);
}

SurfaceBrush::SurfaceBrush(std::optional<int> variant, std::optional<String> mod) {
  m_variant = variant.value_or(0);
  m_mod = mod;
}

void SurfaceBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase == Phase::WallPhase) {
    writer->setForegroundMaterial(position, biomeMaterialForJson(m_variant), 0, DefaultMaterialColorVariant);
    writer->setBackgroundMaterial(position, biomeMaterialForJson(m_variant), 0, DefaultMaterialColorVariant);
  }
  if (phase == Phase::ModsPhase) {
    if (m_mod.has_value()) {
      auto materialDatabase = Root::singleton().materialDatabase();
      writer->setForegroundMod(position, materialDatabase->modId(*m_mod), 0);
    } else {
      if (writer->needsForegroundBiomeMod(position)) {
        writer->setForegroundMod(position, BiomeModId, 0);
      }
    }
  }
}

SurfaceBackgroundBrush::SurfaceBackgroundBrush(std::optional<int> variant, std::optional<String> mod) {
  m_variant = variant.value_or(0);
  m_mod = mod;
}

void SurfaceBackgroundBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase == Phase::WallPhase) {
    writer->setBackgroundMaterial(position, biomeMaterialForJson(m_variant), 0, DefaultMaterialColorVariant);
  }
  if (phase == Phase::ModsPhase) {
    if (m_mod.has_value()) {
      auto materialDatabase = Root::singleton().materialDatabase();
      writer->setBackgroundMod(position, materialDatabase->modId(*m_mod), 0);
    } else {
      if (writer->needsBackgroundBiomeMod(position)) {
        writer->setBackgroundMod(position, BiomeModId, 0);
      }
    }
  }
}

LiquidBrush::LiquidBrush(String const& liquidName, float quantity, bool source)
    : m_liquid(std::move(liquidName)), m_quantity(quantity), m_source(source) {}

void LiquidBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  ConstPtr<LiquidsDatabase> liquidsDatabase = Root::singleton().liquidsDatabase();
  LiquidId liquidId = liquidsDatabase->liquidId(m_liquid);
  LiquidStore liquid(liquidId, m_quantity, 1.0f, m_source);
  if (phase == Phase::WallPhase) {
    writer->requestLiquid(position, liquid);
  }
}

void WireBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase != Phase::WirePhase)
    return;
  writer->requestWire(position, m_wireGroup, m_partLocal);
}

void PlayerStartBrush::paint(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  if (phase == Phase::NpcPhase)
    writer->setPlayerStart(Vec2F(position));
}

InvalidBrush::InvalidBrush(std::optional<String> nameHint) : m_nameHint(std::move(nameHint)) {}

void InvalidBrush::paint(Vec2I, Phase, DungeonGeneratorWriter*) const {
  if (m_nameHint)
    Logger::error("Invalid tile '{}'", *m_nameHint);
  else
    Logger::error("Invalid tile");
}

auto Tile::canPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool {
  if (writer->otherDungeonPresent(position))
    return false;
  else if (position[1] < 0)
    return false;
  for (const auto& rule : rules)
    if (!rule->checkTileCanPlace(position, writer))
      return false;
  return true;
}

void Tile::place(Vec2I position, Phase phase, DungeonGeneratorWriter* writer) const {
  for (const auto& brushe : brushes) {
    brushe->paint(position, phase, writer);
  }
}

auto Tile::usesPlaces() const -> bool {
  if (brushes.size() == 0)
    return false;
  for (const auto& rule : rules)
    if (rule->overdrawable())
      return false;
  return true;
}

auto Tile::modifiesPlaces() const -> bool {
  return brushes.size() != 0;
}

auto Tile::collidesWithPlaces() const -> bool {
  return usesPlaces();
}

auto Tile::requiresOpen() const -> bool {
  for (const auto& rule : rules)
    if (rule->requiresOpen())
      return true;
  return false;
}

auto Tile::requiresSolid() const -> bool {
  for (const auto& rule : rules)
    if (rule->requiresSolid())
      return true;
  return false;
}

auto Tile::requiresLiquid() const -> bool {
  for (const auto& rule : rules)
    if (rule->requiresLiquid())
      return true;
  return false;
}

auto parsePart(DungeonDefinition* dungeon, Json const& definition, std::optional<ConstPtr<ImageTileset>> tileset) -> ConstPtr<Part> {
  String kind = definition.get("def").getString(0);
  if (kind == "image") {
    if (!tileset.has_value())
      throw DungeonException("Dungeon parts designed in images require the 'tiles' key in the .dungeon file");
    return make_shared<const Part>(dungeon, definition, std::make_shared<ImagePartReader>(*tileset));
  } else if (kind == "tmx")
    return make_shared<const Part>(dungeon, definition, std::make_shared<TMXPartReader>());
  throw DungeonException::format("Unknown dungeon part kind: {}", kind);
}

Part::Part(DungeonDefinition* dungeon, Json const& part, Ptr<PartReader> reader) {
  m_dungeon = dungeon;
  m_name = part.getString("name");
  m_rules = Rule::readRules(part.get("rules"));
  m_chance = part.getFloat("chance", 1);
  if (m_chance <= 0)
    m_chance = 0.0001f;
  m_markDungeonId = part.getBool("markDungeonId", true);
  m_overrideAllowAlways = part.getBool("overrideAllowAlways", false);
  m_minimumThreatLevel = part.optFloat("minimumThreatLevel");
  m_maximumThreatLevel = part.optFloat("maximumThreatLevel");
  m_clearAnchoredObjects = part.getBool("clearAnchoredObjects", true);

  m_reader = reader;
  Json const& def = part.get("def");
  if (def.get(1).type() == Json::Type::String) {
    reader->readAsset(AssetPath::relativeTo(dungeon->directory(), def.get(1).toString()));
  } else {
    for (auto const& asset : def.get(1).iterateArray())
      reader->readAsset(AssetPath::relativeTo(dungeon->directory(), asset.toString()));
  }
  m_size = m_reader->size();
  scanConnectors();
  scanAnchor();
}

auto Part::name() const -> String const& {
  return m_name;
}

auto Part::size() const -> Vec2U {
  return m_size;
}

auto Part::anchorPoint() const -> Vec2I {
  return m_anchorPoint;
}

auto Part::chance() const -> float {
  return m_chance;
}

auto Part::markDungeonId() const -> bool {
  return m_markDungeonId;
}

auto Part::minimumThreatLevel() const -> std::optional<float> {
  return m_minimumThreatLevel;
}

auto Part::maximumThreatLevel() const -> std::optional<float> {
  return m_maximumThreatLevel;
}

auto Part::clearAnchoredObjects() const -> bool {
  return m_clearAnchoredObjects;
}

auto Part::placementLevelConstraint() const -> int {
  Vec2I air = {0, size().y()};
  Vec2I ground = {0, 0};
  Vec2I liquid = {0, 0};
  m_reader->forEachTile([&ground, &air, &liquid](Vec2I tilePos, Tile const& tile) -> bool {
    for (auto const& rule : tile.rules) {
      if (is<WorldGenMustContainSolidRule>(rule) && tilePos.y() > ground.y()) {
        ground = tilePos;
      }
      if (is<WorldGenMustContainAirRule>(rule) && tilePos.y() < air.y()) {
        air = tilePos;
      }
      if ((is<WorldGenMustContainLiquidRule>(rule) || is<WorldGenMustNotContainLiquidRule>(rule)) && tilePos.y() > liquid.y()) {
        liquid = tilePos;
      }
    }
    return false;
  });
  ground[1] = std::max(ground[1], liquid[1]);
  if (air.y() < ground.y())
    throw DungeonException::format(
      "Invalid ground vs air contraint! Ground {} can't be above air {}"
      " (try moving your 'require there be air here' anchors above any other 'require there be (something) here' anchors.)",
      ground, air);
  return air.y();
}

auto Part::ignoresPartMaximum() const -> bool {
  for (const auto& m_rule : m_rules)
    if (m_rule->ignorePartMaximum())
      return true;
  return false;
}

auto Part::allowsPlacement(int currentPlacementCount) const -> bool {
  for (const auto& m_rule : m_rules)
    if (!m_rule->allowSpawnCount(currentPlacementCount))
      return false;
  return true;
}

auto Part::connections() const -> List<ConstPtr<Connector>> const& {
  return m_connections;
}

auto Part::doesNotConnectTo(Part* part) const -> bool {
  for (const auto& m_rule : m_rules)
    if (m_rule->doesNotConnectToPart(part->name()))
      return true;
  for (const auto& m_rule : part->m_rules)
    if (m_rule->doesNotConnectToPart(m_name))
      return true;
  return false;
}

auto Part::checkPartCombinationsAllowed(StringMap<int> const& placementCounter) const -> bool {
  for (const auto& m_rule : m_rules)
    if (!m_rule->checkPartCombinationsAllowed(placementCounter))
      return false;
  return true;
}

auto Part::collidesWithPlaces(Vec2I pos, Set<Vec2I>& places) const -> bool {
  if (m_overrideAllowAlways)
    return true;

  bool result = false;
  m_reader->forEachTile([&result, pos, &places](Vec2I tilePos, Tile const& tile) -> bool {
    if (tile.collidesWithPlaces())
      if (places.contains(pos + tilePos)) {
        Logger::debug("Tile collided with place at {}", pos + tilePos);
        result = true;
        return true;
      }
    return false;
  });

  return result;
}

auto Part::canPlace(Vec2I pos, DungeonGeneratorWriter* writer) const -> bool {
  if (m_overrideAllowAlways)
    return true;

  // Speed up repeated failing calls by first checking the tile that failed
  // last time (if it did).
  bool result = true;
  m_reader->forEachTile([&result, pos, writer](Vec2I tilePos, Tile const& tile) -> bool {
    Vec2I position = pos + tilePos;
    if (!tile.canPlace(position, writer)) {
      result = false;
      return true;
    }
    return false;
  });

  return result;
}

void Part::place(Vec2I pos, Set<Vec2I> const& places, DungeonGeneratorWriter* writer) const {
  placePhase(pos, Phase::ClearPhase, places, writer);
  placePhase(pos, Phase::WallPhase, places, writer);
  placePhase(pos, Phase::ModsPhase, places, writer);
  placePhase(pos, Phase::ObjectPhase, places, writer);
  placePhase(pos, Phase::BiomeTreesPhase, places, writer);
  placePhase(pos, Phase::BiomeItemsPhase, places, writer);
  placePhase(pos, Phase::WirePhase, places, writer);
  placePhase(pos, Phase::ItemPhase, places, writer);
  placePhase(pos, Phase::NpcPhase, places, writer);
  placePhase(pos, Phase::DungeonIdPhase, places, writer);
}

void Part::forEachTile(TileCallback const& callback) const {
  m_reader->forEachTile(callback);
}

void Part::placePhase(Vec2I pos, Phase phase, Set<Vec2I> const& places, DungeonGeneratorWriter* writer) const {
  m_reader->forEachTile([&places, pos, phase, writer](Vec2I tilePos, Tile const& tile) -> bool {
    Vec2I position = pos + tilePos;
    if (tile.collidesWithPlaces() || !places.contains(position)) {
      try {
        tile.place(position, phase, writer);
      } catch (std::exception const&) {
        Logger::error("Error at map position {}:", tilePos);
        throw;
      }
    }
    return false;
  });
}

auto Part::tileUsesPlaces(Vec2I pos) const -> bool {
  bool result = false;
  m_reader->forEachTileAt(pos,
                          [&result](Vec2I, Tile const& tile) -> bool {
                            if (tile.usesPlaces()) {
                              result = true;
                              return true;
                            }
                            return false;
                          });
  return result;
}

auto Part::pickByEdge(Vec2I position, Vec2U size) const -> Direction {
  int dxa = position[0];
  int dxb = size[0] - position[0];
  int dya = position[1];
  int dyb = size[1] - position[1];

  int m = std::min({dxa, dxb, dya, dyb});
  if (dxa == m)
    return Direction::Left;
  if (dxb == m)
    return Direction::Right;
  if (dya == m)
    return Direction::Down;
  if (dyb == m)
    return Direction::Up;
  throw DungeonException("Ambiguous direction");
}

auto Part::pickByNeighbours(Vec2I pos) const -> Direction {
  int x = pos.x(), y = pos.y();

  // if on a border use that, corners use the left/right direction
  if (x == 0)
    return Direction::Left;
  if (x == (int)size().x() - 1)
    return Direction::Right;
  if (y == 0)
    return Direction::Down;
  if (y == (int)size().y() - 1)
    return Direction::Up;

  // scans around the connector, the direction where it finds a solid is where
  // it assume the
  // connection comes from

  if (tileUsesPlaces({x + 1, y}) && !tileUsesPlaces({x - 1, y}))
    return Direction::Left;

  if (tileUsesPlaces({x - 1, y}) && !tileUsesPlaces({x + 1, y}))
    return Direction::Right;

  if (tileUsesPlaces({x, y + 1}) && !tileUsesPlaces({x, y - 1}))
    return Direction::Down;

  if (tileUsesPlaces({x, y - 1}) && !tileUsesPlaces({x, y + 1}))
    return Direction::Up;

  return Direction::Unknown;
}

void Part::scanConnectors() {
  try {
    m_reader->forEachTile([this](Vec2I position, Tile const& tile) -> bool {
      if (tile.connector.has_value()) {
        auto d = tile.connector->direction;
        if (d == Direction::Unknown)
          d = pickByNeighbours(position);
        if (d == Direction::Unknown)
          d = pickByEdge(position, m_size);
        Logger::debug("Found connector on {} at {} group {} direction {}", m_name, position, tile.connector->value, (int)d);
        m_connections.append(make_shared<Connector>(this, tile.connector->value, tile.connector->forwardOnly, d, position));
      }

      return false;
    });
  } catch (std::exception& e) {
    throw DungeonException(strf("Exception {} in connector {}", outputException(e, true), m_name));
  }
}

void Part::scanAnchor() {
  int cx, cy, cc;
  cx = cy = cc = 0;
  int lowestAir = m_size[1];
  int highestGound = -1;
  int highestLiquid = -1;
  try {
    m_reader->forEachTile([&](Vec2I pos, Tile const& tile) -> bool {
      int x = pos.x(), y = pos.y();
      if (tile.collidesWithPlaces()) {
        cx += x;
        cy += y;
        cc++;
      }
      if (tile.requiresOpen()) {
        if ((int)y < lowestAir)
          lowestAir = y;
      }
      if (tile.requiresSolid()) {
        if ((int)y > highestGound)
          highestGound = y;
      }
      if (tile.requiresLiquid()) {
        if ((int)y > highestLiquid)
          highestLiquid = y;
      }
      return false;
    });
  } catch (std::exception& e) {
    throw DungeonException(strf("Exception {} in part {}", outputException(e, true), m_name));
  }

  highestGound = std::max(highestGound, highestLiquid);
  if (highestGound == -1)
    highestGound = lowestAir - 1;

  if (lowestAir == (int)m_size[1])
    lowestAir = highestGound + 1;

  if (cc == 0) {
    cx = m_size[0] / 2;
    cy = m_size[1] / 2;
  } else {
    cx /= cc;
    cy /= cc;
  }

  if (highestGound != -1)
    cy = highestGound + 1;

  m_anchorPoint = {cx, cy};
}

auto WorldGenMustContainSolidRule::checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool {
  return writer->checkSolid(position, layer);
}

auto WorldGenMustContainAirRule::checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool {
  return writer->checkOpen(position, layer);
}

auto WorldGenMustContainLiquidRule::checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool {
  return writer->checkLiquid(position);
}

auto WorldGenMustNotContainLiquidRule::checkTileCanPlace(Vec2I position, DungeonGeneratorWriter* writer) const -> bool {
  return !writer->checkLiquid(position);
}

Connector::Connector(Part* part, String value, bool forwardOnly, Direction direction, Vec2I offset)
    : m_value(std::move(value)), m_forwardOnly(forwardOnly), m_direction(direction), m_offset(offset), m_part(part) {}

auto Connector::connectsTo(ConstPtr<Connector> connector) const -> bool {
  if (m_forwardOnly)
    return false;
  if (m_value != connector->m_value)
    return false;
  if (m_direction == Direction::Any || connector->m_direction == Direction::Any)
    return true;
  if (m_direction != flipDirection(connector->m_direction))
    return false;
  return true;
}

auto Connector::value() const -> String {
  return m_value;
}

auto Connector::positionAdjustment() const -> Vec2I {
  if (m_direction == Direction::Any)
    return {0, 0};
  if (m_direction == Direction::Left)
    return {-1, 0};
  if (m_direction == Direction::Right)
    return {1, 0};
  if (m_direction == Direction::Up)
    return {0, 1};
  return {0, -1};
}

auto Connector::part() const -> Part* {
  return m_part;
}

auto Connector::offset() const -> Vec2I {
  return m_offset;
}

DungeonGeneratorWriter::DungeonGeneratorWriter(Ptr<DungeonGeneratorWorldFacade> facade, std::optional<int> terrainMarkingSurfaceLevel, std::optional<int> terrainSurfaceSpaceExtends)
    : m_facade(std::move(facade)), m_terrainMarkingSurfaceLevel(terrainMarkingSurfaceLevel), m_terrainSurfaceSpaceExtends(terrainSurfaceSpaceExtends) {
  m_currentBounds.setMin(Vec2I{std::numeric_limits<std::int32_t>::max(), std::numeric_limits<std::int32_t>::max()});
  m_currentBounds.setMax(Vec2I{std::numeric_limits<std::int32_t>::min(), std::numeric_limits<std::int32_t>::min()});
}

auto DungeonGeneratorWriter::wrapPosition(Vec2I const& pos) const -> Vec2I {
  return m_facade->getWorldGeometry().xwrap(pos);
}

void DungeonGeneratorWriter::setMarkDungeonId(std::optional<DungeonId> dungeonId) {
  m_markDungeonId = dungeonId;
}

void DungeonGeneratorWriter::requestLiquid(Vec2I const& pos, LiquidStore const& liquid) {
  m_pendingLiquids[pos] = liquid;
}

void DungeonGeneratorWriter::setLiquid(Vec2I const& pos, LiquidStore const& liquid) {
  m_liquids[pos] = liquid;
  markPosition(pos);
}

void DungeonGeneratorWriter::setForegroundMaterial(Vec2I const& position, MaterialId material, MaterialHue hueshift, MaterialColorVariant colorVariant) {
  m_foregroundMaterial[position] = {.material = material, .hueshift = hueshift, .colorVariant = colorVariant};
  markPosition(position);
}

void DungeonGeneratorWriter::setBackgroundMaterial(Vec2I const& position, MaterialId material, MaterialHue hueshift, MaterialColorVariant colorVariant) {
  m_backgroundMaterial[position] = {.material = material, .hueshift = hueshift, .colorVariant = colorVariant};
  markPosition(position);
}

void DungeonGeneratorWriter::setForegroundMod(Vec2I const& position, ModId mod, MaterialHue hueshift) {
  m_foregroundMod[position] = {.mod = mod, .hueshift = hueshift};
  markPosition(position);
}

void DungeonGeneratorWriter::setBackgroundMod(Vec2I const& position, ModId mod, MaterialHue hueshift) {
  m_backgroundMod[position] = {.mod = mod, .hueshift = hueshift};
  markPosition(position);
}

auto DungeonGeneratorWriter::needsForegroundBiomeMod(Vec2I const& position) -> bool {
  if (!m_foregroundMaterial.contains(position))
    return false;
  if (!isBiomeMaterial(m_foregroundMaterial[position].material))
    return false;
  Vec2I abovePosition(position.x(), position.y() + 1);
  if (m_foregroundMaterial.contains(abovePosition))
    if (m_foregroundMaterial[abovePosition].material != EmptyMaterialId)
      return false;
  return true;
}

auto DungeonGeneratorWriter::needsBackgroundBiomeMod(Vec2I const& position) -> bool {
  if (!m_backgroundMaterial.contains(position))
    return false;
  if (!isBiomeMaterial(m_backgroundMaterial[position].material))
    return false;
  Vec2I abovePosition(position.x(), position.y() + 1);
  if (m_backgroundMaterial.contains(abovePosition))
    if (m_backgroundMaterial[abovePosition].material != EmptyMaterialId)
      return false;
  if (m_foregroundMaterial.contains(abovePosition))
    if (m_foregroundMaterial[abovePosition].material != EmptyMaterialId)
      return false;
  return true;
}

void DungeonGeneratorWriter::placeObject(Vec2I const& pos, String const& objectType, Star::Direction direction, Json const& parameters) {
  m_objects[pos] = {objectType, direction, parameters};
  markPosition(pos);
}

void DungeonGeneratorWriter::placeVehicle(Vec2F const& pos, String const& vehicleName, Json const& parameters) {
  m_vehicles[pos] = std::make_pair(vehicleName, parameters);
  markPosition(pos);
}

void DungeonGeneratorWriter::placeSurfaceBiomeItems(Vec2I const& pos) {
  m_biomeItems.insert(pos);
  markPosition(pos);
}

void DungeonGeneratorWriter::placeBiomeTree(Vec2I const& pos) {
  m_biomeTrees.insert(pos);
  markPosition(pos);
}

void DungeonGeneratorWriter::addDrop(Vec2F const& position, ItemDescriptor const& item) {
  m_drops[position] = item;
  markPosition(position);
}

void DungeonGeneratorWriter::requestWire(Vec2I const& position, String const& wireGroup, bool partLocal) {
  if (partLocal)
    m_openLocalWires[wireGroup].add(position);
  else
    m_globalWires[wireGroup].add(position);
}

void DungeonGeneratorWriter::spawnNpc(Vec2F const& position, Json const& definition) {
  m_npcs[position] = definition;
  markPosition(position);
}

void DungeonGeneratorWriter::spawnStagehand(Vec2F const& position, Json const& definition) {
  m_stagehands[position] = definition;
  markPosition(position);
}

void DungeonGeneratorWriter::setPlayerStart(Vec2F const& startPosition) {
  m_facade->setPlayerStart(startPosition);
}

auto DungeonGeneratorWriter::checkSolid(Vec2I position, TileLayer layer) -> bool {
  if (m_terrainMarkingSurfaceLevel)
    return position.y() < *m_terrainMarkingSurfaceLevel;
  return m_facade->checkSolid(position, layer);
}

auto DungeonGeneratorWriter::checkOpen(Vec2I position, TileLayer layer) -> bool {
  if (m_terrainMarkingSurfaceLevel)
    return position.y() >= *m_terrainMarkingSurfaceLevel;
  return m_facade->checkOpen(position, layer);
}

auto DungeonGeneratorWriter::checkLiquid(Vec2I const& position) -> bool {
  return m_facade->checkOceanLiquid(position);
}

auto DungeonGeneratorWriter::otherDungeonPresent(Vec2I position) -> bool {
  return m_facade->getDungeonIdAt(position) != NoDungeonId;
}

void DungeonGeneratorWriter::setDungeonId(Vec2I const& pos, DungeonId dungeonId) {
  m_dungeonIds[pos] = dungeonId;
}

void DungeonGeneratorWriter::markPosition(Vec2F const& pos) {
  markPosition(Vec2I(pos.floor()));
}

void DungeonGeneratorWriter::markPosition(Vec2I const& pos) {
  m_currentBounds.combine(pos);
  if (m_markDungeonId)
    m_dungeonIds[pos] = *m_markDungeonId;
}

void DungeonGeneratorWriter::clearTileEntities(RectI const& bounds, Set<Vec2I> const& positions, bool clearAnchoredObjects) {
  m_facade->clearTileEntities(bounds, positions, clearAnchoredObjects);
}

void DungeonGeneratorWriter::finishPart() {
  for (auto& entries : m_openLocalWires)
    m_localWires.append(entries.second);
  m_openLocalWires.clear();

  if (m_currentBounds.xMin() > m_currentBounds.xMax())
    return;
  m_boundingBoxes.push_back(m_currentBounds);
  m_currentBounds.setMin(Vec2I{std::numeric_limits<std::int32_t>::max(), std::numeric_limits<std::int32_t>::max()});
  m_currentBounds.setMax(Vec2I{std::numeric_limits<std::int32_t>::min(), std::numeric_limits<std::int32_t>::min()});
}

void DungeonGeneratorWriter::flushLiquid() {
  // For each liquid type, find each contiguous region of liquid, then
  // pressurize that region based on the highest position in the region

  Map<LiquidId, Set<Vec2I>> unpressurizedLiquids;
  for (auto& p : m_pendingLiquids)
    unpressurizedLiquids[p.second.liquid].add(p.first);

  for (auto& liquidPair : unpressurizedLiquids) {
    auto& unpressurized = liquidPair.second;
    while (!unpressurized.empty()) {
      // Start with the first unpressurized block as the open set.
      Vec2I firstBlock = unpressurized.takeFirst();
      List<Vec2I> openSet = {firstBlock};
      Set<Vec2I> contiguousRegion = {firstBlock};

      // For each element in the previous open set, add all connected blocks
      // in
      // the unpressurized set to the new open set and to the total contiguous
      // region, taking them from the unpressurized set.
      while (!openSet.empty()) {
        auto oldOpenSet = take(openSet);
        for (auto const& p : oldOpenSet) {
          for (auto dir : {Vec2I(1, 0), Vec2I(-1, 0), Vec2I(0, 1), Vec2I(0, -1)}) {
            Vec2I pos = p + dir;
            if (unpressurized.remove(pos)) {
              contiguousRegion.add(pos);
              openSet.append(pos);
            }
          }
        }
      }

      // Once we have found no more blocks in the unpressurized set to add to
      // the open set, then we have taken a contiguous region out of the
      // unpressurized set.  Pressurize it based on the highest point.
      int highestPoint = lowest<int>();
      for (auto const& p : contiguousRegion)
        highestPoint = std::max(highestPoint, p[1]);
      for (auto const& p : contiguousRegion)
        m_pendingLiquids[p].pressure = 1.0f + highestPoint - p[1];
    }
  }

  for (auto& p : m_pendingLiquids)
    setLiquid(p.first, p.second);

  m_pendingLiquids.clear();
}

void DungeonGeneratorWriter::flush() {
  auto geometry = m_facade->getWorldGeometry();
  auto displace = [&](Vec2I pos) -> Vec2I { return geometry.xwrap(pos); };
  auto displaceF = [&](Vec2F pos) -> Vec2F { return geometry.xwrap(pos); };

  PolyF::VertexList terrainBlendingVertexes;
  PolyF::VertexList spaceBlendingVertexes;
  for (auto bb : m_boundingBoxes) {
    m_facade->markRegion(bb);

    if (m_terrainMarkingSurfaceLevel) {
      // Mark the regions of the dungeon above the dungeon surface as needing
      // space, and the regions below the surface as needing terrain
      if (bb.yMin() < *m_terrainMarkingSurfaceLevel) {
        RectI lower = bb;
        lower.setYMax(std::min(lower.yMax(), *m_terrainMarkingSurfaceLevel));
        terrainBlendingVertexes.append(Vec2F(lower.xMin(), lower.yMin()));
        terrainBlendingVertexes.append(Vec2F(lower.xMax(), lower.yMin()));
        terrainBlendingVertexes.append(Vec2F(lower.xMin(), lower.yMax()));
        terrainBlendingVertexes.append(Vec2F(lower.xMax(), lower.yMax()));
      }

      if (bb.yMax() > *m_terrainMarkingSurfaceLevel) {
        RectI upper = bb;
        upper.setYMin(std::max(upper.yMin(), *m_terrainMarkingSurfaceLevel));
        spaceBlendingVertexes.append(Vec2F(upper.xMin(), upper.yMin()));
        spaceBlendingVertexes.append(Vec2F(upper.xMax(), upper.yMin()));
        spaceBlendingVertexes.append(Vec2F(upper.xMin(), upper.yMax() + m_terrainSurfaceSpaceExtends.value_or(0)));
        spaceBlendingVertexes.append(Vec2F(upper.xMax(), upper.yMax() + m_terrainSurfaceSpaceExtends.value_or(0)));
      }
    }
  }

  if (!terrainBlendingVertexes.empty())
    m_facade->markTerrain(PolyF::convexHull(terrainBlendingVertexes));
  if (!spaceBlendingVertexes.empty())
    m_facade->markSpace(PolyF::convexHull(spaceBlendingVertexes));

  for (auto& iter : m_backgroundMaterial)
    m_facade->setBackgroundMaterial(displace(iter.first), iter.second.material, iter.second.hueshift, iter.second.colorVariant);
  for (auto& iter : m_foregroundMaterial)
    m_facade->setForegroundMaterial(displace(iter.first), iter.second.material, iter.second.hueshift, iter.second.colorVariant);
  for (auto& iter : m_foregroundMod)
    m_facade->setForegroundMod(displace(iter.first), iter.second.mod, iter.second.hueshift);
  for (auto& iter : m_backgroundMod)
    m_facade->setBackgroundMod(displace(iter.first), iter.second.mod, iter.second.hueshift);

  List<Vec2I> sortedPositions = m_objects.keys();
  sortByComputedValue(sortedPositions, [](Vec2I pos) -> float { return pos[1] + pos[0] / 1000.0f; });
  for (auto pos : sortedPositions) {
    auto& object = m_objects[pos];
    m_facade->placeObject(displace(pos), object.objectName, object.direction, object.parameters);
  }

  for (auto entry : m_vehicles) {
    String vehicleName;
    Json parameters;
    std::tie(vehicleName, parameters) = entry.second;
    m_facade->placeVehicle(displaceF(entry.first), vehicleName, parameters);
  }

  sortedPositions = List<Vec2I>::from(m_biomeTrees);
  sortByComputedValue(sortedPositions, [](Vec2I pos) -> float { return pos[1] + pos[0] / 1000.0f; });
  for (auto pos : sortedPositions) {
    m_facade->placeBiomeTree(pos);
  }

  sortedPositions = List<Vec2I>::from(m_biomeItems);
  sortByComputedValue(sortedPositions, [](Vec2I pos) -> float { return pos[1] + pos[0] / 1000.0f; });
  for (auto pos : sortedPositions) {
    m_facade->placeSurfaceBiomeItems(pos);
  }

  for (auto& npc : m_npcs) {
    m_facade->spawnNpc(displaceF(npc.first), npc.second);
  }

  for (auto& stagehand : m_stagehands) {
    m_facade->spawnStagehand(displaceF(stagehand.first), stagehand.second);
  }

  for (auto& wires : m_globalWires) {
    List<Vec2I> wireGroup;
    for (auto& pos : wires.second)
      wireGroup.append(displace(pos));
    m_facade->connectWireGroup(wireGroup);
  }
  for (auto& wires : m_localWires) {
    List<Vec2I> wireGroup;
    for (auto& pos : wires)
      wireGroup.append(displace(pos));
    m_facade->connectWireGroup(wireGroup);
  }

  for (auto& m_drop : m_drops)
    m_facade->addDrop(displaceF(m_drop.first), m_drop.second);

  for (auto& m_liquid : m_liquids)
    m_facade->setLiquid(displace(m_liquid.first), m_liquid.second);

  for (auto const& dungeonId : m_dungeonIds)
    m_facade->setDungeonIdAt(dungeonId.first, dungeonId.second);
}

auto DungeonGeneratorWriter::boundingBoxes() const -> List<RectI> {
  return m_boundingBoxes;
}

void DungeonGeneratorWriter::reset() {
  m_currentBounds.setMin(Vec2I{std::numeric_limits<std::int32_t>::max(), std::numeric_limits<std::int32_t>::max()});
  m_currentBounds.setMax(Vec2I{std::numeric_limits<std::int32_t>::min(), std::numeric_limits<std::int32_t>::min()});

  m_pendingLiquids.clear();
  m_foregroundMaterial.clear();
  m_backgroundMaterial.clear();
  m_foregroundMod.clear();
  m_backgroundMod.clear();
  m_objects.clear();
  m_biomeTrees.clear();
  m_biomeItems.clear();
  m_drops.clear();
  m_npcs.clear();
  m_stagehands.clear();
  m_liquids.clear();
  m_globalWires.clear();
  m_localWires.clear();
  m_openLocalWires.clear();
  m_boundingBoxes.clear();
}
}// namespace Dungeon

DungeonDefinitions::DungeonDefinitions() : m_paths(), m_cacheMutex(), m_definitionCache(DefinitionsCacheSize) {
  auto assets = Root::singleton().assets();

  for (auto& file : assets->scan(".dungeon")) {
    Json dungeon = assets->json(file);
    m_paths.insert(dungeon.get("metadata").getString("name"), file);
  }
}

auto DungeonDefinitions::get(String const& name) const -> ConstPtr<DungeonDefinition> {
  MutexLocker locker(m_cacheMutex);
  return m_definitionCache.get(name,
                               [this](String const& name) -> Ptr<DungeonDefinition> {
                                 if (auto path = m_paths.maybe(name))
                                   return readDefinition(*path);
                                 throw DungeonException::format("Unknown dungeon: '{}'", name);
                               });
}

auto DungeonDefinitions::getMetadata(String const& name) const -> JsonObject {
  auto definition = get(name);
  return definition->metadata();
}

auto DungeonDefinitions::readDefinition(String const& path) -> Ptr<DungeonDefinition> {
  try {
    auto assets = Root::singleton().assets();
    return make_shared<DungeonDefinition>(assets->json(path).toObject(), AssetPath::directory(path));
  } catch (std::exception const& e) {
    throw DungeonException::format("Error loading dungeon '{}': {}", path, outputException(e, false));
  }
}

DungeonDefinition::DungeonDefinition(JsonObject const& definition, String const& directory) {
  m_directory = directory;
  m_metadata = definition.get("metadata").toObject();
  m_name = m_metadata.get("name").toString();
  m_displayName = m_metadata.contains("displayName") ? m_metadata.get("displayName").toString() : "";
  m_species = m_metadata.get("species").toString();
  m_isProtected = m_metadata.contains("protected") ? m_metadata.get("protected").toBool() : false;
  if (m_metadata.contains("rules"))
    m_rules = Dungeon::Rule::readRules(m_metadata.get("rules"));

  m_maxRadius = m_metadata.value("maxRadius", 100).toInt();
  m_maxParts = m_metadata.value("maxParts", 100).toInt();
  m_extendSurfaceFreeSpace = m_metadata.value("extendSurfaceFreeSpace", 0).toInt();

  m_anchors = jsonToStringList(m_metadata.get("anchor"));

  auto tileset = definition.maybe("tiles").transform([](Json const& tileset) -> auto {
    return std::make_shared<const Dungeon::ImageTileset>(tileset);
  });

  for (auto const& partsDefMap : definition.get("parts").iterateArray()) {
    ConstPtr<Dungeon::Part> part = parsePart(this, partsDefMap, tileset);
    if (m_parts.contains(part->name()))
      throw DungeonException::format("Duplicate dungeon part name: {}", part->name());
    m_parts.insert(part->name(), part);
  }

  if (m_metadata.contains("gravity"))
    m_gravity = m_metadata.get("gravity").toFloat();

  if (m_metadata.contains("breathable"))
    m_breathable = m_metadata.get("breathable").toBool();
}

auto DungeonDefinition::metadata() const -> JsonObject {
  return m_metadata;
}

auto DungeonDefinition::directory() const -> String {
  return m_directory;
}

auto DungeonDefinition::name() const -> String {
  return m_name;
}

auto DungeonDefinition::displayName() const -> String {
  return m_displayName;
}

auto DungeonDefinition::isProtected() const -> bool {
  return m_isProtected;
}

auto DungeonDefinition::gravity() const -> std::optional<float> {
  return m_gravity;
}

auto DungeonDefinition::breathable() const -> std::optional<bool> {
  return m_breathable;
}

auto DungeonDefinition::parts() const -> StringMap<ConstPtr<Dungeon::Part>> const& {
  return m_parts;
}

auto DungeonDefinition::anchors() const -> List<String> const& {
  return m_anchors;
}

auto DungeonDefinition::optTileset() const -> std::optional<Json> const& {
  return m_tileset;
}

auto DungeonDefinition::maxParts() const -> int {
  return m_maxParts;
}

auto DungeonDefinition::maxRadius() const -> int {
  return m_maxRadius;
}

auto DungeonDefinition::extendSurfaceFreeSpace() const -> int {
  return m_extendSurfaceFreeSpace;
}

DungeonGenerator::DungeonGenerator(String const& dungeonName, std::uint64_t seed, float threatLevel, std::optional<DungeonId> dungeonId)
    : m_rand(seed), m_threatLevel(threatLevel), m_dungeonId(dungeonId) {
  m_def = Root::singleton().dungeonDefinitions()->get(dungeonName);
}

auto DungeonGenerator::generate(Ptr<DungeonGeneratorWorldFacade> facade, Vec2I position, bool markSurfaceAndTerrain, bool forcePlacement) -> std::optional<std::pair<List<RectI>, Set<Vec2I>>> {
  String name = m_def->name();
  try {
    Dungeon::DungeonGeneratorWriter writer(facade, markSurfaceAndTerrain ? position[1] : std::optional<int>(), m_def->extendSurfaceFreeSpace());

    if (forcePlacement)
      Logger::debug("Forcing generation of dungeon {}", name);
    else
      Logger::debug("Generating dungeon {}", name);

    ConstPtr<Dungeon::Part> anchor = pickAnchor();
    if (!anchor) {
      Logger::error("No valid anchor piece found for dungeon {} at {}", name, position);
      return {};
    }

    auto pos = position + Vec2I(0, -anchor->placementLevelConstraint());
    if (forcePlacement || anchor->canPlace(pos, &writer)) {
      Logger::info("Placing dungeon {} at {}", name, position);
      return buildDungeon(anchor, pos, &writer, forcePlacement);
    } else {
      Logger::debug("Failed to place dungeon {} at {}", name, position);
      return {};
    }
  } catch (std::exception const& e) {
    throw DungeonException(strf("Error generating dungeon named '{}'", name), e);
  }
}

auto DungeonGenerator::buildDungeon(ConstPtr<Dungeon::Part> anchor, Vec2I basePos, Dungeon::DungeonGeneratorWriter* writer, bool forcePlacement) -> std::pair<List<RectI>, Set<Vec2I>> {
  writer->reset();

  Deque<std::pair<Dungeon::Part const*, Vec2I>> openSet;
  StringMap<int> placementCounter;
  Set<Vec2I> modifiedTiles;
  Set<Vec2I> preserveTiles;
  int piecesPlaced = 0;

  Logger::debug("Placing dungeon entrance at {}", basePos);

  auto placePart = [&](Dungeon::Part const* part, Vec2I const& placePos) -> void {
    Set<Vec2I> clearTileEntityPositions;
    part->forEachTile([&](Vec2I tilePos, Dungeon::Tile const& tile) -> bool {
      if (tile.modifiesPlaces())
        clearTileEntityPositions.insert(writer->wrapPosition(placePos + tilePos));
      return false;
    });
    auto partBounds = RectI::withSize(placePos, Vec2I(part->size()));
    writer->clearTileEntities(partBounds, clearTileEntityPositions, part->clearAnchoredObjects());

    if (part->markDungeonId())
      writer->setMarkDungeonId(m_dungeonId);
    else
      writer->setMarkDungeonId();

    part->place(placePos, preserveTiles, writer);
    writer->finishPart();

    part->forEachTile([&](Vec2I tilePos, Dungeon::Tile const& tile) -> bool {
      if (tile.usesPlaces())
        preserveTiles.insert(placePos + tilePos);
      if (tile.modifiesPlaces())
        modifiedTiles.insert(placePos + tilePos);
      return false;
    });

    openSet.append({part, placePos});

    placementCounter[part->name()]++;
    piecesPlaced++;

    Logger::debug("placed {}", part->name());
  };

  placePart(anchor.get(), basePos);

  Vec2I origin = basePos + Vec2I(anchor->size()) / 2;

  Set<Vec2I> closedConnectors;
  while (openSet.size()) {
    Dungeon::Part const* parentPart = openSet.first().first;
    Vec2I parentPos = openSet.first().second;
    openSet.takeFirst();
    Logger::debug("Trying to add part {} at {} connectors: {}", parentPart->name(), parentPos, parentPart->connections().size());
    for (auto connector : parentPart->connections()) {
      Vec2I connectorPos = parentPos + connector->offset();
      if (closedConnectors.contains(connectorPos))
        continue;
      List<ConstPtr<Dungeon::Connector>> options = findConnectablePart(connector);
      while (options.size()) {
        ConstPtr<Dungeon::Connector> option = chooseOption(options, m_rand);
        Logger::debug("Trying part {}", option->part()->name());
        Vec2I partPos = connectorPos - option->offset() + option->positionAdjustment();
        Vec2I optionPos = connectorPos + option->positionAdjustment();
        if (!option->part()->ignoresPartMaximum()) {
          if (piecesPlaced >= m_def->maxParts())
            continue;

          if ((partPos - origin).magnitude() > m_def->maxRadius()) {
            Logger::debug("out of range. {} ... {}", partPos, origin);
            continue;
          }
        }
        if (!option->part()->allowsPlacement(placementCounter[option->part()->name()])) {
          Logger::debug("part failed in allowsPlacement");
          continue;
        }
        if (!option->part()->checkPartCombinationsAllowed(placementCounter)) {
          Logger::debug("part failed in checkPartCombinationsAllowed");
          continue;
        }
        if (option->part()->collidesWithPlaces(partPos, preserveTiles)) {
          Logger::debug("part failed in collidesWithPlaces");
          continue;
        }
        if (option->part()->minimumThreatLevel() && m_threatLevel < *option->part()->minimumThreatLevel()) {
          Logger::debug("part failed in minimumThreatLevel");
          continue;
        }
        if (option->part()->maximumThreatLevel() && m_threatLevel > *option->part()->maximumThreatLevel()) {
          Logger::debug("part failed in maximumThreatLevel");
          continue;
        }
        if (forcePlacement || option->part()->canPlace(partPos, writer)) {
          placePart(option->part(), partPos);
          closedConnectors.add(connectorPos);
          closedConnectors.add(optionPos);
          break;
        } else {
          Logger::debug("part failed in canPlace");
        }
      }
    }
  }
  Logger::debug("Settling dungeon water.");
  writer->flushLiquid();
  Logger::debug("Flushing dungeon into the worldgen.");
  writer->flush();

  return {writer->boundingBoxes(), modifiedTiles};
}

auto DungeonGenerator::pickAnchor() -> ConstPtr<Dungeon::Part> {
  auto validAnchors = m_def->anchors().filtered([this](String const& anchorName) -> bool {
    auto anchorPart = m_def->parts().get(anchorName);
    return (!anchorPart->minimumThreatLevel() || m_threatLevel >= *anchorPart->minimumThreatLevel())
      && (!anchorPart->maximumThreatLevel() || m_threatLevel <= *anchorPart->maximumThreatLevel());
  });

  if (validAnchors.empty())
    return {};

  return m_def->parts().get(m_rand.randFrom(validAnchors));
}

auto DungeonGenerator::findConnectablePart(ConstPtr<Dungeon::Connector> connector) const -> List<ConstPtr<Dungeon::Connector>> {
  List<ConstPtr<Dungeon::Connector>> result;
  for (auto const& partPair : m_def->parts()) {
    if (partPair.second->doesNotConnectTo(connector->part()))
      continue;
    for (auto const& connection : partPair.second->connections()) {
      if (connection->connectsTo(connector))
        result.append(connection);
    }
  }
  return result;
}

auto DungeonGenerator::definition() const -> ConstPtr<DungeonDefinition> {
  return m_def;
}

}// namespace Star
