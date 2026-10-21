module;
import std;
import star.type_identity;
import star.json;
import star.json_extra;
import star.math_common;
import star.algorithm;
import star.random;
import star.id_map;
import star.list;
import star.bi_map;
import star.data_stream;
import star.casting;
import star.strong_typedef;
import star.rpc_promise;
import star.rect;
import star.a_star;
import star.audio;
import star.thread;
import star.map;
import star.set;
import star.vector;
import star.color;
import star.string;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.game_types;
import star.liquid_types;
import star.material_types;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;


import star.collision_block;
import star.item_descriptor;
import star.item;
import star.pointable_item;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.tile_modification;
import star.lua_root;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;
import star.game_timers;
import star.actor_movement_controller;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.tool_user_item;
import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;
import star.inspectable_entity;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

export module star.inspection_tool;
import star.liquids_database;
import star.material_database;


export namespace Star {

class InspectionTool
  : public Item,
    public PointableItem,
    public ToolUserItem {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<InspectionTool, Item, PointableItem, ToolUserItem>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<InspectionTool>();
  }


  struct InspectionResult {
    String message;
    std::optional<String> objectName = {};
    std::optional<EntityId> entityId = {};
  };

  InspectionTool(Root& root, Json const& config, String const& directory, Json const& parameters = JsonObject());

  std::shared_ptr<Item> clone() const override;

  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  List<Drawable> drawables() const override;

  List<LightSource> lightSources() const;

  float inspectionHighlightLevel(std::shared_ptr<InspectableEntity> const& inspectableEntity) const;

  List<InspectionResult> pullInspectionResults();

private:
  InspectionResult inspect(Vec2F const& position);

  float inspectionLevel(std::shared_ptr<InspectableEntity> const& inspectableEntity) const;
  float pointInspectionLevel(Vec2F const& position) const;
  bool hasLineOfSight(Vec2I const& targetPosition, Set<Vec2I> const& targetSpaces = {}) const;

  String inspectionFailureText(String const& failureType, String const& species) const;

  float m_currentAngle;
  Vec2F m_currentPosition;

  String m_image;
  Vec2F m_handPosition;
  Vec2F m_lightPosition;
  Color m_lightColor;
  float m_beamWidth;
  float m_ambientFactor;

  bool m_showHighlights;
  bool m_allowScanning;
  bool m_requireLineOfSight;

  Vec2F m_inspectionAngles;
  Vec2F m_inspectionRanges;
  float m_ambientInspectionRadius;
  std::size_t m_fullInspectionSpaces;
  float m_minimumInspectionLevel;
  std::optional<HashSet<EntityType>> m_inspectableTypeFilter;

  FireMode m_lastFireMode;
  List<InspectionResult> m_inspectionResults;
};

}

