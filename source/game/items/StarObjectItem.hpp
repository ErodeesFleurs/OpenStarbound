#pragma once

#include "StarBeamItem.hpp"
#include "StarConfig.hpp"
#include "StarFireableItem.hpp"
#include "StarItem.hpp"

namespace Star {

class ObjectItem : public Item, public FireableItem, public BeamItem {
public:
  ObjectItem(Json const& config, String const& directory, Json const& objectParameters);
  ~ObjectItem() override = default;

  auto clone() const -> Ptr<Item> override;

  void init(ToolUserEntity* owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  auto nonRotatedDrawables() const -> List<Drawable> override;

  auto cooldownTime() const -> float override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;

  auto objectName() const -> String;
  auto objectParameters() const -> Json;

  auto placeInWorld(FireMode mode, bool shifting) -> bool;
  auto canPlace(bool shifting) const -> bool;

private:
  bool m_shifting;
};

}// namespace Star
