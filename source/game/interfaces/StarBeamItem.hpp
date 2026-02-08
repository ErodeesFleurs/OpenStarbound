#pragma once

#include "StarGameTypes.hpp"
#include "StarNonRotatedDrawablesItem.hpp"
#include "StarSpline.hpp"
#include "StarToolUserItem.hpp"

namespace Star {

class BeamItem : public virtual NonRotatedDrawablesItem, public virtual ToolUserItem {
public:
  enum class EndType { Invalid = -1,
                       Object,
                       Tile,
                       TileGroup,
                       Wire };

  BeamItem(Json config);
  ~BeamItem() override = default;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  auto nonRotatedDrawables() const -> List<Drawable> override;

  virtual auto getAngle(float angle) -> float;
  virtual auto drawables() const -> List<Drawable>;
  virtual auto handPosition() const -> Vec2F;
  virtual auto firePosition() const -> Vec2F;
  virtual void setRange(float range);
  virtual auto getAppropriateOpacity() const -> float;
  virtual void setEnd(EndType type);

protected:
  auto beamDrawables(bool canPlace = true) const -> List<Drawable>;

  String m_image;
  StringList m_endImages;
  EndType m_endType;

  float m_segmentsPerUnit;
  float m_nearControlPointElasticity;
  float m_farControlPointElasticity;
  float m_nearControlPointDistance;
  Vec2F m_handPosition;
  Vec2F m_firePosition;
  float m_range;

  float m_targetSegmentRun;
  float m_minBeamWidth;
  float m_maxBeamWidth;
  float m_beamWidthDev;
  float m_minBeamJitter;
  float m_maxBeamJitter;
  float m_beamJitterDev;
  float m_minBeamTrans;
  float m_maxBeamTrans;
  float m_beamTransDev;
  int m_minBeamLines;
  int m_maxBeamLines;
  float m_innerBrightnessScale;
  float m_firstStripeThickness;
  float m_secondStripeThickness;
  Color m_color;

  mutable bool m_inRangeLastUpdate;
  mutable Color m_lastUpdateColor;
  mutable float m_particleGenerateCooldown;

  CSplineF m_beamCurve;
};

}// namespace Star
