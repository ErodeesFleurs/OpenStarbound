#pragma once

#include "StarAssets.hpp"
#include "StarThread.hpp"
#include "StarHumanoid.hpp"
#include "StarDamageTypes.hpp"
#include "StarStatusTypes.hpp"
#include "StarEntitySplash.hpp"
#include "StarItemDescriptor.hpp"

namespace Star {

class Rebuilder;
using RebuilderPtr = SharedPtr<Rebuilder>;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class SpeciesDatabase;
using SpeciesDatabaseConstPtr = SharedPtr<SpeciesDatabase const>;
class PatternedNameGenerator;
using PatternedNameGeneratorConstPtr = SharedPtr<PatternedNameGenerator const>;
class FunctionDatabase;
using FunctionDatabaseConstPtr = SharedPtr<FunctionDatabase const>;
class DanceDatabase;
using DanceDatabaseConstPtr = SharedPtr<DanceDatabase const>;
class EmoteProcessor;
using EmoteProcessorConstPtr = SharedPtr<EmoteProcessor const>;
class Npc;
using NpcPtr = SharedPtr<Npc>;
class NpcDatabase;
using NpcDatabasePtr = SharedPtr<NpcDatabase>;
using NpcDatabaseConstPtr = SharedPtr<NpcDatabase const>;

struct NpcExceptionTag { static constexpr char const* typeName = "NpcException"; };
using NpcException = TypedException<StarException, NpcExceptionTag>;

struct NpcVariant {
  String species;
  String typeName;
  float level;
  uint64_t seed;

  Json overrides;

  StringList scripts;
  unsigned initialScriptDelta;
  Json scriptConfig;

  Maybe<String> description;

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

class NpcDatabase : public enable_shared_from_this<NpcDatabase> {
public:
  NpcDatabase(AssetsConstPtr assets,
      ItemDatabaseConstPtr itemDatabase,
      ObjectDatabaseConstPtr objectDatabase,
      SpeciesDatabaseConstPtr speciesDatabase,
      PatternedNameGeneratorConstPtr nameGenerator,
      FunctionDatabaseConstPtr functionDatabase,
      DanceDatabaseConstPtr danceDatabase,
      EmoteProcessorConstPtr emoteProcessor);

  NpcVariant generateNpcVariant(String const& species, String const& typeName, float level) const;
  NpcVariant generateNpcVariant(String const& species, String const& typeName, float level, uint64_t seed, Json const& overrides) const;

  ByteArray writeNpcVariant(NpcVariant const& variant, NetCompatibilityRules rules = {}) const;
  NpcVariant readNpcVariant(ByteArray const& data, NetCompatibilityRules rules = {}) const;

  Json writeNpcVariantToJson(NpcVariant const& variant) const;
  NpcVariant readNpcVariantFromJson(Json const& data) const;

  NpcPtr createNpc(NpcVariant const& npcVariant) const;
  NpcPtr diskLoadNpc(Json const& diskStore) const;
  NpcPtr netLoadNpc(ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

  List<Drawable> npcPortrait(NpcVariant const& npcVariant, PortraitMode mode) const;

  Json buildConfig(String const& typeName, Json const& overrides = Json()) const;

private:
  // Recursively merges maps and lets any non-null merger (including lists)
  // override any base value
  Json mergeConfigValues(Json const& base, Json const& merger) const;

  RebuilderPtr m_rebuilder;
  AssetsConstPtr m_assets;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  PatternedNameGeneratorConstPtr m_nameGenerator;
  FunctionDatabaseConstPtr m_functionDatabase;
  DanceDatabaseConstPtr m_danceDatabase;
  EmoteProcessorConstPtr m_emoteProcessor;

  StringMap<Json> m_npcTypes;
};

}
