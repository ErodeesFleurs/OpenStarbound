module;
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarIdMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarDataStream.hpp"
#include "StarString.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarBiMap.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarConfig.hpp"
#include "StarSet.hpp"
#include "StarOrderedMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
import star.version;


import star.item;
import star.status_types;
import star.status_effect_item;
import star.effect_source_item;
import star.damage_types;
import star.world_geometry;
import star.damage;
import star.light_source;
import star.entity;
import star.animation;
import star.particle;
import star.interaction_types;
import star.tile_damage;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
#include "StarLuaRoot.hpp"
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;
import star.game_timers;
import star.actor_movement_controller;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.tool_user_item;
import star.activatable_item;
import star.drawable;
import star.pointable_item;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

export module star.instrument_item;

export namespace Star {

STAR_CLASS(InstrumentItem);

class InstrumentItem : public Item,
                       public StatusEffectItem,
                       public EffectSourceItem,
                       public ToolUserItem,
                       public ActivatableItem,
                       public PointableItem {
public:
  InstrumentItem(Json const& config, String const& directory, Json const& data);

  ItemPtr clone() const override;

  List<PersistentStatusEffect> statusEffects() const override;
  StringSet effectSources() const override;

  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  bool active() const override;
  void setActive(bool active) override;
  bool usable() const override;
  void activate() override;

  List<Drawable> drawables() const override;
  float getAngle(float angle) override;

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

ItemPtr InstrumentItem::clone() const {
  return make_shared<InstrumentItem>(*this);
}

List<PersistentStatusEffect> InstrumentItem::statusEffects() const {
  if (active())
    return m_activeStatusEffects;
  return m_inactiveStatusEffects;
}

StringSet InstrumentItem::effectSources() const {
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

bool InstrumentItem::active() const {
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

bool InstrumentItem::usable() const {
  return true;
}

void InstrumentItem::activate() {
  owner()->interact(InteractAction{InteractActionType::OpenSongbookInterface, owner()->entityId(), {}});
}

List<Drawable> InstrumentItem::drawables() const {
  if (active())
    return m_activeDrawables;
  return m_drawables;
}

float InstrumentItem::getAngle(float angle) {
  if (active())
    return m_activeAngle;
  return angle;
}

}
