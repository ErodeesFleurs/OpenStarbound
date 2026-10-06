#include "StarAssetPath.hpp"

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarRoot.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
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
import star.widget;
import star.pane;

#include "StarOrderedMap.hpp"
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;


import star.main_interface_types;


import star.status_pane;
#include "StarJsonExtra.hpp"
#include "StarUniverseClient.hpp"
import star.widget_parsing;
import star.gui_reader;
import star.image_widget;
#include "StarPlayer.hpp"
#include "StarAssets.hpp"
#include "StarImageProcessing.hpp"

import star.simple_tooltip;
import star.status_effect_database;
import star.image_metadata_database;

namespace Star {

StatusPane::StatusPane(MainInterfacePaneManager* paneManager, UniverseClientPtr client) {
  m_paneManager = paneManager;
  m_client = client;
  m_player = m_client->mainPlayer();

  m_guiContext = GuiContext::singletonPtr();
  auto assets = Root::singleton().assets();

  GuiReader reader;
  reader.construct(assets->json("/interface/windowconfig/statuspane.config:paneLayout"), this);
  disableScissoring();
}

PanePtr StatusPane::createTooltip(Vec2I const& screenPosition) {
  auto interfaceScale = m_guiContext->interfaceScale();
  for (auto const& indicator : m_statusIndicators) {
    if (indicator.screenRect.contains(Vec2F(screenPosition * interfaceScale))) {
      if (!indicator.label.empty())
        return SimpleTooltipBuilder::buildTooltip(indicator.label);
    }
  }
  return {};
}

void StatusPane::renderImpl() {
  Pane::renderImpl();

  auto assets = Root::singleton().assets();
  auto interfaceScale = m_guiContext->interfaceScale();
  auto imageMetadataDatabase = Root::singleton().imageMetadataDatabase();

  String statusIconDarkenImage = assets->json("/interface.config:statusIconDarkenImage").toString();

  for (auto const& entry : m_statusIndicators) {
    String image = entry.icon;
    if (entry.durationPercentage) {
      int imageHeight = imageMetadataDatabase->imageSize(image)[1];
      int yOffset = -(int)(*entry.durationPercentage * imageHeight);
      image += "?" + imageOperationToString(BlendImageOperation{
                         BlendImageOperation::Multiply, {statusIconDarkenImage}, Vec2I(0, yOffset)});
    }
    m_guiContext->drawQuad(image, entry.screenRect.min(), interfaceScale);
  }
}

void StatusPane::update(float dt) {
  Pane::update(dt);

  auto assets = Root::singleton().assets();
  auto interfaceScale = m_guiContext->interfaceScale();
  int roundWindowHeight = ceil(windowHeight() / interfaceScale) * interfaceScale;

  auto imageMetadataDatabase = Root::singleton().imageMetadataDatabase();
  auto statusEffectDatabase = Root::singleton().statusEffectDatabase();

  Vec2I statusIconOffset = jsonToVec2I(assets->json("/interface.config:statusIconPos"));
  Vec2I statusIconPos = Vec2I(statusIconOffset[0] * interfaceScale, roundWindowHeight - statusIconOffset[1] * interfaceScale);
  Vec2I statusIconShift = jsonToVec2I(assets->json("/interface.config:statusIconShift")) * interfaceScale;

  RectF boundRect = RectF::null();

  m_statusIndicators.clear();
  for (auto const& pair : m_player->activeUniqueStatusEffectSummary()) {
    auto effectConfig = statusEffectDatabase->uniqueEffectConfig(pair.first);
    if (effectConfig.icon) {
      RectF rect = RectF::withSize(Vec2F(statusIconPos), Vec2F(imageMetadataDatabase->imageSize(*effectConfig.icon)) * interfaceScale);
      boundRect.combine(rect);
      m_statusIndicators.append(StatusEffectIndicator{*effectConfig.icon, pair.second, effectConfig.label, rect});
      statusIconPos += statusIconShift;
    }
  }

  setPosition(Vec2I::round(boundRect.min() / interfaceScale));
  setSize(Vec2I::round(boundRect.size() / interfaceScale));
}

}
