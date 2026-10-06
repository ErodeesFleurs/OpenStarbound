module;

#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"




import star.world_geometry;
import star.world_camera;
import star.drawable;

namespace Star {
STAR_CLASS(WorldClient);
}

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

export module star.quest_indicator_painter;

export namespace Star {
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::QuestIndicatorPainter;
  using ::Star::QuestIndicatorPainterPtr;
  using ::Star::QuestIndicatorPainterConstPtr;
  using ::Star::QuestIndicatorPainterWeakPtr;
  using ::Star::QuestIndicatorPainterConstWeakPtr;
  using ::Star::QuestIndicatorPainterUPtr;
  using ::Star::QuestIndicatorPainterConstUPtr;
  using ::Star::Animation;
  using ::Star::AnimationPtr;
  using ::Star::AnimationConstPtr;
  using ::Star::AnimationWeakPtr;
  using ::Star::AnimationConstWeakPtr;
  using ::Star::AnimationUPtr;
  using ::Star::AnimationConstUPtr;
}
