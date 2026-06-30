#pragma once

#include "StarIAssets.hpp"
#include "StarWorldCamera.hpp"
#include "StarWorldClient.hpp"

namespace Star {

class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;
class QuestIndicatorPainter;
using QuestIndicatorPainterPtr = SharedPtr<QuestIndicatorPainter>;

class QuestIndicatorPainter {
public:
  struct Services {
    IAssetsConstPtr assets;
  };

  QuestIndicatorPainter(UniverseClientPtr const& client, Services services = {});

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

  AnimationPtr indicatorAnimation(String const& indicatorPath) const;

  UniverseClientPtr m_client;
  IAssetsConstPtr m_assets;
  WorldCamera m_camera;
  Map<EntityId, Indicator> m_indicators;
};

}
