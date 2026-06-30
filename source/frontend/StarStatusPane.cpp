#include "StarStatusPane.hpp"
#include "StarAlgorithm.hpp"
#include "StarJsonExtra.hpp"
#include "StarUniverseClient.hpp"
#include "StarGuiReader.hpp"
#include "StarImageWidget.hpp"
#include "StarPlayer.hpp"
#include "StarAssets.hpp"
#include "StarStatusEffectDatabase.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarImageProcessing.hpp"
#include "StarSimpleTooltip.hpp"

namespace Star {

StatusPane::StatusPane(UniverseClientPtr client, StatusPaneServices services)
  : Pane(services.guiContext),
    m_client(std::move(client)),
    m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "StatusPane", "assets")),
    m_imageMetadataDatabase(requireServiceValueAs<StarException>(std::move(services.imageMetadataDatabase), "StatusPane", "image metadata")),
    m_statusEffectDatabase(requireServiceValueAs<StarException>(std::move(services.statusEffectDatabase), "StatusPane", "status effect database")),
    m_guiContext(services.guiContext) {
  m_player = m_client->mainPlayer();

  GuiReader reader(m_guiContext);
  reader.construct(m_assets->json("/interface/windowconfig/statuspane.config:paneLayout"), this);
  disableScissoring();
}

PanePtr StatusPane::createTooltip(Vec2I const& screenPosition) {
  auto interfaceScale = m_guiContext.interfaceScale();
  for (auto const& indicator : m_statusIndicators) {
    if (indicator.screenRect.contains(Vec2F(screenPosition * interfaceScale))) {
      if (!indicator.label.empty())
        return SimpleTooltipBuilder::buildTooltip(indicator.label, SimpleTooltipServices{m_assets, m_guiContext});
    }
  }
  return {};
}

void StatusPane::renderImpl() {
  Pane::renderImpl();

  auto interfaceScale = m_guiContext.interfaceScale();

  String statusIconDarkenImage = m_assets->json("/interface.config:statusIconDarkenImage").toString();

  for (auto const& entry : m_statusIndicators) {
    String image = entry.icon;
    if (entry.durationPercentage) {
      int imageHeight = m_imageMetadataDatabase->imageSize(image)[1];
      int yOffset = -(int)(*entry.durationPercentage * imageHeight);
      image += "?" + imageOperationToString(BlendImageOperation{
                         BlendImageOperation::Multiply, {statusIconDarkenImage}, Vec2I(0, yOffset)});
    }
    m_guiContext.drawQuad(image, entry.screenRect.min(), interfaceScale);
  }
}

void StatusPane::update(float dt) {
  Pane::update(dt);

  auto interfaceScale = m_guiContext.interfaceScale();
  int roundWindowHeight = ceil(windowHeight() / interfaceScale) * interfaceScale;

  Vec2I statusIconOffset = jsonToVec2I(m_assets->json("/interface.config:statusIconPos"));
  Vec2I statusIconPos = Vec2I(statusIconOffset[0] * interfaceScale, roundWindowHeight - statusIconOffset[1] * interfaceScale);
  Vec2I statusIconShift = jsonToVec2I(m_assets->json("/interface.config:statusIconShift")) * interfaceScale;

  RectF boundRect = RectF::null();

  m_statusIndicators.clear();
  for (auto const& pair : m_player->activeUniqueStatusEffectSummary()) {
    auto effectConfig = m_statusEffectDatabase->uniqueEffectConfig(pair.first);
    if (effectConfig.icon) {
      RectF rect = RectF::withSize(Vec2F(statusIconPos), Vec2F(m_imageMetadataDatabase->imageSize(*effectConfig.icon)) * interfaceScale);
      boundRect.combine(rect);
      m_statusIndicators.append(StatusEffectIndicator{*effectConfig.icon, pair.second, effectConfig.label, rect});
      statusIconPos += statusIconShift;
    }
  }

  setPosition(Vec2I::round(boundRect.min() / interfaceScale));
  setSize(Vec2I::round(boundRect.size() / interfaceScale));
}

}
