module;
import std;
import star.map;
import star.format;
import star.json;
import star.id_map;
import star.game_types;
import star.maybe;
import star.variant;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
import star.list;
import star.rpc_promise;
import star.vector;
import star.rect;
import star.bi_map;
import star.a_star;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.data_stream;
import star.image_processing;
import star.color;
import star.string;
import star.poly;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;


import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
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
import star.damage_types;
import star.drawable;
import star.entity_rendering_types;

import star.either;
import star.net_compatibility;
import star.byte_array;
namespace Star {

class LuaRoot;
using LuaRootPtr = std::shared_ptr<LuaRoot>;
using LuaRootConstPtr = std::shared_ptr<const LuaRoot>;
using LuaRootWeakPtr = std::weak_ptr<LuaRoot>;
using LuaRootConstWeakPtr = std::weak_ptr<const LuaRoot>;
using LuaRootUPtr = std::unique_ptr<LuaRoot>;
using LuaRootConstUPtr = std::unique_ptr<const LuaRoot>;
class Rebuilder;
using RebuilderPtr = std::shared_ptr<Rebuilder>;
using RebuilderConstPtr = std::shared_ptr<const Rebuilder>;
using RebuilderWeakPtr = std::weak_ptr<Rebuilder>;
using RebuilderConstWeakPtr = std::weak_ptr<const Rebuilder>;
using RebuilderUPtr = std::unique_ptr<Rebuilder>;
using RebuilderConstUPtr = std::unique_ptr<const Rebuilder>;
class RandomSource;
using RandomSourcePtr = std::shared_ptr<RandomSource>;
using RandomSourceConstPtr = std::shared_ptr<const RandomSource>;
using RandomSourceWeakPtr = std::weak_ptr<RandomSource>;
using RandomSourceConstWeakPtr = std::weak_ptr<const RandomSource>;
using RandomSourceUPtr = std::unique_ptr<RandomSource>;
using RandomSourceConstUPtr = std::unique_ptr<const RandomSource>;
class Monster;
using MonsterPtr = std::shared_ptr<Monster>;
using MonsterConstPtr = std::shared_ptr<const Monster>;
using MonsterWeakPtr = std::weak_ptr<Monster>;
using MonsterConstWeakPtr = std::weak_ptr<const Monster>;
using MonsterUPtr = std::unique_ptr<Monster>;
using MonsterConstUPtr = std::unique_ptr<const Monster>;
class MonsterDatabase;
using MonsterDatabasePtr = std::shared_ptr<MonsterDatabase>;
using MonsterDatabaseConstPtr = std::shared_ptr<const MonsterDatabase>;
using MonsterDatabaseWeakPtr = std::weak_ptr<MonsterDatabase>;
using MonsterDatabaseConstWeakPtr = std::weak_ptr<const MonsterDatabase>;
using MonsterDatabaseUPtr = std::unique_ptr<MonsterDatabase>;
using MonsterDatabaseConstUPtr = std::unique_ptr<const MonsterDatabase>;

struct MonsterExceptionTag {
  static constexpr char const* name() { return "MonsterException"; }
};
using MonsterException = StarError<MonsterExceptionTag, StarException>;

struct MonsterVariant {
  String type;
  std::uint64_t seed;
  Json uniqueParameters;

  Maybe<String> shortDescription;
  Maybe<String> description;

  Json animatorConfig;
  StringMap<String> animatorPartTags;
  float animatorZoom;
  // Is the animator specified Left facing?
  bool reversed;

  // Either is a String which specifies a dropPool, or a map which maps
  // damageSourceKind to the appropriate treasure pool for this monster, with a
  // "default" key as a catch-all.
  Json dropPoolConfig;

  // Every parameter specified in each section of the monster configuration is
  // stored here.  The base parameters, size parameters, variation parameters,
  // and part parameters are all merged together into one final configuration.
  Json parameters;

  // Parameters common to all Monsters

  StringList scripts;
  unsigned initialScriptDelta;
  StringList animationScripts;

  RectF metaBoundBox;
  EntityRenderLayer renderLayer;
  float scale;

  ActorMovementParameters movementSettings;
  float walkMultiplier;
  float runMultiplier;
  float jumpMultiplier;
  float weightMultiplier;
  float healthMultiplier;
  float touchDamageMultiplier;

  Json touchDamageConfig;
  StringMap<Json> animationDamageParts;
  Json statusSettings;
  Vec2F mouthOffset;
  Vec2F feetOffset;

  String powerLevelFunction;
  String healthLevelFunction;

  ClientEntityMode clientEntityMode;
  bool persistent;

  TeamType damageTeamType;
  std::uint8_t damageTeam;

  PolyF selfDamagePoly;

  Maybe<String> portraitIcon;

  float damageReceivedAggressiveDuration;
  float onDamagedOthersAggressiveDuration;
  float onFireAggressiveDuration;

  Vec3B nametagColor;
  Maybe<ColorReplaceMap> colorSwap;
};

class MonsterDatabase {
public:
  MonsterDatabase();

  void cleanup();

  StringList monsterTypes() const;

  MonsterVariant randomMonster(String const& typeName, Json const& uniqueParameters = JsonObject()) const;
  MonsterVariant monsterVariant(String const& typeName, std::uint64_t seed, Json const& uniqueParameters = JsonObject()) const;

