#pragma once

#include "StarGameTypes.hpp"
#include "StarItem.hpp"
#include "StarStatusEffectItem.hpp"
#include "StarEffectSourceItem.hpp"
#include "StarPreviewableItem.hpp"
#include "StarSwingableItem.hpp"
#include "StarAssets.hpp"
namespace Star {

enum class ArmorType : uint8_t {
  Head,
  Chest,
  Legs,
  Back
};
extern EnumMap<ArmorType> ArmorTypeNames;

class ArmorItem;
using ArmorItemPtr = SharedPtr<ArmorItem>;
class FunctionDatabase;
using FunctionDatabaseConstPtr = SharedPtr<FunctionDatabase const>;
class HeadArmor;
using HeadArmorPtr = SharedPtr<HeadArmor>;
class ChestArmor;
using ChestArmorPtr = SharedPtr<ChestArmor>;
class LegsArmor;
using LegsArmorPtr = SharedPtr<LegsArmor>;
class BackArmor;
using BackArmorPtr = SharedPtr<BackArmor>;

class ArmorItem : public Item, public EffectSourceItem, public SwingableItem {
public:
  ArmorItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& data, FunctionDatabaseConstPtr functionDatabase);
  virtual ~ArmorItem() = default;

  [[nodiscard]] List<PersistentStatusEffect> statusEffects() const override;
  [[nodiscard]] bool statusEffectsInCosmeticSlot() const;
  [[nodiscard]] List<PersistentStatusEffect> cosmeticStatusEffects() const;

  [[nodiscard]] StringSet effectSources() const override;

  [[nodiscard]] List<Drawable> drawables() const override;

  [[nodiscard]] float getAngle(float aimAngle) override;

  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;
  void fireTriggered() override;

  [[nodiscard]] virtual ArmorType armorType() const = 0;

  [[nodiscard]] List<String> const& colorOptions();

  [[nodiscard]] Directives const& directives(bool flip = false) const;
  [[nodiscard]] bool fullbright() const;
  [[nodiscard]] bool flipping() const;
  [[nodiscard]] bool visible(bool extraCosmetics = false) const;
  [[nodiscard]] HashSet<ArmorType> const& armorTypesToHide();
  [[nodiscard]] bool hideBody() const;
  [[nodiscard]] bool bypassNude() const;

  [[nodiscard]] Maybe<String> const& techModule() const;

protected:
  AssetsConstPtr m_assets;
  FunctionDatabaseConstPtr m_functionDatabase;

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
  HeadArmor(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& data, FunctionDatabaseConstPtr functionDatabase);
  virtual ~HeadArmor() = default;

  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] ArmorType armorType() const override;

  [[nodiscard]] String const& frameset(Gender gender) const;
  [[nodiscard]] Directives const& maskDirectives() const;

  [[nodiscard]] List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

private:
  String m_maleImage;
  String m_femaleImage;
  Directives m_maskDirectives;
};

class ChestArmor : public ArmorItem, public PreviewableItem {
public:
  ChestArmor(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& data, FunctionDatabaseConstPtr functionDatabase);
  virtual ~ChestArmor() = default;

  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] ArmorType armorType() const override;

  // Will have :run, :normal, :duck, and :portrait
  [[nodiscard]] String const& bodyFrameset(Gender gender) const;
  // Will have :idle[1-5], :duck, :rotation, :walk[1-5], :run[1-5], :jump[1-4],
  // :fall[1-4]
  [[nodiscard]] String const& frontSleeveFrameset(Gender gender) const;
  // Same as FSleeve
  [[nodiscard]] String const& backSleeveFrameset(Gender gender) const;

  [[nodiscard]] List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

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
  LegsArmor(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& data, FunctionDatabaseConstPtr functionDatabase);
  virtual ~LegsArmor() = default;

  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] ArmorType armorType() const override;

  // Will have :idle, :duck, :walk[1-8], :run[1-8], :jump[1-4], :fall[1-4]
  [[nodiscard]] String const& frameset(Gender gender) const;

  [[nodiscard]] List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

private:
  String m_maleImage;
  String m_femaleImage;
};

class BackArmor : public ArmorItem, public PreviewableItem {
public:
  BackArmor(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& data, FunctionDatabaseConstPtr functionDatabase);
  virtual ~BackArmor() = default;

  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] ArmorType armorType() const override;

  // Will have :idle, :duck, :walk[1-8], :run[1-8], :jump[1-4], :fall[1-4]
  [[nodiscard]] String const& frameset(Gender gender) const;

  [[nodiscard]] List<Drawable> preview(PlayerPtr const& viewer = {}) const override;

private:
  String m_maleImage;
  String m_femaleImage;
};

}
