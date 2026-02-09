#pragma once

#include "StarConfig.hpp"
#include "StarGameTypes.hpp"
#include "StarItem.hpp"
#include "StarSwingableItem.hpp"

import std;

namespace Star {

class ConsumableItem : public Item, public SwingableItem {
public:
  ConsumableItem(Json const& config, String const& directory, Json const& data);

  auto clone() const -> Ptr<Item> override;

  auto drawables() const -> List<Drawable> override;

  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void fireTriggered() override;
  void uninit() override;

private:
  auto canUse() const -> bool;

  void triggerEffects();
  void maybeConsume();

  StringSet m_blockingEffects;
  std::optional<float> m_foodValue;
  StringSet m_emitters;
  String m_emote;
  bool m_consuming;
};

}// namespace Star
