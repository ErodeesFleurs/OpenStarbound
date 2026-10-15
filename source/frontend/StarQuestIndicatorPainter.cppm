module;

import std;
import star.string;
import star.map;
import star.vector;
import star.game_types;




import star.world_geometry;
import star.world_camera;
import star.drawable;

namespace Star {
class WorldClient;
}

namespace Star {

class UniverseClient;
class Animation;

class QuestIndicatorPainter {
public:
  QuestIndicatorPainter(std::shared_ptr<UniverseClient> const& client);

  void update(float dt, std::shared_ptr<WorldClient> const& world, WorldCamera const& camera);
  void render();

private:
  struct Indicator {
    Drawable render(float pixelRatio) const;

    EntityId entityId;
    Vec2F screenPos;
    String indicatorName;
    std::shared_ptr<Animation> animation;
  };

  std::shared_ptr<UniverseClient> m_client;
  WorldCamera m_camera;
  Map<EntityId, Indicator> m_indicators;
};

}

export module star.quest_indicator_painter;

export namespace Star {
  using ::Star::UniverseClient;
  using ::Star::QuestIndicatorPainter;
  using ::Star::Animation;
}
