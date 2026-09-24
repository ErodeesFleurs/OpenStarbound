module;

#include "StarObject.hpp"
#include "StarLoungingEntities.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"
#include "StarObjectDatabase.hpp"

export module star.loungeable_object;

export namespace Star {

class LoungeableObject : public Object, public virtual LoungeableEntity {
public:
  LoungeableObject(ObjectConfigConstPtr config, Json const& parameters = Json());

  void render(RenderCallback* renderCallback) override;

  InteractAction interact(InteractRequest const& request) override;

  size_t anchorCount() const override;
  LoungeAnchorConstPtr loungeAnchor(size_t positionIndex) const override;

protected:
  void setOrientationIndex(size_t orientationIndex) override;

private:
  List<Vec2F> m_sitPositions;
  bool m_sitFlipDirection;
  LoungeOrientation m_sitOrientation;
  float m_sitAngle;
  String m_sitCoverImage;
  bool m_flipImages;
  List<PersistentStatusEffect> m_sitStatusEffects;
  StringSet m_sitEffectEmitters;
  Maybe<String> m_sitEmote;
  Maybe<String> m_sitDance;
  JsonObject m_sitArmorCosmeticOverrides;
  Maybe<String> m_sitCursorOverride;
};

}

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
