import star.json;
import star.json_extra;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.id_map;
import star.data_stream;
import star.bi_map;
import star.game_types;
import star.strong_typedef;
import star.casting;
import star.list;
import star.image_processing;
import std;
import star.format;
import star.variant;
import star.rpc_promise;
import star.a_star;
import star.set;
import star.string;
import star.color;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.periodic_function;
import star.audio;
import star.map;
import star.maybe;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import star.version;
import star.periodic;
import star.lua;
import star.spline;

import star.lua_root;
import star.lua_components;
import star.lua_animation_component;
import star.drawable;
import star.item;
import star.item_descriptor;
import star.actor_movement_controller;
import star.item_database;
import star.item_recipe;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.humanoid;
import star.status_types;
import star.damage_types;
import star.world_geometry;
import star.damage;

import star.armor_wearer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.status_effect_item;
import star.effect_source_item;
import star.previewable_item;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;
import star.game_timers;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.tool_user_item;
import star.fireable_item;
import star.swingable_item;
import star.armors;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.entity_rendering_types;
import star.entity_rendering;
import star.non_rotated_drawables_item;
import star.beam_item;
import star.durability_item;
import star.pointable_item;
struct PreviewTile;
import star.preview_tile_tool;
import star.tools;
import star.activatable_item;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;



import star.effect_emitter;

import star.light_source;

