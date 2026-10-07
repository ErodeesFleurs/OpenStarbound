module;
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
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
#include "StarRandom.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarOrderedMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;
import star.observer_stream;
#include "StarNetElementExt.hpp"
#include "StarEither.hpp"
#include "StarInterpolation.hpp"
import star.periodic_function;
#include "StarMatrix3.hpp"
#include "StarAudio.hpp"
#include "StarMap.hpp"


import star.drawable;
import star.item;
import star.damage_types;
import star.world_geometry;
import star.status_types;
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
import star.status_effect_item;
#include "StarLuaComponents.hpp"
import star.fireable_item;
import star.swingable_item;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
#include "StarLuaActorMovementComponent.hpp"
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.entity_rendering_types;
import star.stat_collection;
import star.status_effect_database;
import star.status_controller;

export module star.consumable_item;

export namespace Star {

class ConsumableItem : public Item, public SwingableItem {
public:
  ConsumableItem(Json const& config, String const& directory, Json const& data);

  ItemPtr clone() const override;

  List<Drawable> drawables() const override;

  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void fireTriggered() override;
  void uninit() override;

private:
  bool canUse() const;

  void triggerEffects();
  void maybeConsume();

  StringSet m_blockingEffects;
  Maybe<float> m_foodValue;
  StringSet m_emitters;
  String m_emote;
  bool m_consuming;
};

}

namespace Star {

ConsumableItem::ConsumableItem(Json const& config, String const& directory, Json const& data)
  : Item(config, directory, data), SwingableItem(config) {
  setWindupTime(0);
  setCooldownTime(0.25f);
  m_requireEdgeTrigger = true;
  m_swingStart = config.getFloat("swingStart", -60) * Constants::pi / 180;
  m_swingFinish = config.getFloat("swingFinish", 40) * Constants::pi / 180;
  m_swingAimFactor = config.getFloat("swingAimFactor", 0.2f);
  m_blockingEffects = jsonToStringSet(instanceValue("blockingEffects", JsonArray()));
  if (auto foodValue = instanceValue("foodValue")) {
    m_foodValue = foodValue.toFloat();
    m_blockingEffects.add("wellfed");
  }
  m_emitters = jsonToStringSet(instanceValue("emitters", JsonArray{"eating"}));
  m_emote = instanceValue("emote", "eat").toString();
  m_consuming = false;
}

ItemPtr ConsumableItem::clone() const {
  return make_shared<ConsumableItem>(*this);
}

List<Drawable> ConsumableItem::drawables() const {
  auto drawables = iconDrawables();
  Drawable::scaleAll(drawables, 1.0f / TilePixels);
  Drawable::translateAll(drawables, -handPosition() / TilePixels);
  return drawables;
}

void ConsumableItem::update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) {
  SwingableItem::update(dt, fireMode, shifting, moves);

  if (entityMode() == EntityMode::Master) {
    if (m_consuming)
      owner()->addEffectEmitters(m_emitters);
    if (ready())
      maybeConsume();
  }
}

void ConsumableItem::fire(FireMode mode, bool shifting, bool edgeTriggered) {
  if (canUse())
    FireableItem::fire(mode, shifting, edgeTriggered);
}

void ConsumableItem::fireTriggered() {
  if (canUse()) {
    triggerEffects();
    FireableItem::fireTriggered();
  }
}

void ConsumableItem::uninit() {
  maybeConsume();
  FireableItem::uninit();
}

bool ConsumableItem::canUse() const {
  if (!count() || m_consuming)
    return false;

  for (auto pair : owner()->statusController()->activeUniqueStatusEffectSummary()) {
    if (m_blockingEffects.contains(pair.first))
      return false;
  }
  return true;
}

void ConsumableItem::triggerEffects() {
  auto options = instanceValue("effects", JsonArray()).toArray();
  if (options.size()) {
    auto option = Random::randFrom(options).toArray().transformed(jsonToEphemeralStatusEffect);
    owner()->statusController()->addEphemeralEffects(option);
  }

  if (m_foodValue) {
    owner()->statusController()->giveResource("food", *m_foodValue);
    if (owner()->statusController()->resourcePercentage("food") == 1.0f)
      owner()->statusController()->addEphemeralEffect(EphemeralStatusEffect{UniqueStatusEffect("wellfed"), {}});
  }

  if (!m_emote.empty())
    owner()->requestEmote(m_emote);

  m_consuming = true;
}

void ConsumableItem::maybeConsume() {
  if (m_consuming) {
    m_consuming = false;

    world()->sendEntityMessage(owner()->entityId(), "recordEvent", {"useItem", JsonObject {
      {"itemType", name()}
    }});
    if (count())
      setCount(count() - 1);
    else
      setCount(0);
  }
}

}
