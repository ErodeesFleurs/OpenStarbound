#pragma once

#include "StarEntityRenderingTypes.hpp"
#include "StarException.hpp"
#include "StarProjectile.hpp"

import std;

namespace Star {

using ProjectileDatabaseException = ExceptionDerived<"ProjectileDatabaseException">;

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
  std::optional<String> damageRepeatGroup;
  std::optional<float> damageRepeatTimeout;

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

  [[nodiscard]] auto allProjectileTypes() const -> StringList;
  [[nodiscard]] auto isProjectile(String const& typeName) const -> bool;

  [[nodiscard]] auto projectileConfig(String const& type) const -> Json;

  [[nodiscard]] auto damageKindImage(String const& type) const -> String;
  [[nodiscard]] auto gravityMultiplier(String const& type) const -> float;

  [[nodiscard]] auto createProjectile(String const& type, Json const& parameters = JsonObject()) const -> Ptr<Projectile>;
  [[nodiscard]] auto netLoadProjectile(ByteArray const& netStore, NetCompatibilityRules rules = {}) const -> Ptr<Projectile>;

private:
  auto readConfig(String const& path) -> Ptr<ProjectileConfig>;

  StringMap<Ptr<ProjectileConfig>> m_configs;
};

}// namespace Star
