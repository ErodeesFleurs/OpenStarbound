#include "StarInstrumentItem.hpp"

#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"

import std;

namespace Star {

InstrumentItem::InstrumentItem(Json const& config, String const& directory, Json const& data) : Item(config, directory, data) {
  m_activeCooldown = 0;

  auto image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  Vec2F position = jsonToVec2F(instanceValue("handPosition", JsonArray{0, 0}));
  m_drawables.append(Drawable::makeImage(image, 1.0f / TilePixels, true, position));

  image = AssetPath::relativeTo(directory, instanceValue("activeImage").toString());
  position = jsonToVec2F(instanceValue("activeHandPosition", JsonArray{0, 0}));
  m_activeDrawables.append(Drawable::makeImage(image, 1.0f / TilePixels, true, position));

  m_activeAngle = (instanceValue("activeAngle").toFloat() / 180.0f) * Constants::pi;

  m_activeStatusEffects = instanceValue("activeStatusEffects", JsonArray()).toArray().transformed(jsonToPersistentStatusEffect);
  m_inactiveStatusEffects = instanceValue("inactiveStatusEffects", JsonArray()).toArray().transformed(jsonToPersistentStatusEffect);
  m_activeEffectSources = jsonToStringSet(instanceValue("activeEffectSources", JsonArray()));
  m_inactiveEffectSources = jsonToStringSet(instanceValue("inactiveEffectSources", JsonArray()));

  m_kind = instanceValue("kind").toString();
}

auto InstrumentItem::clone() const -> Ptr<Item> {
  return std::make_shared<InstrumentItem>(*this);
}

auto InstrumentItem::statusEffects() const -> List<PersistentStatusEffect> {
  if (active())
    return m_activeStatusEffects;
  return m_inactiveStatusEffects;
}

auto InstrumentItem::effectSources() const -> StringSet {
  if (active())
    return m_activeEffectSources;
  return m_inactiveEffectSources;
}

void InstrumentItem::update(float, FireMode, bool, HashSet<MoveControlType> const&) {
  if (entityMode() == EntityMode::Master) {
    if (active()) {
      m_activeCooldown--;
      owner()->addEffectEmitters({"music"});
    }
  }
  owner()->instrumentEquipped(m_kind);
}

auto InstrumentItem::active() const -> bool {
  if (!initialized())
    return false;
  return (m_activeCooldown > 0) || owner()->instrumentPlaying();
}

void InstrumentItem::setActive(bool active) {
  if (active)
    m_activeCooldown = 3;
  else
    m_activeCooldown = 0;
}

auto InstrumentItem::usable() const -> bool {
  return true;
}

void InstrumentItem::activate() {
  owner()->interact(InteractAction{InteractActionType::OpenSongbookInterface, owner()->entityId(), {}});
}

auto InstrumentItem::drawables() const -> List<Drawable> {
  if (active())
    return m_activeDrawables;
  return m_drawables;
}

auto InstrumentItem::getAngle(float angle) -> float {
  if (active())
    return m_activeAngle;
  return angle;
}

}// namespace Star
