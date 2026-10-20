module;
import std;
import star.format;
import star.json;
import star.string;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.id_map;
import star.color;
import star.bi_map;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;


import star.drawable;
import star.animated_part_set;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.humanoid;
import star.damage_types;
import star.status_types;
import star.animation;
import star.particle;
import star.item_descriptor;

import star.entity_splash;

import star.net_compatibility;
import star.byte_array;
namespace Star {

class Root;
class Rebuilder;
class ItemDatabase;
class Item;
class Npc;

struct NpcExceptionTag {
  static constexpr char const* name() { return "NpcException"; }
};
using NpcException = StarError<NpcExceptionTag, StarException>;

struct NpcVariant {
  String species;
  String typeName;
  float level;
  std::uint64_t seed;

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
  std::uint8_t damageTeam;

  Vec3B nametagColor;

  EntitySplashConfig splashConfig;
};

class NpcDatabase {
public:
  explicit NpcDatabase(Root& root);

  NpcVariant generateNpcVariant(String const& species, String const& typeName, float level, ItemDatabase const* itemCatalog = nullptr) const;
  NpcVariant generateNpcVariant(String const& species, String const& typeName, float level, std::uint64_t seed, Json const& overrides, ItemDatabase const* itemCatalog = nullptr) const;

  ByteArray writeNpcVariant(NpcVariant const& variant, NetCompatibilityRules rules = {}) const;
  NpcVariant readNpcVariant(ByteArray const& data, NetCompatibilityRules rules = {}) const;

  Json writeNpcVariantToJson(NpcVariant const& variant) const;
  NpcVariant readNpcVariantFromJson(Json const& data) const;

  std::shared_ptr<Npc> createNpc(NpcVariant const& npcVariant) const;
  std::shared_ptr<Npc> diskLoadNpc(Json const& diskStore) const;
  std::shared_ptr<Npc> netLoadNpc(ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

  List<Drawable> npcPortrait(NpcVariant const& npcVariant, PortraitMode mode, ItemDatabase const* itemCatalog = nullptr) const;

  Json buildConfig(String const& typeName, Json const& overrides = Json()) const;

private:
  // Recursively merges maps and lets any non-null merger (including lists)
  // override any base value
  Json mergeConfigValues(Json const& base, Json const& merger) const;

  Root& m_root;

  std::shared_ptr<Rebuilder> m_rebuilder;

  StringMap<Json> m_npcTypes;
};

}

export module star.npc_database;

export namespace Star {
  using ::Star::NpcExceptionTag;
  using ::Star::NpcException;
  using ::Star::NpcVariant;
  using ::Star::Rebuilder;
  using ::Star::Item;
  using ::Star::Npc;
  using ::Star::NpcDatabase;
}
