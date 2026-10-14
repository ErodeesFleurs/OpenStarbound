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
using WorldClientPtr = std::shared_ptr<WorldClient>;
using WorldClientConstPtr = std::shared_ptr<WorldClient const>;
using WorldClientWeakPtr = std::weak_ptr<WorldClient>;
using WorldClientConstWeakPtr = std::weak_ptr<WorldClient const>;
using WorldClientUPtr = std::unique_ptr<WorldClient>;
using WorldClientConstUPtr = std::unique_ptr<WorldClient const>;
}

namespace Star {

class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
class QuestIndicatorPainter;
using QuestIndicatorPainterPtr = std::shared_ptr<QuestIndicatorPainter>;
using QuestIndicatorPainterConstPtr = std::shared_ptr<QuestIndicatorPainter const>;
using QuestIndicatorPainterWeakPtr = std::weak_ptr<QuestIndicatorPainter>;
using QuestIndicatorPainterConstWeakPtr = std::weak_ptr<QuestIndicatorPainter const>;
using QuestIndicatorPainterUPtr = std::unique_ptr<QuestIndicatorPainter>;
using QuestIndicatorPainterConstUPtr = std::unique_ptr<QuestIndicatorPainter const>;
class Animation;
using AnimationPtr = std::shared_ptr<Animation>;
using AnimationConstPtr = std::shared_ptr<Animation const>;
using AnimationWeakPtr = std::weak_ptr<Animation>;
using AnimationConstWeakPtr = std::weak_ptr<Animation const>;
using AnimationUPtr = std::unique_ptr<Animation>;
using AnimationConstUPtr = std::unique_ptr<Animation const>;

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
