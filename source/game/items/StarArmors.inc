#pragma once
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarDataStream.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarAssetPath.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarSet.hpp"

namespace Star {
STAR_CLASS(ArmorItem);
STAR_CLASS(HeadArmor);
STAR_CLASS(ChestArmor);
STAR_CLASS(LegsArmor);
STAR_CLASS(BackArmor);
}

import star.drawable;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;
import star.status_types;
import star.status_effect_item;
import star.effect_source_item;
import star.previewable_item;
import star.damage_types;
import star.world_geometry;
import star.damage;
import star.light_source;
import star.entity;
import star.animation;
import star.particle;
import star.interaction_types;
import star.tile_damage;
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
#include "StarLuaComponents.hpp"
import star.fireable_item;
import star.swingable_item;

namespace Star {

enum class ArmorType : uint8_t {
  Head,
  Chest,
  Legs,
  Back
};
extern EnumMap<ArmorType> ArmorTypeNames;


class ArmorItem : public Item, public EffectSourceItem, public SwingableItem {
public:
  ArmorItem(Json const& config, String const& directory, Json const& data);
  virtual ~ArmorItem() {}

  virtual List<PersistentStatusEffect> statusEffects() const override;
  bool statusEffectsInCosmeticSlot() const;
  List<PersistentStatusEffect> cosmeticStatusEffects() const;

  virtual StringSet effectSources() const override;

  virtual List<Drawable> drawables() const override;

  virtual float getAngle(float aimAngle) override;

  virtual void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  virtual void fireTriggered() override;

  virtual ArmorType armorType() const = 0;

  List<String> const& colorOptions();

  Directives const& directives(bool flip = false) const;
  bool fullbright() const;
  bool flipping() const;
  bool visible(bool extraCosmetics = false) const;
  HashSet<ArmorType> const& armorTypesToHide();
  bool hideBody() const;
  bool bypassNude() const;

  Maybe<String> const& techModule() const;

private:
  void refreshIconDrawables();
  void refreshStatusEffects();

  List<String> m_colorOptions;
  List<PersistentStatusEffect> m_statusEffects;
  StringSet m_effectSources;
  Directives m_directives;
  Maybe<Directives> m_flipDirectives;
  bool m_hideBody;
  bool m_bypassNude;
  bool m_hideInVanillaSlots;
  bool m_statusEffectsInCosmeticSlot;
  bool m_fullbright;
  List<PersistentStatusEffect> m_cosmeticStatusEffects;
  Maybe<HashSet<ArmorType>> m_armorTypesToHide;
  Maybe<String> m_techModule;
};

class HeadArmor : public ArmorItem, public PreviewableItem {
public:
  HeadArmor(Json const& config, String const& directory, Json const& data);
  virtual ~HeadArmor() {}

  virtual ItemPtr clone() const override;

  virtual ArmorType armorType() const override;

  String const& frameset(Gender gender) const;
  Directives const& maskDirectives() const;

  virtual List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

private:
  String m_maleImage;
  String m_femaleImage;
  Directives m_maskDirectives;
};

class ChestArmor : public ArmorItem, public PreviewableItem {
public:
  ChestArmor(Json const& config, String const& directory, Json const& data);
  virtual ~ChestArmor() {}

  virtual ItemPtr clone() const override;

  virtual ArmorType armorType() const override;

  // Will have :run, :normal, :duck, and :portrait
  String const& bodyFrameset(Gender gender) const;
  // Will have :idle[1-5], :duck, :rotation, :walk[1-5], :run[1-5], :jump[1-4],
  // :fall[1-4]
  String const& frontSleeveFrameset(Gender gender) const;
  // Same as FSleeve
  String const& backSleeveFrameset(Gender gender) const;

  virtual List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

private:
  String m_maleBodyImage;
  String m_maleFrontSleeveImage;
  String m_maleBackSleeveImage;

  String m_femaleBodyImage;
  String m_femaleFrontSleeveImage;
  String m_femaleBackSleeveImage;
};

class LegsArmor : public ArmorItem, public PreviewableItem {
public:
  LegsArmor(Json const& config, String const& directory, Json const& data);
  virtual ~LegsArmor() {}

  virtual ItemPtr clone() const override;

  virtual ArmorType armorType() const override;

  // Will have :idle, :duck, :walk[1-8], :run[1-8], :jump[1-4], :fall[1-4]
  String const& frameset(Gender gender) const;

  virtual List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

private:
  String m_maleImage;
  String m_femaleImage;
};

class BackArmor : public ArmorItem, public PreviewableItem {
public:
  BackArmor(Json const& config, String const& directory, Json const& data);
  virtual ~BackArmor() {}

  virtual ItemPtr clone() const override;

  virtual ArmorType armorType() const override;

  // Will have :idle, :duck, :walk[1-8], :run[1-8], :jump[1-4], :fall[1-4]
  String const& frameset(Gender gender) const;

  virtual List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

private:
  String m_maleImage;
  String m_femaleImage;
};

}

