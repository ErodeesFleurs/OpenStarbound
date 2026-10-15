import star.json;
#include "StarJsonExtra.hpp"
import star.set;
import star.thread;
import star.color;
import star.string;
import star.data_stream;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.game_types;
import star.id_map;
import star.ordered_map;
import star.variant;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
import star.rpc_promise;
import star.lua;
import star.maybe;
import star.bi_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.format;
import star.version;

import star.drawable;
import star.entity_rendering_types;
import star.damage_types;
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
import star.mobile_entity;
import star.scripted_entity;
import star.status_effect_entity;
import star.movement_controller;
import star.animation;
import star.particle;
#include "StarLuaComponents.hpp"
import star.effect_emitter;
import star.projectile;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.projectile_database;

#include "StarDataStreamDevices.hpp"

import star.net_compatibility;
import star.byte_array;
namespace Star {

ProjectileDatabase::ProjectileDatabase() {
  auto assets = Root::singleton().assets();

  auto& files = assets->scanExtension("projectile");
  assets->queueJsons(files);
  for (auto& file : files) {
    try {
      auto projectileConfig = readConfig(file);
      if (m_configs.contains(projectileConfig->typeName))
        Logger::error("Duplicate projectile asset typeName {}. configfile {}", projectileConfig->typeName, file);
      else
        m_configs[projectileConfig->typeName] = projectileConfig;
    } catch (std::exception const& e) {
      Logger::error("Could not read projectile '{}', error: {}", file, outputException(e, false));
    }
  }
}

StringList ProjectileDatabase::allProjectileTypes() const {
  return m_configs.keys();
}

bool ProjectileDatabase::isProjectile(String const& projectileName) const {
  return m_configs.contains(projectileName);
}

Json ProjectileDatabase::projectileConfig(String const& type) const {
  if (!m_configs.contains(type))
    throw ProjectileDatabaseException(strf("Unknown projectile with typeName {}.", type));
  return m_configs.get(type)->config;
}

std::shared_ptr<Projectile> ProjectileDatabase::createProjectile(String const& type, Json const& parameters) const {
  if (!m_configs.contains(type))
    throw ProjectileDatabaseException(strf("Unknown projectile with typeName {}.", type));
  return std::make_shared<Projectile>(m_configs.get(type), parameters);
}

String ProjectileDatabase::damageKindImage(String const& type) const {
  if (!m_configs.contains(type))
    throw ProjectileDatabaseException(strf("Unknown projectile with typeName {}.", type));
  auto& config = m_configs.get(type);
  return config->damageKindImage;
}

float ProjectileDatabase::gravityMultiplier(String const& type) const {
  if (!m_configs.contains(type))
    throw ProjectileDatabaseException(strf("Unknown projectile with typeName {}.", type));
  auto& config = m_configs.get(type);
  return config->movementSettings.getFloat("gravityMultiplier", 1);
}

std::shared_ptr<Projectile> ProjectileDatabase::netLoadProjectile(ByteArray const& netStore, NetCompatibilityRules rules) const {
  DataStreamBuffer ds(netStore);
  ds.setStreamCompatibilityVersion(rules);
  String typeName = ds.read<String>();
  return std::make_shared<Projectile>(m_configs.get(typeName), ds, rules);
}

std::shared_ptr<ProjectileConfig> ProjectileDatabase::readConfig(String const& path) {
  auto assets = Root::singleton().assets();

  Json config = assets->json(path);

  auto projectileConfig = std::make_shared<ProjectileConfig>();
  projectileConfig->config = config;
  projectileConfig->typeName = config.getString("projectileName");
  projectileConfig->directory = AssetPath::directory(path);

  projectileConfig->description = config.getString("description", "");

  projectileConfig->boundBox = config.opt("boundBox").apply(jsonToRectF).value({-5, -5, 5, 5});

  auto physicsType = config.getString("physics", "default");
  JsonObject movementSettings = config.getObject("movementSettings", JsonObject());
  projectileConfig->movementSettings =
      jsonMerge(assets->json(strf("/projectiles/physics.config:{}", physicsType)), movementSettings);

  projectileConfig->initialSpeed = config.getFloat("speed", 50);
  projectileConfig->acceleration = config.getFloat("acceleration", 0);
  projectileConfig->power = config.getFloat("power", 1);
  if (config.contains("damagePoly")) {
    projectileConfig->damagePoly = jsonToPolyF(config.get("damagePoly"));
    projectileConfig->damagePoly.scale(1.0f / TilePixels);
  }
  projectileConfig->piercing = config.getBool("piercing", false);
  projectileConfig->falldown = config.getBool("falldown", false);
  projectileConfig->bounces = config.getInt("bounces", 0);

  projectileConfig->actionOnCollide = config.getArray("actionOnCollide", {});
  projectileConfig->actionOnReap = config.getArray("actionOnReap", {});
  projectileConfig->actionOnHit = config.getArray("actionOnHit", {});
  projectileConfig->actionOnTimeout = config.getArray("actionOnTimeout", {});

  for (auto const& c : config.getArray("periodicActions", {}))
    projectileConfig->periodicActions.append(std::make_tuple(c.getFloat("time"), c.getBool("repeat", true), c));

  projectileConfig->image = AssetPath::relativeTo(path, config.getString("image"));
  projectileConfig->frameNumber = config.getUInt("frameNumber", 1);
  projectileConfig->animationCycle = config.getFloat("animationCycle", 1.0);
  projectileConfig->animationLoops = config.getBool("animationLoops", true);
  projectileConfig->windupFrames = config.getUInt("windupFrames", 0);
  projectileConfig->intangibleWindup = config.getBool("intangibleWindup", false);
  projectileConfig->winddownFrames = config.getUInt("winddownFrames", 0);
  projectileConfig->intangibleWinddown = config.getBool("intangibleWinddown", false);
  projectileConfig->flippable = config.getBool("flippable", false);
  projectileConfig->orientationLocked = config.getBool("orientationLocked", false);

  projectileConfig->fullbright = config.getBool("fullbright", false);
  projectileConfig->renderLayer = parseRenderLayer(config.getString("renderLayer", "Projectile"));

  projectileConfig->lightColor = jsonToColor(config.get("lightColor", JsonArray{0, 0, 0}));
  projectileConfig->lightPosition = jsonToVec2F(config.get("lightPosition", JsonArray{0, 0}));
  if (auto lightType = config.optString("lightType"))
    projectileConfig->lightType = LightTypeNames.getLeft(*lightType);
  else
    projectileConfig->lightType = (LightType)config.getBool("pointLight", false);

  projectileConfig->persistentAudio = config.getString("persistentAudio", "");

  // Initialize timeToLive after animationCycle so we can have the default be
  // based on animationCycle
  if (!projectileConfig->animationLoops)
    projectileConfig->timeToLive = config.getFloat("timeToLive", projectileConfig->animationCycle);
  else
    projectileConfig->timeToLive = config.getFloat("timeToLive", 5.0);

  projectileConfig->damageKindImage = config.getString("damageKindImage", "");

  projectileConfig->damageKind = config.getString("damageKind", "");
  projectileConfig->damageType = config.getString("damageType", "damage");
  projectileConfig->damageTeam = config.get("damageTeam", Json());
  projectileConfig->damageRepeatGroup = config.optString("damageRepeatGroup");
  projectileConfig->damageRepeatTimeout = config.optFloat("damageRepeatTimeout");

  if (!projectileConfig->damageKindImage.empty())
    projectileConfig->damageKindImage =
        AssetPath::relativeTo(projectileConfig->directory, projectileConfig->damageKindImage);

  projectileConfig->statusEffects = config.getArray("statusEffects", {}).transformed(jsonToEphemeralStatusEffect);

  projectileConfig->emitters = jsonToStringSet(config.getArray("emitters", JsonArray()));

  projectileConfig->hydrophobic = config.getBool("hydrophobic", false);

  projectileConfig->rayCheckToSource = config.getBool("rayCheckToSource", false);
  projectileConfig->knockback = config.getFloat("knockback", 0.0f);
  projectileConfig->knockbackDirectional = config.getBool("knockbackDirectional", false);

  projectileConfig->onlyHitTerrain = config.getBool("onlyHitTerrain", false);

  projectileConfig->clientEntityMode = ClientEntityModeNames.getLeft(config.getString("clientEntityMode", "ClientMasterAllowed"));
  projectileConfig->masterOnly = config.getBool("masterOnly", false);

  projectileConfig->scripts =
      jsonToStringList(config.get("scripts", JsonArray())).transformed(std::bind(AssetPath::relativeTo, path, std::placeholders::_1));

  projectileConfig->physicsForces = config.get("physicsForces", JsonObject());
  projectileConfig->physicsCollisions = config.get("physicsCollisions", JsonObject());

  projectileConfig->persistentStatusEffects = config.getArray("persistentStatusEffects", JsonArray()).transformed(jsonToPersistentStatusEffect);
  projectileConfig->statusEffectArea = jsonToPolyF(config.get("statusEffectArea", JsonArray()));

  return projectileConfig;
}

}
