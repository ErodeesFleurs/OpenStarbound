#include "StarToolUserItem.hpp"

namespace Star {

void ToolUserItem::init(ToolUserEntity& owner, ToolHand hand) {
  m_owner.reset(&owner);
  m_hand = hand;
}

void ToolUserItem::uninit() {
  m_owner.reset();
  m_hand = {};
}

void ToolUserItem::update(float, FireMode, bool, HashSet<MoveControlType> const&) {}

[[nodiscard]] bool ToolUserItem::initialized() const {
  return static_cast<bool>(m_owner);
}

[[nodiscard]] ToolUserEntity& ToolUserItem::owner() const {
  if (!m_owner)
    throw ToolUserItemException("Not initialized in ToolUserItem::owner");
  return *m_owner;
}

[[nodiscard]] EntityMode ToolUserItem::entityMode() const {
  if (!m_owner)
    throw ToolUserItemException("Not initialized in ToolUserItem::entityMode");
  return m_owner->entityMode().value(EntityMode::Master);
}

[[nodiscard]] ToolHand ToolUserItem::hand() const {
  if (!m_owner)
    throw ToolUserItemException("Not initialized in ToolUserItem::hand");
  return *m_hand;
}

[[nodiscard]] World* ToolUserItem::world() const {
  if (!m_owner)
    throw ToolUserItemException("Not initialized in ToolUserItem::world");
  return &m_owner->world();
}

[[nodiscard]] List<DamageSource> ToolUserItem::damageSources() const {
  return {};
}

[[nodiscard]] List<PolyF> ToolUserItem::shieldPolys() const {
  return {};
}

[[nodiscard]] List<PhysicsForceRegion> ToolUserItem::forceRegions() const {
  return {};
}

}