import star.array;
namespace Star {

ArmorWearer::ArmorWearer() : m_lastNude(true) {
  for (std::size_t i = 0; i != m_armors.size(); ++i) {
    auto& armor = m_armors[i];
    armor.isCosmetic = i >= 4;
    if (i >= 8)
      armor.netState.setCompatibilityVersion(9);
    addNetElement(&armor.netState);
  }

  reset();
}

bool ArmorWearer::setupHumanoid(Humanoid& humanoid, bool forceNude) {
  bool nudeChanged = m_lastNude != forceNude;
  auto gender = humanoid.identity().gender;
  bool genderChanged = !m_lastGender || *m_lastGender != gender;
  Direction direction = humanoid.facingDirection();
  bool dirChanged = !m_lastDirection || *m_lastDirection != direction;
  m_lastNude = forceNude;
  m_lastGender = gender;
  m_lastDirection = direction;

  bool allNeedsSync = genderChanged;
  bool anyNeedsSync = allNeedsSync;
  Array<std::uint8_t, 4> wornCosmeticTypes;
  for (std::size_t i = 0; i != m_armors.size(); ++i) {
    auto& armor = m_armors[i];
    auto& item = armor.item;
    if (armor.needsSync)
      anyNeedsSync = true;
    else if (allNeedsSync || (item && ((dirChanged && item->flipping()) || (nudeChanged && !item->bypassNude()))))
      anyNeedsSync = armor.needsSync = true;

    if (armor.visible && armor.isCosmetic && item && item->visible(i >= 8)) {
      for (auto armorType : item->armorTypesToHide())
        ++wornCosmeticTypes[(std::uint8_t)armorType];
    }
  }

  bool bodyHidden = false;
  Json humanoidConfig;
  auto addHumanoidConfig = [&](ArmorItem const& item) {
    bodyHidden |= item.hideBody();
    auto newConfig = item.instanceValue("humanoidConfig");
    if (newConfig.isType(Json::Type::Object)) {
      if (!humanoidConfig)
        humanoidConfig = JsonObject();
      humanoidConfig = jsonMerge(humanoidConfig, newConfig);
    }
  };

  std::exception_ptr configException;
  bool movementParametersChanged;
  if (anyNeedsSync) {
    for (std::size_t i = 0; i != m_armors.size(); ++i) {
      Armor& armor = m_armors[i];
      auto& item = armor.item;
      bool allowed = true;
      if (!armor.visible || !item || !item->visible(i >= 8) || (forceNude && !item->bypassNude())) {
        allowed = false;
      } else if (!armor.isCosmetic) {
        std::uint8_t typeIndex = (std::uint8_t)armor.item->armorType();
        std::uint8_t prevWorn = m_wornCosmeticTypes[typeIndex];
        std::uint8_t curWorn = wornCosmeticTypes[typeIndex];
        if (curWorn == 0) {
          if (prevWorn != 0)
            armor.needsSync = true;
        } else {
          allowed = false;
        }
      }

      m_armors[i].isCurrentlyVisible = allowed;
      if (allowed) {
        addHumanoidConfig(*armor.item);
        if (armor.needsSync) {
          if (auto head = as<HeadArmor>(armor.item))
            humanoid.setWearableFromHead(i, *head, gender);
          else if (auto chest = as<ChestArmor>(armor.item))
            humanoid.setWearableFromChest(i, *chest, gender);
          else if (auto legs = as<LegsArmor>(armor.item))
            humanoid.setWearableFromLegs(i, *legs, gender);
          else if (auto back = as<BackArmor>(armor.item))
            humanoid.setWearableFromBack(i, *back, gender);
          armor.needsSync = false;
        }
      } else
        humanoid.removeWearable(i);
    }
    try
      { movementParametersChanged = humanoid.loadConfig(humanoidConfig); }
    catch (std::exception const&)
      { configException = std::current_exception(); }
    humanoid.setBodyHidden(bodyHidden);
  }
  m_wornCosmeticTypes = wornCosmeticTypes;
  if (configException)
    std::rethrow_exception(configException);
  return movementParametersChanged;
}

void ArmorWearer::effects(EffectEmitter& effectEmitter) {
  StringSet headEffects, chestEffects, legsEffects, backEffects;

  for (std::uint8_t i = 0; i != m_armors.size(); ++i) {
    auto& armor = m_armors[i];
    if (auto item = as<EffectSourceItem>(armor.item)) {
      auto armorType = armor.item->armorType();
      if (!armor.visible || (!armor.isCosmetic && m_wornCosmeticTypes[(std::uint8_t)armorType] > 0))
        continue;
      auto newEffects = item->effectSources();
      if (armorType == ArmorType::Head)
        headEffects.addAll(newEffects);
      else if (armorType == ArmorType::Chest)
        chestEffects.addAll(newEffects);
      else if (armorType == ArmorType::Legs)
        legsEffects.addAll(newEffects);
      else if (armorType == ArmorType::Back)
        backEffects.addAll(newEffects);
    }
  }

  effectEmitter.addEffectSources("headArmor", headEffects);
  effectEmitter.addEffectSources("chestArmor", chestEffects);
  effectEmitter.addEffectSources("legsArmor", legsEffects);
  effectEmitter.addEffectSources("backArmor", backEffects);
}

void ArmorWearer::reset() {
  m_lastGender.reset();
  m_lastDirection.reset();
  for (auto& armor : m_armors) {
    armor.item.reset();
    armor.needsStore = armor.needsSync = true;
  }
}

Json ArmorWearer::diskStore() const {
  JsonObject res;
  auto save = [&](std::uint8_t slot, String const& id) {
    auto& armor = m_armors[slot];
    if (armor.item)
      res[id] = armor.item->descriptor().diskStore();
  };

  save(0, "headItem");
  save(1, "chestItem");
  save(2, "legsItem");
  save(3, "backItem");
  save(4, "headCosmeticItem");
  save(5, "chestCosmeticItem");
  save(6, "legsCosmeticItem");
  save(7, "backCosmeticItem");

  for (std::size_t i = 8; i != m_armors.size(); ++i)
    save(i, strf("cosmetic{}Item", i - 7));

  return res;
}

void ArmorWearer::diskLoad(Json const& diskStore) {
  auto itemDb = Root::singleton().itemDatabase();
  auto load = [&](std::uint8_t slot, String const& id) {
    if (auto item = as<ArmorItem>(itemDb->diskLoad(diskStore.get(id, {}))))
      setItem(slot, item);
  };

  load(0, "headItem");
  load(1, "chestItem");
  load(2, "legsItem");
  load(3, "backItem");
  load(4, "headCosmeticItem");
  load(5, "chestCosmeticItem");
  load(6, "legsCosmeticItem");
  load(7, "backCosmeticItem");

  for (std::size_t i = 8; i != m_armors.size(); ++i)
    load(i, strf("cosmetic{}Item", i - 7));
}

List<PersistentStatusEffect> ArmorWearer::statusEffects(bool cosmeticOnly) const {
  List<PersistentStatusEffect> statusEffects;

  for (std::size_t i = 0; i != m_armors.size(); ++i) {
    if (!m_armors[i].item)
      continue;
    if (!cosmeticOnly && ((i < 4) ||  m_armors[i].item->statusEffectsInCosmeticSlot()))
      statusEffects.appendAll(m_armors[i].item->statusEffects());
    if (m_armors[i].isCurrentlyVisible)
      statusEffects.appendAll(m_armors[i].item->cosmeticStatusEffects());
  }

  return statusEffects;
}

bool ArmorWearer::setItem(std::uint8_t slot, std::shared_ptr<ArmorItem> item, bool visible) {
  if (slot >= m_armors.size())
    return false;
  auto& armor = m_armors[slot];

  bool itemChanged = !Item::itemsEqual(armor.item, item);
  bool visChanged = visible != armor.visible;

  if (itemChanged)
    armor.item = item;
  if (visChanged)
    armor.visible = visible;

  if (itemChanged || visChanged)
    return armor.needsStore = armor.needsSync = true;
  else
    return false;
}

void ArmorWearer::setHeadItem(std::shared_ptr<HeadArmor> headItem) { setItem(0, headItem, true); }
void ArmorWearer::setChestItem(std::shared_ptr<ChestArmor> chestItem) { setItem(1, chestItem, true); }
void ArmorWearer::setLegsItem(std::shared_ptr<LegsArmor> legsItem) { setItem(2, legsItem, true); }
void ArmorWearer::setBackItem(std::shared_ptr<BackArmor> backItem) { setItem(3, backItem, true); }
void ArmorWearer::setHeadCosmeticItem(std::shared_ptr<HeadArmor> headCosmeticItem) { setItem(4, headCosmeticItem, true); }
void ArmorWearer::setChestCosmeticItem(std::shared_ptr<ChestArmor> chestCosmeticItem) { setItem(5, chestCosmeticItem, true); }
void ArmorWearer::setLegsCosmeticItem(std::shared_ptr<LegsArmor> legsCosmeticItem) { setItem(6, legsCosmeticItem, true); }
void ArmorWearer::setBackCosmeticItem(std::shared_ptr<BackArmor> backCosmeticItem) { setItem(7, backCosmeticItem, true); }

std::shared_ptr<ArmorItem> ArmorWearer::item(std::uint8_t slot) const {
  if (slot < m_armors.size())
    return m_armors[slot].item;
  return {};
}