namespace Star {

InspectionTool::InspectionTool(Root& root, Json const& config, String const& directory, Json const& parameters)
  : Item(root, config, directory, parameters) {
  m_image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  m_handPosition = jsonToVec2F(instanceValue("handPosition"));
  m_lightPosition = jsonToVec2F(instanceValue("lightPosition"));
  m_lightColor = jsonToColor(instanceValue("lightColor"));
  m_beamWidth = instanceValue("beamLevel").toFloat();
  m_ambientFactor = instanceValue("beamAmbience").toFloat();

  m_showHighlights = instanceValue("showHighlights").toBool();
  m_allowScanning = instanceValue("allowScanning").toBool();
  m_requireLineOfSight = instanceValue("requireLineOfSight", true) == Json(true);
  m_inspectionAngles = jsonToVec2F(instanceValue("inspectionAngles"));
  m_inspectionRanges = jsonToVec2F(instanceValue("inspectionRanges"));
  m_ambientInspectionRadius = instanceValue("ambientInspectionRadius").toFloat();
  m_fullInspectionSpaces = instanceValue("fullInspectionSpaces").toUInt();
  m_minimumInspectionLevel = instanceValue("minimumInspectionLevel").toFloat();
  if (auto typeFilter = instanceValue("inspectableTypeFilter"); typeFilter.isType(Json::Type::Array)) {
    m_inspectableTypeFilter.emplace();
    for (auto& str : typeFilter.toArray()) {
      if (!str.isType(Json::Type::String))
        continue;
      if (auto entityType = EntityTypeNames.leftPtr(str.toString()))
        m_inspectableTypeFilter->add(*entityType);
    }
  }

  m_lastFireMode = FireMode::None;
}

std::shared_ptr<Item> InspectionTool::clone() const {
  return std::make_shared<InspectionTool>(*this);
}

void InspectionTool::update(float, FireMode fireMode, bool, HashSet<MoveControlType> const&) {
  m_currentAngle = world()->geometry().diff(owner()->aimPosition(), owner()->position()).angle();
  m_currentPosition = owner()->position() + owner()->handPosition(hand(), m_lightPosition - m_handPosition);
  root().spatialLogger().logPoint("world", m_currentPosition, {0, 0, 255, 255});

  if (fireMode != m_lastFireMode) {
    if (fireMode != FireMode::None)
      m_inspectionResults.append(inspect(owner()->aimPosition()));
  }

  m_lastFireMode = fireMode;
}

List<Drawable> InspectionTool::drawables() const {
  return {Drawable::makeImage(*root().imageMetadataDatabase(), m_image, 1.0f / TilePixels, true, -m_handPosition)};
}

List<LightSource> InspectionTool::lightSources() const {
  if (!initialized())
    return {};

  float angle = world()->geometry().diff(owner()->aimPosition(), owner()->position()).angle();
  LightSource lightSource;
  lightSource.type = LightType::Point;
  lightSource.position = owner()->position() + owner()->handPosition(hand(), m_lightPosition - m_handPosition);
  lightSource.color = m_lightColor.toRgbF();
  lightSource.pointBeam = m_beamWidth;
  lightSource.beamAngle = angle;
  lightSource.beamAmbience = m_ambientFactor;
  return {std::move(lightSource)};
}

float InspectionTool::inspectionHighlightLevel(std::shared_ptr<InspectableEntity> const& inspectable) const {
  if (m_showHighlights)
    return inspectionLevel(inspectable);
  return 0;
}

List<InspectionTool::InspectionResult> InspectionTool::pullInspectionResults() {
  return Star::take(m_inspectionResults);
}

float InspectionTool::inspectionLevel(std::shared_ptr<InspectableEntity> const& inspectable) const {
  if (!initialized() || !inspectable->inspectable())
    return 0;

  if (m_inspectableTypeFilter && !m_inspectableTypeFilter->contains(inspectable->entityType()))
    return 0;

  if (auto tileEntity = as<TileEntity>(inspectable)) {
    float totalLevel = 0;

    // convert spaces to a set of world positions
    Set<Vec2I> spaceSet;
    for (auto space : tileEntity->spaces())
      spaceSet.add(tileEntity->tilePosition() + space);

    for (auto space : spaceSet) {
      float pointLevel = pointInspectionLevel(centerOfTile(space));
      if (pointLevel > 0 && hasLineOfSight(space, spaceSet))
        totalLevel += pointLevel;
    }
    return clamp(totalLevel / std::min(spaceSet.size(), m_fullInspectionSpaces), 0.0f, 1.0f);
  }
  else
    return pointInspectionLevel(inspectable->position());
}

float InspectionTool::pointInspectionLevel(Vec2F const& position) const {
  Vec2F gdiff = world()->geometry().diff(position, m_currentPosition);
  float gdist = gdiff.magnitude();
  float angleFactor = (std::abs(angleDiff(gdiff.angle(), m_currentAngle)) - m_inspectionAngles[0]) / (m_inspectionAngles[1] - m_inspectionAngles[0]);
  float distFactor = (gdist - m_inspectionRanges[0]) / (m_inspectionRanges[1] - m_inspectionRanges[0]);
  float ambientFactor = gdist / m_ambientInspectionRadius;
  return 1 - clamp(std::max(distFactor, std::min(ambientFactor, angleFactor)), 0.0f, 1.0f);
}

bool InspectionTool::hasLineOfSight(Vec2I const& position, Set<Vec2I> const& targetSpaces) const {
  if (!m_requireLineOfSight)
    return true;
  auto collisions = world()->collidingTilesAlongLine(centerOfTile(m_currentPosition), centerOfTile(position));
  for (auto collision : collisions) {
    if (collision != position && !targetSpaces.contains(collision))
      return false;
  }
  return true;
}

InspectionTool::InspectionResult InspectionTool::inspect(Vec2F const& position) {
  auto assets = root().assets();
  auto species = owner()->species();

  // if there's a candidate InspectableEntity at the position, make sure that entity's total inspection level
  // is above the minimum threshold
  auto check = [&](std::shared_ptr<InspectableEntity> entity) -> std::optional<InspectionTool::InspectionResult> {
    if (m_inspectableTypeFilter && !m_inspectableTypeFilter->contains(entity->entityType()))
      return {};
    if (entity->inspectable() && inspectionLevel(entity) >= m_minimumInspectionLevel) {
      if (m_allowScanning)
        return InspectionResult{entity->inspectionDescription(species).value_or(String{}), entity->inspectionLogName(), entity->entityId()};
      else
        return InspectionResult{entity->inspectionDescription(species).value_or(String{}), {}, {}};
    }
    return {};
  };


  WorldGeometry geometry = world()->geometry();
  for (auto& entity : world()->query<InspectableEntity>(RectF::withCenter(position, Vec2F()), [&](std::shared_ptr<InspectableEntity> const& entity) {
    if (entity->entityType() == EntityType::Object)
      return false;
    else if (!geometry.rectContains(entity->metaBoundBox().translated(entity->position()), position))
      return false;
    else {
      auto hitPoly = entity->hitPoly();
      return hitPoly && geometry.polyContains(*hitPoly, position);
    }
  })) {
    if (auto result = check(entity))
      return *result;
  }

  for (auto& entity : world()->atTile<InspectableEntity>(Vec2I::floor(position))) {
    if (auto result = check(entity))
      return *result;
  }

  // check the inspection level at the selected tile
  if (!hasLineOfSight(Vec2I::floor(position)) || pointInspectionLevel(centerOfTile(position)) < m_minimumInspectionLevel)
    return {inspectionFailureText("outOfRangeText", species), {}};

  // check the tile for foreground mod or material
  MaterialId fgMaterial = world()->material(Vec2I::floor(position), TileLayer::Foreground);
  MaterialId fgMod = world()->mod(Vec2I(position.floor()), TileLayer::Foreground);
  auto materialDatabase = root().materialDatabase();
  if (isRealMaterial(fgMaterial)) {
    if (isRealMod(fgMod))
      return {materialDatabase->modDescription(fgMod, species), {}};
    else
      return {materialDatabase->materialDescription(fgMaterial, species), {}};
  }

  // check for liquid at the tile
  auto liquidLevel = world()->liquidLevel(Vec2I::floor(position));
  auto liquidsDatabase = root().liquidsDatabase();
  if (liquidLevel.liquid != EmptyLiquidId)
    return {liquidsDatabase->liquidDescription(liquidLevel.liquid, species), {}};

  // check the tile for background mod or material
  MaterialId bgMaterial = world()->material(Vec2I::floor(position), TileLayer::Background);
  MaterialId bgMod = world()->mod(Vec2I(position.floor()), TileLayer::Background);
  if (isRealMaterial(bgMaterial)) {
    if (isRealMod(bgMod))
      return {materialDatabase->modDescription(bgMod, species), {}};
    else
      return {materialDatabase->materialDescription(bgMaterial, species), {}};
  }

  // at this point you're just staring into the void
  return {inspectionFailureText("nothingThereText", species), {}};
}

String InspectionTool::inspectionFailureText(String const& failureType, String const& species) const {
  JsonArray textOptions;
  Json nothingThere = instanceValue(failureType);
  if (nothingThere.contains(species))
    textOptions = nothingThere.getArray(species);
  else
    textOptions = nothingThere.getArray("default");
  return textOptions.wrap(root().random().randu64()).toString();
}

}
