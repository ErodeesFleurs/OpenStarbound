module;
#include "StarIdMap.hpp"
#include "StarSet.hpp"
#include "StarThread.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarGameTypes.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarBiMap.hpp"
#include "StarJson.hpp"
#include "StarStrongTypedef.hpp"
#include "StarDataStream.hpp"

import star.drawable;
import star.entity_rendering_types;
import star.damage_types;
import star.world_geometry;
import star.damage;
import star.light_source;
import star.entity;
import star.status_types;

namespace Star {

STAR_STRUCT(ProjectileConfig);
STAR_CLASS(Projectile);
STAR_CLASS(ProjectileDatabase);

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
  List<tuple<float, bool, Json>> periodicActions;

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
