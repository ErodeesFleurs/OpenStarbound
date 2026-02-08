#pragma once

#include "StarConfig.hpp"
#include "StarEffectSourceItem.hpp"
#include "StarGameTypes.hpp"
#include "StarItem.hpp"
#include "StarPreviewableItem.hpp"
#include "StarSwingableItem.hpp"

import std;

namespace Star {

enum class ArmorType : std::uint8_t {
  Head,
  Chest,
  Legs,
  Back
};
extern EnumMap<ArmorType> ArmorTypeNames;

class ArmorItem : public Item, public EffectSourceItem, public SwingableItem {
public:
  ArmorItem(Json const& config, String const& directory, Json const& data);
  ~ArmorItem() override = default;

  auto statusEffects() const -> List<PersistentStatusEffect> override;
  auto statusEffectsInCosmeticSlot() const -> bool;
  auto cosmeticStatusEffects() const -> List<PersistentStatusEffect>;

  auto effectSources() const -> StringSet override;

  auto drawables() const -> List<Drawable> override;

  auto getAngle(float aimAngle) -> float override;

  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void fireTriggered() override;

  virtual auto armorType() const -> ArmorType = 0;

  auto colorOptions() -> List<String> const&;

  auto directives(bool flip = false) const -> Directives const&;
  auto fullbright() const -> bool;
  auto flipping() const -> bool;
  auto visible(bool extraCosmetics = false) const -> bool;
  auto armorTypesToHide() -> HashSet<ArmorType> const&;
  auto hideBody() const -> bool;
  auto bypassNude() const -> bool;

  auto techModule() const -> std::optional<String> const&;

private:
  void refreshIconDrawables();
  void refreshStatusEffects();

  List<String> m_colorOptions;
  List<PersistentStatusEffect> m_statusEffects;
  StringSet m_effectSources;
  Directives m_directives;
  std::optional<Directives> m_flipDirectives;
  bool m_hideBody;
  bool m_bypassNude;
  bool m_hideInVanillaSlots;
  bool m_statusEffectsInCosmeticSlot;
  bool m_fullbright;
  List<PersistentStatusEffect> m_cosmeticStatusEffects;
  std::optional<HashSet<ArmorType>> m_armorTypesToHide;
  std::optional<String> m_techModule;
};

class HeadArmor : public ArmorItem, public PreviewableItem {
public:
  HeadArmor(Json const& config, String const& directory, Json const& data);
  ~HeadArmor() override = default;

  auto clone() const -> Ptr<Item> override;

  auto armorType() const -> ArmorType override;

  auto frameset(Gender gender) const -> String const&;
  auto maskDirectives() const -> Directives const&;

  auto preview(Ptr<Player> const& viewer = {}) const -> List<Drawable> override;

private:
  String m_maleImage;
  String m_femaleImage;
  Directives m_maskDirectives;
};

class ChestArmor : public ArmorItem, public PreviewableItem {
public:
  ChestArmor(Json const& config, String const& directory, Json const& data);
  ~ChestArmor() override = default;

  auto clone() const -> Ptr<Item> override;

  auto armorType() const -> ArmorType override;

  // Will have :run, :normal, :duck, and :portrait
  auto bodyFrameset(Gender gender) const -> String const&;
  // Will have :idle[1-5], :duck, :rotation, :walk[1-5], :run[1-5], :jump[1-4],
  // :fall[1-4]
  auto frontSleeveFrameset(Gender gender) const -> String const&;
  // Same as FSleeve
  auto backSleeveFrameset(Gender gender) const -> String const&;

  auto preview(Ptr<Player> const& viewer = {}) const -> List<Drawable> override;

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
  ~LegsArmor() override = default;

  auto clone() const -> Ptr<Item> override;

  auto armorType() const -> ArmorType override;

  // Will have :idle, :duck, :walk[1-8], :run[1-8], :jump[1-4], :fall[1-4]
  auto frameset(Gender gender) const -> String const&;

  auto preview(Ptr<Player> const& viewer = {}) const -> List<Drawable> override;

private:
  String m_maleImage;
  String m_femaleImage;
};

class BackArmor : public ArmorItem, public PreviewableItem {
public:
  BackArmor(Json const& config, String const& directory, Json const& data);
  ~BackArmor() override = default;

  auto clone() const -> Ptr<Item> override;

  auto armorType() const -> ArmorType override;

  // Will have :idle, :duck, :walk[1-8], :run[1-8], :jump[1-4], :fall[1-4]
  auto frameset(Gender gender) const -> String const&;

  auto preview(Ptr<Player> const& viewer = {}) const -> List<Drawable> override;

private:
  String m_maleImage;
  String m_femaleImage;
};

}// namespace Star
