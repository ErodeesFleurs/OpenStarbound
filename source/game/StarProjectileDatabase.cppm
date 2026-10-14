module;
import std;
import star.maybe;
import star.format;
import star.json;
import star.net_compatibility;
import star.byte_array;
import star.id_map;
import star.set;
import star.thread;
import star.color;
import star.string;
import star.data_stream;
import star.poly;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.game_types;
#include "StarCasting.hpp"
import star.vector;
import star.bi_map;
#include "StarStrongTypedef.hpp"

import star.drawable;
import star.entity_rendering_types;
import star.damage_types;
import star.world_geometry;
import star.damage;
import star.light_source;
import star.entity;
import star.status_types;

namespace Star {

struct ProjectileConfig;
using ProjectileConfigPtr = std::shared_ptr<ProjectileConfig>;
using ProjectileConfigConstPtr = std::shared_ptr<const ProjectileConfig>;
using ProjectileConfigWeakPtr = std::weak_ptr<ProjectileConfig>;
using ProjectileConfigConstWeakPtr = std::weak_ptr<const ProjectileConfig>;
using ProjectileConfigUPtr = std::unique_ptr<ProjectileConfig>;
using ProjectileConfigConstUPtr = std::unique_ptr<const ProjectileConfig>;
class Projectile;
using ProjectilePtr = std::shared_ptr<Projectile>;
using ProjectileConstPtr = std::shared_ptr<const Projectile>;
using ProjectileWeakPtr = std::weak_ptr<Projectile>;
using ProjectileConstWeakPtr = std::weak_ptr<const Projectile>;
using ProjectileUPtr = std::unique_ptr<Projectile>;
using ProjectileConstUPtr = std::unique_ptr<const Projectile>;
class ProjectileDatabase;
using ProjectileDatabasePtr = std::shared_ptr<ProjectileDatabase>;
using ProjectileDatabaseConstPtr = std::shared_ptr<const ProjectileDatabase>;
using ProjectileDatabaseWeakPtr = std::weak_ptr<ProjectileDatabase>;
using ProjectileDatabaseConstWeakPtr = std::weak_ptr<const ProjectileDatabase>;
using ProjectileDatabaseUPtr = std::unique_ptr<ProjectileDatabase>;
using ProjectileDatabaseConstUPtr = std::unique_ptr<const ProjectileDatabase>;

struct ProjectileDatabaseExceptionTag {
  static constexpr char const* name() { return "ProjectileDatabaseException"; }
};
using ProjectileDatabaseException = StarError<ProjectileDatabaseExceptionTag, StarException>;

struct ProjectileConfig {
  Json config;

  String typeName;
  String directory;

  String description;

  RectF boundBox;

  Json movementSettings;
  float timeToLive = 0.0f;
  float initialSpeed = 0.0f;
  float acceleration = 0.0f;
  float power = 0.0f;
  PolyF damagePoly;
  bool piercing = false;
  bool falldown = false;
  bool rayCheckToSource = false;
  float knockback = 0.0f;
  bool knockbackDirectional = false;

  // Negative value means infinite bounces.
  int bounces = 0;

  // Happens each time the projectile collides with a solid material
  JsonArray actionOnCollide;

  // Happens when projectile dies in any fashion
  JsonArray actionOnReap;

  // Happens when projectile dies after having collided
  JsonArray actionOnHit;

  // Happens when projectile dies without having collided
  JsonArray actionOnTimeout;

  // Time, repeat flag, and action config
  List<std::tuple<float, bool, Json>> periodicActions;

  String image;
  unsigned frameNumber = 0;
  float animationCycle = 0.0f;
  bool animationLoops = false;
  unsigned windupFrames = 0;
  bool intangibleWindup = false;
  unsigned winddownFrames = 0;
  bool intangibleWinddown = false;
  bool flippable = false;
  bool orientationLocked = false;

  bool fullbright = false;
  EntityRenderLayer renderLayer;

  Color lightColor;
  Vec2F lightPosition;
  LightType lightType = LightType::Spread;

  String persistentAudio;

  String damageKindImage;

  String damageKind;
  String damageType;
  Json damageTeam;
  Maybe<String> damageRepeatGroup;
  Maybe<float> damageRepeatTimeout;

  List<EphemeralStatusEffect> statusEffects;

  StringSet emitters;

  bool hydrophobic = false;
  bool onlyHitTerrain = false;
  ClientEntityMode clientEntityMode = ClientEntityMode::ClientMasterAllowed;
  bool masterOnly = false;

  StringList scripts;

  List<PersistentStatusEffect> persistentStatusEffects;
  PolyF statusEffectArea;

  Json physicsForces;
  Json physicsCollisions;
};

class ProjectileDatabase {
public:
  ProjectileDatabase();

  StringList allProjectileTypes() const;
  bool isProjectile(String const& typeName) const;

  Json projectileConfig(String const& type) const;

  String damageKindImage(String const& type) const;
  float gravityMultiplier(String const& type) const;

  ProjectilePtr createProjectile(String const& type, Json const& parameters = JsonObject()) const;
  ProjectilePtr netLoadProjectile(ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

private:
  ProjectileConfigPtr readConfig(String const& path);

  StringMap<ProjectileConfigPtr> m_configs;
};

}

export module star.projectile_database;

export namespace Star {
  using ::Star::ProjectileDatabaseExceptionTag;
  using ::Star::ProjectileDatabaseException;
  using ::Star::ProjectileConfig;
  using ::Star::ProjectileConfigPtr;
  using ::Star::ProjectileConfigConstPtr;
  using ::Star::ProjectileConfigWeakPtr;
  using ::Star::ProjectileConfigConstWeakPtr;
  using ::Star::ProjectileConfigUPtr;
  using ::Star::ProjectileConfigConstUPtr;
  using ::Star::Projectile;
  using ::Star::ProjectilePtr;
  using ::Star::ProjectileConstPtr;
  using ::Star::ProjectileWeakPtr;
  using ::Star::ProjectileConstWeakPtr;
  using ::Star::ProjectileUPtr;
  using ::Star::ProjectileConstUPtr;
  using ::Star::ProjectileDatabase;
  using ::Star::ProjectileDatabasePtr;
  using ::Star::ProjectileDatabaseConstPtr;
  using ::Star::ProjectileDatabaseWeakPtr;
  using ::Star::ProjectileDatabaseConstWeakPtr;
  using ::Star::ProjectileDatabaseUPtr;
  using ::Star::ProjectileDatabaseConstUPtr;
}
