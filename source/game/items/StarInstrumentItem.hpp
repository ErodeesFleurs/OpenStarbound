#pragma once

#include "StarActivatableItem.hpp"
#include "StarConfig.hpp"
#include "StarEffectSourceItem.hpp"
#include "StarItem.hpp"
#include "StarPointableItem.hpp"
#include "StarStatusEffectItem.hpp"
#include "StarToolUserItem.hpp"

namespace Star {

class InstrumentItem : public Item,
                       public StatusEffectItem,
                       public EffectSourceItem,
                       public ToolUserItem,
                       public ActivatableItem,
                       public PointableItem {
public:
  InstrumentItem(Json const& config, String const& directory, Json const& data);

  [[nodiscard]] auto clone() const -> Ptr<Item> override;

  [[nodiscard]] auto statusEffects() const -> List<PersistentStatusEffect> override;
  [[nodiscard]] auto effectSources() const -> StringSet override;

  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  [[nodiscard]] auto active() const -> bool override;
  void setActive(bool active) override;
  [[nodiscard]] auto usable() const -> bool override;
  void activate() override;

  [[nodiscard]] auto drawables() const -> List<Drawable> override;
  auto getAngle(float angle) -> float override;

private:
  List<PersistentStatusEffect> m_activeStatusEffects;
  List<PersistentStatusEffect> m_inactiveStatusEffects;
  StringSet m_activeEffectSources;
  StringSet m_inactiveEffectSources;
  List<Drawable> m_drawables;
  List<Drawable> m_activeDrawables;
  int m_activeCooldown;

  float m_activeAngle;
  String m_kind;
};

}// namespace Star
