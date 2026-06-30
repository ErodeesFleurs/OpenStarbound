#pragma once

#include "StarToolUserEntity.hpp"
#include "StarPhysicsEntity.hpp"

namespace Star {

struct ToolUserItemExceptionTag { static constexpr char const* typeName = "ToolUserItemException"; };
using ToolUserItemException = TypedException<StarException, ToolUserItemExceptionTag>;

class ToolUserItem;

// FIXME: You know what another name for an item that a tool user uses is?  A
// Tool.  Three words when one will do, rename.
class ToolUserItem {
public:
  ToolUserItem() = default;
  virtual ~ToolUserItem() = default;

  // Owner must be initialized when a ToolUserItem is initialized and
  // uninitialized before the owner is uninitialized.
  virtual void init(ToolUserEntity& owner, ToolHand hand);
  virtual void uninit();

  // Default implementation does nothing
  virtual void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves);

  // Default implementations return empty list
  [[nodiscard]] virtual List<DamageSource> damageSources() const;
  [[nodiscard]] virtual List<PolyF> shieldPolys() const;
  [[nodiscard]] virtual List<PhysicsForceRegion> forceRegions() const;

  [[nodiscard]] bool initialized() const;

  // owner, entityMode, hand, and world throw ToolUserException if
  // initialized() is false
  [[nodiscard]] ToolUserEntity* owner() const;
  [[nodiscard]] EntityMode entityMode() const;
  [[nodiscard]] ToolHand hand() const;
  [[nodiscard]] World* world() const;

private:
  ToolUserEntity* m_owner = nullptr;
  Maybe<ToolHand> m_hand;
};

}
