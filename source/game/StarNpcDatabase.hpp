#pragma once

#include "StarConfig.hpp"
#include "StarDamageTypes.hpp"
#include "StarEntitySplash.hpp"
#include "StarException.hpp"
#include "StarHumanoid.hpp"
#include "StarItemDescriptor.hpp"
#include "StarStatusTypes.hpp"

import std;

namespace Star {

class Npc;
class Rebuilder;

using NpcException = ExceptionDerived<"NpcException">;

struct NpcVariant {
  String species;
  String typeName;
  float level;
  uint64_t seed;

  Json overrides;

  StringList scripts;
  unsigned initialScriptDelta;
  Json scriptConfig;

  std::optional<String> description;

  HumanoidIdentity humanoidIdentity;
  Json humanoidConfig;
  bool uniqueHumanoidConfig;
  JsonObject humanoidParameters;

  Json movementParameters;
  Json statusControllerSettings;
  List<PersistentStatusEffect> innateStatusEffects;
  Json touchDamageConfig;

  StringMap<ItemDescriptor> items;

  StringList dropPools;
  bool disableWornArmor;

  bool persistent;
  bool keepAlive;

  TeamType damageTeamType;
  uint8_t damageTeam;

  Vec3B nametagColor;

  EntitySplashConfig splashConfig;
};

class NpcDatabase {
public:
  NpcDatabase();

  [[nodiscard]] auto generateNpcVariant(String const& species, String const& typeName, float level) const -> NpcVariant;
  [[nodiscard]] auto generateNpcVariant(String const& species, String const& typeName, float level, uint64_t seed, Json const& overrides) const -> NpcVariant;

  [[nodiscard]] auto writeNpcVariant(NpcVariant const& variant, NetCompatibilityRules rules = {}) const -> ByteArray;
  [[nodiscard]] auto readNpcVariant(ByteArray const& data, NetCompatibilityRules rules = {}) const -> NpcVariant;

  [[nodiscard]] auto writeNpcVariantToJson(NpcVariant const& variant) const -> Json;
  [[nodiscard]] auto readNpcVariantFromJson(Json const& data) const -> NpcVariant;

  [[nodiscard]] auto createNpc(NpcVariant const& npcVariant) const -> Ptr<Npc>;
  [[nodiscard]] auto diskLoadNpc(Json const& diskStore) const -> Ptr<Npc>;
  [[nodiscard]] auto netLoadNpc(ByteArray const& netStore, NetCompatibilityRules rules = {}) const -> Ptr<Npc>;

  [[nodiscard]] auto npcPortrait(NpcVariant const& npcVariant, PortraitMode mode) const -> List<Drawable>;

  [[nodiscard]] auto buildConfig(String const& typeName, Json const& overrides = Json()) const -> Json;

private:
  // Recursively merges maps and lets any non-null merger (including lists)
  // override any base value
  [[nodiscard]] auto mergeConfigValues(Json const& base, Json const& merger) const -> Json;

  Ptr<Rebuilder> m_rebuilder;

  StringMap<Json> m_npcTypes;
};

}// namespace Star
