module;

#include "StarHumanoid.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
import star.damage_types;
#include "StarJson.hpp"
#include "StarStrongTypedef.hpp"
#include "StarDataStream.hpp"
#include "StarIdMap.hpp"
import star.status_types;
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
import star.animation;
import star.particle;
#include "StarJson.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
import star.item_descriptor;

import star.entity_splash;

namespace Star {

STAR_CLASS(Rebuilder);
STAR_CLASS(Item);
STAR_CLASS(Npc);
STAR_CLASS(NpcDatabase);

struct NpcExceptionTag {
  static constexpr char const* name() { return "NpcException"; }
};
using NpcException = StarError<NpcExceptionTag, StarException>;

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

class NpcDatabase {
public:
  NpcDatabase();

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

  StringMap<Json> m_npcTypes;
};

}

export module star.npc_database;

export namespace Star {
  using ::Star::NpcExceptionTag;
  using ::Star::NpcException;
  using ::Star::NpcVariant;
  using ::Star::Rebuilder;
  using ::Star::RebuilderPtr;
  using ::Star::RebuilderConstPtr;
  using ::Star::RebuilderWeakPtr;
  using ::Star::RebuilderConstWeakPtr;
  using ::Star::RebuilderUPtr;
  using ::Star::RebuilderConstUPtr;
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::Npc;
  using ::Star::NpcPtr;
  using ::Star::NpcConstPtr;
  using ::Star::NpcWeakPtr;
  using ::Star::NpcConstWeakPtr;
  using ::Star::NpcUPtr;
  using ::Star::NpcConstUPtr;
  using ::Star::NpcDatabase;
  using ::Star::NpcDatabasePtr;
  using ::Star::NpcDatabaseConstPtr;
  using ::Star::NpcDatabaseWeakPtr;
  using ::Star::NpcDatabaseConstWeakPtr;
  using ::Star::NpcDatabaseUPtr;
  using ::Star::NpcDatabaseConstUPtr;
}
