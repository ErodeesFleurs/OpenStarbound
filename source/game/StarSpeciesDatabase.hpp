#pragma once

#include "StarConfig.hpp"
#include "StarHumanoid.hpp"
#include "StarItemDescriptor.hpp"
#include "StarLuaRoot.hpp"
#include "StarStatusTypes.hpp"
#include "StarThread.hpp"

import std;

namespace Star {

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
  SpeciesOption();

  String species;
  bool headOptionAsHairColor;
  bool headOptionAsFacialhair;
  bool altOptionAsUndyColor;
  bool altOptionAsHairColor;
  bool altOptionAsFacialMask;
  bool hairColorAsBodySubColor;
  bool bodyColorAsFacialMaskSubColor;
  bool altColorAsFacialMaskSubColor;
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
  SpeciesDefinition(Json const& config);

  [[nodiscard]] auto config() const -> Json;
  [[nodiscard]] auto kind() const -> String;
  [[nodiscard]] auto playerSelectable() const -> bool;
  [[nodiscard]] auto options() const -> SpeciesOption const&;
  [[nodiscard]] auto humanoidConfig() const -> Json;
  [[nodiscard]] auto personalities() const -> List<Personality> const&;
  [[nodiscard]] auto nameGen(Gender gender) const -> String;
  [[nodiscard]] auto ouchNoise(Gender gender) const -> String;
  [[nodiscard]] auto defaultItems() const -> List<ItemDescriptor>;
  [[nodiscard]] auto defaultBlueprints() const -> List<ItemDescriptor>;
  [[nodiscard]] auto charGenTextLabels() const -> StringList;
  [[nodiscard]] auto skull() const -> String;
  [[nodiscard]] auto statusEffects() const -> List<PersistentStatusEffect>;
  [[nodiscard]] auto effectDirectives() const -> String;

  [[nodiscard]] auto tooltip() const -> SpeciesCharCreationTooltip const&;

private:
  String m_kind;
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
  SpeciesDatabase();

  auto species(String const& kind) const -> Ptr<SpeciesDefinition>;
  auto allSpecies() const -> StringMap<Ptr<SpeciesDefinition>>;

  auto humanoidConfig(HumanoidIdentity identity, JsonObject parameters = JsonObject(), Json config = Json()) const -> Json;
  auto createHumanoid(String name, String speciesChoice, size_t genderChoice, size_t bodyColor, size_t alty, size_t hairChoice, size_t heady, size_t shirtChoice, size_t shirtColor, size_t pantsChoice, size_t pantsColor, size_t personality, LuaVariadic<LuaValue> ext = {}) const -> CharacterCreationResult;

  auto generateHumanoid(String species, std::int64_t seed, std::optional<Gender> = {}) const -> CharacterCreationResult;

private:
  StringMap<Ptr<SpeciesDefinition>> m_species;

  mutable RecursiveMutex m_luaMutex;
  Ptr<LuaRoot> m_luaRoot;
};

}// namespace Star
