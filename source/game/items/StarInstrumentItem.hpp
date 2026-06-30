#pragma once

#include "StarItem.hpp"
#include "StarInstrumentItem.hpp"
#include "StarStatusEffectItem.hpp"
#include "StarEffectSourceItem.hpp"
#include "StarToolUserItem.hpp"
#include "StarActivatableItem.hpp"
#include "StarPointableItem.hpp"
#include "StarAssets.hpp"

namespace Star {

class World;
class ToolUserEntity;
class InstrumentItem;

class InstrumentItem : public Item,
                       public StatusEffectItem,
                       public EffectSourceItem,
                       public ToolUserItem,
                       public ActivatableItem,
                       public PointableItem {
public:
  InstrumentItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& data);

  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] List<PersistentStatusEffect> statusEffects() const override;
  [[nodiscard]] StringSet effectSources() const override;

  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  [[nodiscard]] bool active() const override;
  void setActive(bool active) override;
  [[nodiscard]] bool usable() const override;
  void activate() override;

  [[nodiscard]] List<Drawable> drawables() const override;
  [[nodiscard]] float getAngle(float angle) override;

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

}