  ByteArray writeMonsterVariant(MonsterVariant const& variant, NetCompatibilityRules rules = {}) const;
  MonsterVariant readMonsterVariant(ByteArray const& data, NetCompatibilityRules rules = {}) const;

  Json writeMonsterVariantToJson(MonsterVariant const& mVar) const;
  MonsterVariant readMonsterVariantFromJson(Json const& variant) const;

  // If level is 0, then the monster will start with the threat level of
  // whatever world they're spawned in.
  MonsterPtr createMonster(MonsterVariant monsterVariant, Maybe<float> level = {}, Json uniqueParameters = {}) const;
  MonsterPtr diskLoadMonster(Json const& diskStore) const;
  MonsterPtr netLoadMonster(ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

  List<Drawable> monsterPortrait(MonsterVariant const& variant) const;

  std::pair<String, String> skillInfo(String const& skillName) const;
  Json skillConfigParameter(String const& skillName, String const& configParameterName) const;

  ColorReplaceMap colorSwap(String const& setName, std::uint64_t seed) const;

  Json monsterConfig(String const& typeName) const;

private:
  struct MonsterType {
    String typeName;
    Maybe<String> shortDescription;
    Maybe<String> description;

    StringList categories;
    StringList partTypes;

    String animationConfigPath;
    String colors;
    bool reversed;

    JsonArray dropPools;

    Json baseParameters;

    // Additional part-specific parameters which will override any part-specific
    // parameters (such as skills, sounds, etc.) defined in individual .monsterpart files
    Json partParameterOverrides;

    // Description of all part parameters, and how they are combined and with
    // what defaults.
    Json partParameterDescription;

    Json toJson() const;
  };

  struct MonsterPart {
    String name;
    String category;
    String type;

    String path;
    JsonObject frames;
    Json partParameters;
  };

  struct MonsterSkill {
    String name;
    String label;
    String image;

    Json config;
    Json parameters;
    Json animationParameters;
  };

  // Merges part configuration by the method specified in the part parameter
  // config.
  static Json mergePartParameters(Json const& partParameterDescription, JsonArray const& parameters);

  // Merges final monster variant parameters together according to the
  // hard-coded variant merge rules (handles things like scripts which are
  // combined rather than overwritten)
  static Json mergeFinalParameters(JsonArray const& parameters);

  // Reads common parameters out of parameters map
  static void readCommonParameters(MonsterVariant& monsterVariant);

  // Maps category name -> part type -> part name -> MonsterPart.  part name ->
  // MonsterPart needs to be be in a predictable order.
  typedef StringMap<StringMap<Map<String, MonsterPart>>> PartDirectory;

  MonsterVariant produceMonster(String const& typeName, std::uint64_t seed, Json const& uniqueParameters) const;

  // Given a variant including parameters for baseSkills and specialSkills,
  // returns a variant containing a final 'skills' list of chosen skills, also
  // merges animation configs from skills together.
  std::pair<Json, Json> chooseSkills(Json const& parameters, Json const& animatorConfig, RandomSource& rand) const;

  StringMap<MonsterType> m_monsterTypes;
  PartDirectory m_partDirectory;
  StringMap<MonsterSkill> m_skills;
  StringMap<List<ColorReplaceMap>> m_colorSwaps;

  mutable Mutex m_cacheMutex;

  RebuilderPtr m_rebuilder;

  // Key here is the type name, seed, and the serialized unique parameters JSON
  mutable HashTtlCache<std::tuple<String, std::uint64_t, Json>, MonsterVariant> m_monsterCache;
};

}

export module star.monster_database;

export namespace Star {
  using ::Star::MonsterExceptionTag;
  using ::Star::MonsterException;
  using ::Star::MonsterVariant;
  using ::Star::LuaRoot;
  using ::Star::LuaRootPtr;
  using ::Star::LuaRootConstPtr;
  using ::Star::LuaRootWeakPtr;
  using ::Star::LuaRootConstWeakPtr;
  using ::Star::LuaRootUPtr;
  using ::Star::LuaRootConstUPtr;
  using ::Star::Rebuilder;
  using ::Star::RebuilderPtr;
  using ::Star::RebuilderConstPtr;
  using ::Star::RebuilderWeakPtr;
  using ::Star::RebuilderConstWeakPtr;
  using ::Star::RebuilderUPtr;
  using ::Star::RebuilderConstUPtr;
  using ::Star::RandomSource;
  using ::Star::RandomSourcePtr;
  using ::Star::RandomSourceConstPtr;
  using ::Star::RandomSourceWeakPtr;
  using ::Star::RandomSourceConstWeakPtr;
  using ::Star::RandomSourceUPtr;
  using ::Star::RandomSourceConstUPtr;
  using ::Star::Monster;
  using ::Star::MonsterPtr;
  using ::Star::MonsterConstPtr;
  using ::Star::MonsterWeakPtr;
  using ::Star::MonsterConstWeakPtr;
  using ::Star::MonsterUPtr;
  using ::Star::MonsterConstUPtr;
  using ::Star::MonsterDatabase;
  using ::Star::MonsterDatabasePtr;
  using ::Star::MonsterDatabaseConstPtr;
  using ::Star::MonsterDatabaseWeakPtr;
  using ::Star::MonsterDatabaseConstWeakPtr;
  using ::Star::MonsterDatabaseUPtr;
  using ::Star::MonsterDatabaseConstUPtr;
}
