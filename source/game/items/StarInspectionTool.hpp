#pragma once

#include "StarConfig.hpp"
#include "StarInspectableEntity.hpp"
#include "StarItem.hpp"
#include "StarPointableItem.hpp"
#include "StarToolUserItem.hpp"

import std;

namespace Star {

class InspectionTool
    : public Item,
      public PointableItem,
      public ToolUserItem {
public:
  struct InspectionResult {
    String message;
    std::optional<String> objectName = {};
    std::optional<EntityId> entityId = {};
  };

  InspectionTool(Json const& config, String const& directory, Json const& parameters = JsonObject());

  [[nodiscard]] auto clone() const -> Ptr<Item> override;

  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  [[nodiscard]] auto drawables() const -> List<Drawable> override;

  [[nodiscard]] auto lightSources() const -> List<LightSource>;

  [[nodiscard]] auto inspectionHighlightLevel(Ptr<InspectableEntity> const& inspectableEntity) const -> float;

  auto pullInspectionResults() -> List<InspectionResult>;

private:
  auto inspect(Vec2F const& position) -> InspectionResult;

  [[nodiscard]] auto inspectionLevel(Ptr<InspectableEntity> const& inspectableEntity) const -> float;
  [[nodiscard]] auto pointInspectionLevel(Vec2F const& position) const -> float;
  [[nodiscard]] auto hasLineOfSight(Vec2I const& targetPosition, Set<Vec2I> const& targetSpaces = {}) const -> bool;

  [[nodiscard]] auto inspectionFailureText(String const& failureType, String const& species) const -> String;

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
  size_t m_fullInspectionSpaces;
  float m_minimumInspectionLevel;
  std::optional<HashSet<EntityType>> m_inspectableTypeFilter;

  FireMode m_lastFireMode;
  List<InspectionResult> m_inspectionResults;
};

}// namespace Star
