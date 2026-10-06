#pragma once
#include "StarPoly.hpp"
import star.world_geometry;
#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"

#include "StarWorldClient.hpp"

import star.world_camera;

namespace Star {

STAR_CLASS(UniverseClient);
STAR_CLASS(QuestIndicatorPainter);
STAR_CLASS(Animation);

class QuestIndicatorPainter {
public:
  QuestIndicatorPainter(UniverseClientPtr const& client);

  void update(float dt, WorldClientPtr const& world, WorldCamera const& camera);
  void render();

private:
  struct Indicator {
    Drawable render(float pixelRatio) const;

    EntityId entityId;
    Vec2F screenPos;
    String indicatorName;
    AnimationPtr animation;
  };

  UniverseClientPtr m_client;
  WorldCamera m_camera;
  Map<EntityId, Indicator> m_indicators;
};

}
