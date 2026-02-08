#pragma once

#include "StarException.hpp"
#include "StarToolUserEntity.hpp"

import std;

namespace Star {

using ToolUserItemException = ExceptionDerived<"ToolUserItemException">;

// FIXME: You know what another name for an item that a tool user uses is?  A
// Tool.  Three words when one will do, rename.
class ToolUserItem {
public:
  ToolUserItem();
  virtual ~ToolUserItem() = default;

  // Owner must be initialized when a ToolUserItem is initialized and
  // uninitialized before the owner is uninitialized.
  virtual void init(ToolUserEntity* owner, ToolHand hand);
  virtual void uninit();

  // Default implementation does nothing
  virtual void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves);

  // Default implementations return empty list
  [[nodiscard]] virtual auto damageSources() const -> List<DamageSource>;
  [[nodiscard]] virtual auto shieldPolys() const -> List<PolyF>;
  [[nodiscard]] virtual auto forceRegions() const -> List<PhysicsForceRegion>;

  [[nodiscard]] auto initialized() const -> bool;

  // owner, entityMode, hand, and world throw ToolUserException if
  // initialized() is false
  [[nodiscard]] auto owner() const -> ToolUserEntity*;
  [[nodiscard]] auto entityMode() const -> EntityMode;
  [[nodiscard]] auto hand() const -> ToolHand;
  [[nodiscard]] auto world() const -> World*;

private:
  ToolUserEntity* m_owner;
  std::optional<ToolHand> m_hand;
};

}// namespace Star
