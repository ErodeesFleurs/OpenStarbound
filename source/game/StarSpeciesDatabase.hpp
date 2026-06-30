#pragma once

#include "StarAssets.hpp"
#include "StarThread.hpp"
#include "StarItemDescriptor.hpp"
#include "StarHumanoid.hpp"
#include "StarStatusTypes.hpp"
#include "StarLuaRoot.hpp"
#include "StarTtlCache.hpp"

namespace Star {

class SpeciesDefinition;
using SpeciesDefinitionPtr = SharedPtr<SpeciesDefinition>;
class PatternedNameGenerator;
using PatternedNameGeneratorConstPtr = SharedPtr<PatternedNameGenerator const>;
class SpeciesDatabase;
using SpeciesDatabasePtr = SharedPtr<SpeciesDatabase>;
using SpeciesDatabaseConstPtr = SharedPtr<SpeciesDatabase const>;

struct SpeciesCharCreationTooltip {
  String title;
  String subTitle;
  String description;
};

struct SpeciesGenderOption {
  Gender gender;
  String name;
  String image;
  String characterImage;
  List<String> hairOptions;
  String hairGroup;
  List<String> shirtOptions;
  List<String> pantsOptions;
  String facialHairGroup;
  List<String> facialHairOptions;
  String facialMaskGroup;
  List<String> facialMaskOptions;
};

struct SpeciesOption {
  SpeciesOption() = default;

  String species;
  bool headOptionAsHairColor = false;
  bool headOptionAsFacialhair = false;
  bool altOptionAsUndyColor = false;
  bool altOptionAsHairColor = false;
  bool altOptionAsFacialMask = false;
  bool hairColorAsBodySubColor = false;
  bool bodyColorAsFacialMaskSubColor = false;
  bool altColorAsFacialMaskSubColor = false;
  List<SpeciesGenderOption> genderOptions;
  List<String> bodyColorDirectives;
  List<String> undyColorDirectives;
  List<String> hairColorDirectives;
};

struct CharacterCreationResult {
  HumanoidIdentity identity;
  JsonObject humanoidParameters;
  JsonObject armor;
};

class SpeciesDefinition {
public:
  SpeciesDefinition(Json const& config, AssetsConstPtr assets);

  [[nodiscard]] Json config() const;
  [[nodiscard]] String kind() const;
  [[nodiscard]] bool playerSelectable() const;
  [[nodiscard]] SpeciesOption const& options() const;
  [[nodiscard]] Json humanoidConfig() const;
  [[nodiscard]] List<Personality> const& personalities() const;
  [[nodiscard]] String nameGen(Gender gender) const;
  [[nodiscard]] String ouchNoise(Gender gender) const;
  [[nodiscard]] List<ItemDescriptor> defaultItems() const;
  [[nodiscard]] List<ItemDescriptor> defaultBlueprints() const;
  [[nodiscard]] StringList charGenTextLabels() const;
  [[nodiscard]] String skull() const;
  [[nodiscard]] List<PersistentStatusEffect> statusEffects() const;
  [[nodiscard]] String effectDirectives() const;

  [[nodiscard]] SpeciesCharCreationTooltip const& tooltip() const;

private:
  String m_kind;
  AssetsConstPtr m_assets;
  SpeciesCharCreationTooltip m_tooltip;
  bool m_playerSelectable;
  Json m_config;
  String m_humanoidConfig;
  Json m_humanoidOverrides;
  List<Personality> m_personalities;
  List<String> m_nameGen;
  List<String> m_ouchNoises;
  SpeciesOption m_options;
  List<ItemDescriptor> m_defaultItems;
  List<ItemDescriptor> m_defaultBlueprints;
  StringList m_charGenTextLabels;
  String m_skull;
  List<PersistentStatusEffect> m_statusEffects;
  String m_effectDirectives;

  List<String> m_buildScripts;
  List<String> m_creationScripts;

  friend class SpeciesDatabase;
};

class SpeciesDatabase {
public:
  SpeciesDatabase(AssetsConstPtr assets, PatternedNameGeneratorConstPtr nameGenerator, LuaRootServices luaRootServices);

  [[nodiscard]] bool hasSpecies(String const& kind) const;
  [[nodiscard]] SpeciesDefinitionPtr species(String const& kind) const;
  [[nodiscard]] StringList speciesNames() const;
  [[nodiscard]] StringMap<SpeciesDefinitionPtr> allSpecies() const;

  [[nodiscard]] Json humanoidConfig(HumanoidIdentity identity, JsonObject parameters = JsonObject(), Json config = Json()) const;
  [[nodiscard]] CharacterCreationResult createHumanoid(String name, String speciesChoice, size_t genderChoice, size_t bodyColor, size_t alty, size_t hairChoice, size_t heady, size_t shirtChoice, size_t shirtColor, size_t pantsChoice, size_t pantsColor, size_t personality, LuaVariadic<LuaValue> ext = {}) const;

  [[nodiscard]] CharacterCreationResult generateHumanoid(String species, int64_t seed, Maybe<Gender> = {}) const;

private:
  PatternedNameGeneratorConstPtr m_nameGenerator;
  StringMap<SpeciesDefinitionPtr> m_species;

  mutable RecursiveMutex m_luaMutex;
  LuaRootPtr m_luaRoot;
};

}
