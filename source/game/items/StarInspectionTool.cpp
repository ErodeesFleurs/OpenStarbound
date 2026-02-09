#include "StarInspectionTool.hpp"

#include "StarCasting.hpp"
#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLogging.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

InspectionTool::InspectionTool(Json const& config, String const& directory, Json const& parameters)
    : Item(config, directory, parameters) {
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

auto InspectionTool::clone() const -> Ptr<Item> {
  return std::make_shared<InspectionTool>(*this);
}

void InspectionTool::update(float, FireMode fireMode, bool, HashSet<MoveControlType> const&) {
  m_currentAngle = world()->geometry().diff(owner()->aimPosition(), owner()->position()).angle();
  m_currentPosition = owner()->position() + owner()->handPosition(hand(), m_lightPosition - m_handPosition);
  SpatialLogger::logPoint("world", m_currentPosition, {0, 0, 255, 255});

  if (fireMode != m_lastFireMode) {
    if (fireMode != FireMode::None)
      m_inspectionResults.append(inspect(owner()->aimPosition()));
  }

  m_lastFireMode = fireMode;
}

auto InspectionTool::drawables() const -> List<Drawable> {
  return {Drawable::makeImage(m_image, 1.0f / TilePixels, true, -m_handPosition)};
}

auto InspectionTool::lightSources() const -> List<LightSource> {
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

auto InspectionTool::inspectionHighlightLevel(Ptr<InspectableEntity> const& inspectable) const -> float {
  if (m_showHighlights)
    return inspectionLevel(inspectable);
  return 0;
}

auto InspectionTool::pullInspectionResults() -> List<InspectionTool::InspectionResult> {
  return Star::take(m_inspectionResults);
}

auto InspectionTool::inspectionLevel(Ptr<InspectableEntity> const& inspectable) const -> float {
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
    return std::clamp(totalLevel / std::min(spaceSet.size(), m_fullInspectionSpaces), 0.0f, 1.0f);
  } else
    return pointInspectionLevel(inspectable->position());
}

auto InspectionTool::pointInspectionLevel(Vec2F const& position) const -> float {
  Vec2F gdiff = world()->geometry().diff(position, m_currentPosition);
  float gdist = gdiff.magnitude();
  float angleFactor = (std::abs(angleDiff(gdiff.angle(), m_currentAngle)) - m_inspectionAngles[0]) / (m_inspectionAngles[1] - m_inspectionAngles[0]);
  float distFactor = (gdist - m_inspectionRanges[0]) / (m_inspectionRanges[1] - m_inspectionRanges[0]);
  float ambientFactor = gdist / m_ambientInspectionRadius;
  return 1 - std::clamp(std::max(distFactor, std::min(ambientFactor, angleFactor)), 0.0f, 1.0f);
}

auto InspectionTool::hasLineOfSight(Vec2I const& position, Set<Vec2I> const& targetSpaces) const -> bool {
  if (!m_requireLineOfSight)
    return true;
  auto collisions = world()->collidingTilesAlongLine(centerOfTile(m_currentPosition), centerOfTile(position));
  for (auto collision : collisions) {
    if (collision != position && !targetSpaces.contains(collision))
      return false;
  }
  return true;
}

auto InspectionTool::inspect(Vec2F const& position) -> InspectionTool::InspectionResult {
  auto assets = Root::singleton().assets();
  auto species = owner()->species();

  // if there's a candidate InspectableEntity at the position, make sure that entity's total inspection level
  // is above the minimum threshold
  auto check = [&](Ptr<InspectableEntity> entity) -> std::optional<InspectionTool::InspectionResult> {
    if (m_inspectableTypeFilter && !m_inspectableTypeFilter->contains(entity->entityType()))
      return {};
    if (entity->inspectable() && inspectionLevel(entity) >= m_minimumInspectionLevel) {
      if (m_allowScanning)
        return {{.message = entity->inspectionDescription(species).value(), .objectName = entity->inspectionLogName(), .entityId = entity->entityId()}};
      else
        return {{.message = entity->inspectionDescription(species).value(), .objectName = {}, .entityId = {}}};
    }
    return {};
  };

  WorldGeometry geometry = world()->geometry();
  for (auto& entity : world()->query<InspectableEntity>(RectF::withCenter(position, Vec2F()), [&](Ptr<InspectableEntity> const& entity) -> bool {
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
    return {.message = inspectionFailureText("outOfRangeText", species), .objectName = {}};

  // check the tile for foreground mod or material
  MaterialId fgMaterial = world()->material(Vec2I::floor(position), TileLayer::Foreground);
  MaterialId fgMod = world()->mod(Vec2I(position.floor()), TileLayer::Foreground);
  ConstPtr<MaterialDatabase> materialDatabase = Root::singleton().materialDatabase();
  if (isRealMaterial(fgMaterial)) {
    if (isRealMod(fgMod))
      return {.message = materialDatabase->modDescription(fgMod, species), .objectName = {}};
    else
      return {.message = materialDatabase->materialDescription(fgMaterial, species), .objectName = {}};
  }

  // check for liquid at the tile
  auto liquidLevel = world()->liquidLevel(Vec2I::floor(position));
  ConstPtr<LiquidsDatabase> liquidsDatabase = Root::singleton().liquidsDatabase();
  if (liquidLevel.liquid != EmptyLiquidId)
    return {.message = liquidsDatabase->liquidDescription(liquidLevel.liquid, species), .objectName = {}};

  // check the tile for background mod or material
  MaterialId bgMaterial = world()->material(Vec2I::floor(position), TileLayer::Background);
  MaterialId bgMod = world()->mod(Vec2I(position.floor()), TileLayer::Background);
  if (isRealMaterial(bgMaterial)) {
    if (isRealMod(bgMod))
      return {.message = materialDatabase->modDescription(bgMod, species), .objectName = {}};
    else
      return {.message = materialDatabase->materialDescription(bgMaterial, species), .objectName = {}};
  }

  // at this point you're just staring into the void
  return {.message = inspectionFailureText("nothingThereText", species), .objectName = {}};
}

auto InspectionTool::inspectionFailureText(String const& failureType, String const& species) const -> String {
  JsonArray textOptions;
  Json nothingThere = instanceValue(failureType);
  if (nothingThere.contains(species))
    textOptions = nothingThere.getArray(species);
  else
    textOptions = nothingThere.getArray("default");
  return textOptions.wrap(Random::randu64()).toString();
}

}// namespace Star
