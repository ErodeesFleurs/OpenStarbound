#include "StarPoly.hpp"
import star.world_geometry;
#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"

#include "StarWorldClient.hpp"

import star.world_camera;


import star.quest_indicator_painter;
#include "StarRoot.hpp"
#include "StarAssets.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarInputEvent.hpp"
#include "StarThread.hpp"
#include "StarVector.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
#include "StarUniverseClient.hpp"

import star.animation;
import star.quest_manager;

namespace Star {

QuestIndicatorPainter::QuestIndicatorPainter(UniverseClientPtr const& client) {
  m_client = client;
}

AnimationPtr indicatorAnimation(String indicatorPath) {
  auto assets = Root::singleton().assets();
  return make_shared<Animation>(assets->json(indicatorPath), indicatorPath);
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
  auto& context = GuiContext::singleton();

  for (auto const& indicator : m_indicators.values()) {
    Drawable drawable = indicator.render(m_camera.pixelRatio());
    drawable.fullbright = true;
    context.drawDrawable(drawable, Vec2F(indicator.screenPos), 1, Vec4B(255, 255, 255, 255));
  }
}

}
