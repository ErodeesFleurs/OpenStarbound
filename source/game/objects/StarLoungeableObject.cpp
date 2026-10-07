#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarIdMap.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarMathCommon.hpp"
#include "StarRandom.hpp"
import star.periodic;
#include "StarInterpolation.hpp"
import star.periodic_function;
#include "StarNetElementSystem.hpp"
#include "StarSet.hpp"
#include "StarLua.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarMaybe.hpp"
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;


#include "StarLuaRoot.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.animation;
import star.particle;
import star.mixer;
import star.networked_animator;
import star.entity_rendering;
import star.object;
import star.drawable;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.anchorable_entity;
import star.entity_rendering_types;
import star.lounging_entities;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.loungeable_object;
import star.object_database;


namespace Star {

LoungeableObject::LoungeableObject(ObjectConfigConstPtr config, Json const& parameters) : Object(config, parameters) {
  m_interactive.set(true);
}

void LoungeableObject::render(RenderCallback* renderCallback) {
  Object::render(renderCallback);

  if (!m_sitCoverImage.empty()) {
    if (!entitiesLounging().empty()) {
      if (auto orientation = currentOrientation()) {
        Drawable drawable =
            Drawable::makeImage(m_sitCoverImage, 1.0f / TilePixels, false, position() + orientation->imagePosition);
        if (m_flipImages)
          drawable.scale(Vec2F(-1, 1), drawable.boundBox(false).center());
        renderCallback->addDrawable(std::move(drawable), RenderLayerObject + 2);
      }
    }
  }
}

InteractAction LoungeableObject::interact(InteractRequest const& request) {
  auto res = Object::interact(request);
  if (res.type == InteractActionType::None && !m_sitPositions.empty()) {
    Maybe<size_t> index;
    Vec2F interactOffset =
        direction() == Direction::Right ? position() - request.interactPosition : request.interactPosition - position();
    for (size_t i = 0; i < m_sitPositions.size(); ++i) {
      if (!index || vmag(m_sitPositions[i] + interactOffset) < vmag(m_sitPositions[*index] + interactOffset))
        index = i;
    }
    return InteractAction(InteractActionType::SitDown, entityId(), *index);
  } else {
    return res;
  }
}

size_t LoungeableObject::anchorCount() const {
  return m_sitPositions.size();
}

LoungeAnchorConstPtr LoungeableObject::loungeAnchor(size_t positionIndex) const {
  if (positionIndex >= m_sitPositions.size())
    return {};

  auto loungeAnchor = make_shared<LoungeAnchor>();

  loungeAnchor->suppressTools = false;
  loungeAnchor->controllable = false;
  loungeAnchor->direction = m_sitFlipDirection ? -direction() : direction();

  loungeAnchor->position = m_sitPositions.at(positionIndex);
  if (loungeAnchor->direction == Direction::Left)
    loungeAnchor->position[0] *= -1;
  loungeAnchor->position += position();

  loungeAnchor->exitBottomPosition = Vec2F(loungeAnchor->position[0], position()[1] + volume().boundBox().min()[1]);

  loungeAnchor->angle = m_sitAngle;
  if (loungeAnchor->direction == Direction::Left)
    loungeAnchor->angle *= -1;

  loungeAnchor->orientation = m_sitOrientation;
  // Layer all anchored entities one above the object layer, in top to bottom
  // order based on the anchor index.
  loungeAnchor->loungeRenderLayer = RenderLayerObject + m_sitPositions.size() - positionIndex;

  loungeAnchor->statusEffects = m_sitStatusEffects;
  loungeAnchor->effectEmitters = m_sitEffectEmitters;
  loungeAnchor->emote = m_sitEmote;
  loungeAnchor->dance = m_sitDance;
  loungeAnchor->armorCosmeticOverrides = m_sitArmorCosmeticOverrides;
  loungeAnchor->cursorOverride = m_sitCursorOverride;

  return loungeAnchor;
}

void LoungeableObject::setOrientationIndex(size_t orientationIndex) {
  Object::setOrientationIndex(orientationIndex);
  if (orientationIndex != NPos) {
    if (auto sp = configValue("sitPosition")) {
      m_sitPositions = {jsonToVec2F(configValue("sitPosition")) / TilePixels};
    } else if (auto sps = configValue("sitPositions")) {
      m_sitPositions.clear();
      for (auto const& sp : sps.toArray())
        m_sitPositions.append(jsonToVec2F(sp) / TilePixels);
    }
    m_sitFlipDirection = configValue("sitFlipDirection", false).toBool();
    m_sitOrientation = LoungeOrientationNames.getLeft(configValue("sitOrientation", "sit").toString());
    m_sitAngle = configValue("sitAngle", 0).toFloat() * Constants::pi / 180.0f;
    m_sitCoverImage = configValue("sitCoverImage", "").toString();
    m_flipImages = configValue("flipImages", false).toBool();
    m_sitStatusEffects = configValue("sitStatusEffects", JsonArray()).toArray().transformed(jsonToPersistentStatusEffect);
    m_sitEffectEmitters = jsonToStringSet(configValue("sitEffectEmitters", JsonArray()));
    m_sitEmote = configValue("sitEmote").optString();
    m_sitDance = configValue("sitDance").optString();
    m_sitArmorCosmeticOverrides = configValue("sitArmorCosmeticOverrides", JsonObject()).toObject();
    m_sitCursorOverride = configValue("sitCursorOverride").optString();
  }
}

}
