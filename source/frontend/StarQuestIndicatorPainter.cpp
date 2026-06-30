#include "StarQuestIndicatorPainter.hpp"
#include "StarAlgorithm.hpp"
#include "StarAssets.hpp"
#include "StarGuiContext.hpp"
#include "StarQuestManager.hpp"
#include "StarWorldClient.hpp"
#include "StarUniverseClient.hpp"

namespace Star {

QuestIndicatorPainter::QuestIndicatorPainter(UniverseClientPtr const& client, Services services)
  : m_client(client), m_assets(std::move(services.assets)), m_guiContext(services.guiContext) {
  requireNotNull(m_assets, "QuestIndicatorPainter", "assets");
}

AnimationPtr QuestIndicatorPainter::indicatorAnimation(String const& indicatorPath) const {
  return make_shared<Animation>(m_assets->json(indicatorPath), indicatorPath, m_assets, m_guiContext.imageMetadata());
}

void QuestIndicatorPainter::update(float dt, WorldClientPtr const& world, WorldCamera const& camera) {
  m_camera = camera;

  Set<EntityId> foundIndicators;
  for (auto const& entity : world->query<Entity>(camera.worldScreenRect())) {
    auto indicator = m_client->questManager()->getQuestIndicator(entity);
    if (!indicator) continue;

    foundIndicators.insert(entity->entityId());
    Vec2F screenPos = camera.worldToScreen(indicator->worldPosition);

    if (auto currentIndicator = m_indicators.ptr(entity->entityId())) {
      currentIndicator->screenPos = screenPos;
      if (currentIndicator->indicatorName == indicator->indicatorImage) {
        currentIndicator->animation->update(dt);
      } else {
        currentIndicator->indicatorName = indicator->indicatorImage;
        currentIndicator->animation = indicatorAnimation(indicator->indicatorImage);
      }
    } else {
      m_indicators[entity->entityId()] = Indicator {
          entity->entityId(),
          screenPos,
          indicator->indicatorImage,
          indicatorAnimation(indicator->indicatorImage)
        };
    }
  }

  m_indicators = Map<EntityId, Indicator>::from(m_indicators.pairs().filtered([&foundIndicators](pair<EntityId, Indicator> indicator) {
      return foundIndicators.contains(indicator.first);
    }));
}

Drawable QuestIndicatorPainter::Indicator::render(float pixelRatio) const {
  return animation->drawable(pixelRatio);
}

void QuestIndicatorPainter::render() {
  for (auto const& indicator : m_indicators.values()) {
    Drawable drawable = indicator.render(m_camera.pixelRatio());
    drawable.fullbright = true;
    m_guiContext.drawDrawable(drawable, Vec2F(indicator.screenPos), 1, Vec4B(255, 255, 255, 255));
  }
}

}