 std::shared_ptr<HeadArmor> ArmorWearer:: headItem() const { return as< HeadArmor>(item(0)); }
std::shared_ptr<ChestArmor> ArmorWearer::chestItem() const { return as<ChestArmor>(item(1)); }
 std::shared_ptr<LegsArmor> ArmorWearer:: legsItem() const { return as< LegsArmor>(item(2)); }
 std::shared_ptr<BackArmor> ArmorWearer:: backItem() const { return as< BackArmor>(item(3)); }
 std::shared_ptr<HeadArmor> ArmorWearer:: headCosmeticItem() const { return as< HeadArmor>(item(4)); }
std::shared_ptr<ChestArmor> ArmorWearer::chestCosmeticItem() const { return as<ChestArmor>(item(5)); }
 std::shared_ptr<LegsArmor> ArmorWearer:: legsCosmeticItem() const { return as< LegsArmor>(item(6)); }
 std::shared_ptr<BackArmor> ArmorWearer:: backCosmeticItem() const { return as< BackArmor>(item(7)); }

ItemDescriptor ArmorWearer::itemDescriptor(std::uint8_t slot) const {
   if (auto foundItem = item(slot))
     return foundItem->descriptor();
   return {};
}

ItemDescriptor ArmorWearer::headItemDescriptor() const {
  if (auto item = headItem())
    return item->descriptor();
  return {};
}

ItemDescriptor ArmorWearer::chestItemDescriptor() const {
  if (auto item = chestItem())
    return item->descriptor();
  return {};
}

ItemDescriptor ArmorWearer::legsItemDescriptor() const {
  if (auto item = legsItem())
    return item->descriptor();
  return {};
}

ItemDescriptor ArmorWearer::backItemDescriptor() const {
  if (auto item = backItem())
    return item->descriptor();
  return {};
}

ItemDescriptor ArmorWearer::headCosmeticItemDescriptor() const {
  if (auto item = headCosmeticItem())
    return item->descriptor();
  return {};
}

ItemDescriptor ArmorWearer::chestCosmeticItemDescriptor() const {
  if (auto item = chestCosmeticItem())
    return item->descriptor();
  return {};
}

ItemDescriptor ArmorWearer::legsCosmeticItemDescriptor() const {
  if (auto item = legsCosmeticItem())
    return item->descriptor();
  return {};
}

ItemDescriptor ArmorWearer::backCosmeticItemDescriptor() const {
  if (auto item = backCosmeticItem())
    return item->descriptor();
  return {};
}

bool ArmorWearer::setCosmeticItem(std::uint8_t slot, std::shared_ptr<ArmorItem> cosmeticItem) {
  return setItem(slot + 8, cosmeticItem, true);
}

std::shared_ptr<ArmorItem> ArmorWearer::cosmeticItem(std::uint8_t slot) const {
  return item(slot + 8);
}

ItemDescriptor ArmorWearer::cosmeticItemDescriptor(std::uint8_t slot) const {
  if (auto item = cosmeticItem(slot + 8))
    return item->descriptor();
  return {};
}

void ArmorWearer::netElementsNeedLoad(bool) {
  auto itemDatabase = Root::singleton().itemDatabase();

  for (auto& armor : m_armors) {
    if (armor.netState.pullUpdated())
      armor.needsStore |= armor.needsSync |= itemDatabase->loadItem(armor.netState.get(), armor.item);
  }
}

void ArmorWearer::netElementsNeedStore() {
  auto itemDatabase = Root::singleton().itemDatabase();
  for (auto& armor : m_armors) {
    if (armor.needsStore) {
      armor.netState.set(armor.visible ? itemSafeDescriptor(armor.item) : ItemDescriptor());
      armor.needsStore = false;
    }
  }
}

}
