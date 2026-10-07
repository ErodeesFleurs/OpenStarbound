#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"
#include "StarRect.hpp"
#include "StarList.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarJsonExtra.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
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
#include "StarMap.hpp"
import star.world_geometry;

#include "StarWorldClient.hpp"



import star.chat_bubble_separation;
#include "StarRenderer.hpp"
import star.font_texture_group;
import star.anchor_types;
#include "StarRoot.hpp"
import star.text_painter;

import star.world_camera;


import star.nameplate_painter;
#include "StarAssets.hpp"
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.nametag_entity;
#include "StarPlayer.hpp"
#include "StarApplicationController.hpp"
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;

namespace Star {

NameplatePainter::NameplatePainter() {
  auto assets = Root::singleton().assets();

  Json nametagConfig = assets->json("/interface.config:nametag");
  m_showMasterNames = nametagConfig.getBool("showMasterNames");
  m_opacityRate = nametagConfig.getFloat("opacityRate");
  m_inspectOpacityRate = nametagConfig.queryFloat("inspectOpacityRate", m_opacityRate);
  m_offset = jsonToVec2F(nametagConfig.get("offset"));
  Json textStyle = nametagConfig.get("textStyle");
  m_textStyle = textStyle;
  m_statusTextStyle = nametagConfig.get("statusTextStyle", textStyle);
  m_statusOffset = jsonToVec2F(nametagConfig.get("statusOffset"));
  m_statusTextStyle.color = jsonToColor(nametagConfig.get("statusColor")).toRgba();
  m_opacityBoost = nametagConfig.getFloat("opacityBoost");
  m_nametags.setTweenFactor(nametagConfig.getFloat("tweenFactor"));
  m_nametags.setMovementThreshold(nametagConfig.getFloat("movementThreshold"));
}

void NameplatePainter::update(float dt, WorldClientPtr const& world, WorldCamera const& camera, bool inspectionMode) {
  m_camera = camera;

  Set<EntityId> foundEntities;
  for (auto const& entity : world->query<NametagEntity>(camera.worldScreenRect())) {
    if ((entity->isMaster() && !m_showMasterNames) || !entity->displayNametag())
      continue;
    if (auto player = as<Player>(entity)) {
      if (player->isTeleporting())
        continue;
    }
    foundEntities.insert(entity->entityId());

    if (!m_entitiesWithNametags.contains(entity->entityId())) {
      Nametag nametag = {entity->nametag(), entity->statusText(), entity->nametagColor(), 1.0f, entity->entityId()};
      RectF boundBox = determineBoundBox(Vec2F(), nametag);
      m_nametags.addBubble(Vec2F(), boundBox, std::move(nametag));
    }
  }

  m_nametags.filter([&foundEntities](
      BubbleState<Nametag> const&, Nametag const& nametag) { return foundEntities.contains(nametag.entityId); });

  m_nametags.forEach([&world, &camera, this, inspectionMode](BubbleState<Nametag>& bubbleState, Nametag& nametag) {
    if (auto entity = as<NametagEntity>(world->entity(nametag.entityId))) {
      bubbleState.idealDestination = camera.worldToScreen(entity->nametagOrigin()) + m_offset * camera.pixelRatio();
      bubbleState.boundBox = determineBoundBox(bubbleState.idealDestination, nametag);

      nametag.statusText = entity->statusText();
      nametag.name = entity->nametag();
      nametag.color = entity->nametagColor();
      bool fullyOnScreen = world->geometry().rectContains(camera.worldScreenRect(), entity->position());
      if (inspectionMode)
        nametag.opacity = approach(1.0f, nametag.opacity, m_inspectOpacityRate);
      else if (fullyOnScreen)
        nametag.opacity = approach(0.0f, nametag.opacity, m_opacityRate);
      else
        nametag.opacity = approach(m_opacityBoost, nametag.opacity, m_opacityRate);
    }
  });

  m_entitiesWithNametags = std::move(foundEntities);
  m_nametags.update(dt);
}

void NameplatePainter::render() {
  auto& context = GuiContext::singleton();

  m_nametags.forEach([&context, this](BubbleState<Nametag> const& bubble, Nametag const& nametag) {
    if (nametag.opacity == 0.0f)
      return;

    auto& setStyle = context.setTextStyle(m_textStyle);
    auto color = Color::rgb(nametag.color);
    color.setAlphaF(nametag.opacity);
    setStyle.color = color.toRgba();

    context.renderText(nametag.name, namePosition(bubble.currentPosition));

    if (nametag.statusText) {
      context.setTextStyle(m_statusTextStyle).color[3] *= nametag.opacity;
      context.renderText(*nametag.statusText, statusPosition(bubble.currentPosition));
    }
    context.clearTextStyle();
  });
}

TextPositioning NameplatePainter::namePosition(Vec2F bubblePosition) const {
  return TextPositioning(bubblePosition, HorizontalAnchor::HMidAnchor, VerticalAnchor::BottomAnchor);
}

TextPositioning NameplatePainter::statusPosition(Vec2F bubblePosition) const {
  auto& context = GuiContext::singleton();
  return TextPositioning(
      bubblePosition + m_statusOffset * context.interfaceScale(),
      HorizontalAnchor::HMidAnchor, VerticalAnchor::BottomAnchor);
}

RectF NameplatePainter::determineBoundBox(Vec2F bubblePosition, Nametag const& nametag) const {
  auto& context = GuiContext::singleton();
  context.setTextStyle(m_textStyle);
  RectF nametagBox = context.determineTextSize(nametag.name, namePosition(bubblePosition));
  if (nametag.statusText) {
    context.setTextStyle(m_statusTextStyle);
    nametagBox.combine(context.determineTextSize(*nametag.statusText, statusPosition(bubblePosition)));
  }
  context.clearTextStyle();
  return nametagBox;
}

}
