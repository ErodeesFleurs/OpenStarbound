import std;
import star.optional;
import star.string;
import star.list;
import star.map;
import star.json;
import star.lua;
import star.format;
import star.lua_game_converters;
import star.lua_converters;
import star.inventory_types;
import star.collision_block;
import star.platformer_astar_types;
import star.actor_movement_controller;
import star.status_types;
import star.damage;
import star.damage_types;
import star.liquid_types;
import star.game_types;
import star.drawable;
import star.asset_path;
import star.color;
import star.physics_entity;
import star.force_regions;
import star.mixer;
import star.bi_map;

import star.collection_database;
import star.lua_root;
import star.root;
import star.image_metadata_database;

import star.behavior_database;
import star.behavior_state;
import star.platformer_astar;

import star.vector;
import star.matrix3;
import star.line;
import star.rect;
import star.poly;
namespace Star {

namespace {

[[nodiscard]] std::expected<void, StarException> setFields(LuaTable&) {
  return {};
}

template <typename Key, typename Value, typename... Rest>
[[nodiscard]] std::expected<void, StarException> setFields(LuaTable& table, Key&& key, Value&& value, Rest&&... rest) {
  auto result = table.set(std::forward<Key>(key), std::forward<Value>(value));
  if (!result)
    return std::unexpected(std::move(result.error()));
  return setFields(table, std::forward<Rest>(rest)...);
}

template <typename... Fields>
[[nodiscard]] std::expected<LuaValue, StarException> makeTable(LuaEngine& engine, Fields&&... fields) {
  auto created = engine.createTable();
  if (!created)
    return std::unexpected(std::move(created.error()));
  auto table = std::move(*created);
  auto result = setFields(table, std::forward<Fields>(fields)...);
  if (!result)
    return std::unexpected(std::move(result.error()));
  return table;
}

template <typename T, typename Key>
[[nodiscard]] std::expected<std::optional<T>, StarException> maybeField(LuaEngine& engine, LuaTable const& table, Key&& key) {
  auto value = table.get<LuaValue>(std::forward<Key>(key));
  if (!value)
    return std::unexpected(std::move(value.error()));
  return engine.luaMaybeTo<T>(*value);
}

[[nodiscard]] std::expected<bool, StarException> readFields(LuaEngine&, LuaTable const&) {
  return true;
}

// A failed probe is distinct from a failed table operation or converter.
template <typename Key, typename Value, typename... Rest>
[[nodiscard]] std::expected<bool, StarException> readFields(LuaEngine& engine, LuaTable const& table, Key&& key, Value& value, Rest&&... rest) {
  auto converted = maybeField<Value>(engine, table, std::forward<Key>(key));
  if (!converted)
    return std::unexpected(std::move(converted.error()));
  if (!*converted)
    return false;
  value = std::move(**converted);
  return readFields(engine, table, std::forward<Rest>(rest)...);
}

template <typename Enum>
[[nodiscard]] std::expected<String const*, StarException> enumName(std::expected<EnumMap<Enum>, StarException> const& names, Enum value) {
  if (!names)
    return std::unexpected(names.error());
  return names->getRight(value);
}

template <typename Enum>
[[nodiscard]] std::expected<std::optional<Enum>, StarException> enumValue(std::expected<EnumMap<Enum>, StarException> const& names, String const& value) {
  if (!names)
    return std::unexpected(names.error());
  return names->maybeLeft(value);
}

template <typename Enum>
[[nodiscard]] std::expected<Enum, StarException> requiredEnum(std::expected<EnumMap<Enum>, StarException> const& names, String const& value) {
  if (!names)
    return std::unexpected(names.error());
  auto result = names->getLeft(value);
  if (!result)
    return std::unexpected(std::move(result.error()));
  return **result;
}

}

std::expected<LuaValue, StarException> LuaConverter<InventorySlot>::from(LuaEngine& engine, InventorySlot k) {
  if (auto equipment = std::get_if<EquipmentSlot>(&k)) {
    auto name = enumName(EquipmentSlotNames, *equipment);
    if (!name)
      return std::unexpected(std::move(name.error()));
    return engine.createString(**name);
  } else if (auto bag = std::get_if<BagSlot>(&k)) {
    return makeTable(engine, 1, bag->first, 2, bag->second);
  } else if (std::holds_alternative<SwapSlot>(k)) {
    return engine.createString("swap");
  } else if (std::holds_alternative<TrashSlot>(k)) {
    return engine.createString("trash");
  }
  return LuaNil;
}

std::expected<std::optional<InventorySlot>, StarException> LuaConverter<InventorySlot>::to(LuaEngine& engine, LuaValue const& v) {
  if (auto str = std::get_if<LuaString>(&v)) {
    auto string = str->toString();
    auto swap = string.equalsIgnoreCase("swap");
    if (!swap)
      return std::unexpected(std::move(swap.error()));
    if (*swap)
      return InventorySlot(SwapSlot());
    auto trash = string.equalsIgnoreCase("trash");
    if (!trash)
      return std::unexpected(std::move(trash.error()));
    if (*trash)
      return InventorySlot(TrashSlot());
    auto equipment = enumValue(EquipmentSlotNames, string);
    if (!equipment)
      return std::unexpected(std::move(equipment.error()));
    if (*equipment)
      return InventorySlot(**equipment);
  } else if (auto table = std::get_if<LuaTable>(&v)) {
    String name;
    unsigned index;
    auto read = readFields(engine, *table, 1, name, 2, index);
    if (!read)
      return std::unexpected(std::move(read.error()));
    if (!*read)
      return std::nullopt;
    return InventorySlot(BagSlot(std::move(name), (std::uint8_t)index));
  }
  return std::nullopt;
}

std::expected<LuaValue, StarException> LuaConverter<CollisionKind>::from(LuaEngine& engine, CollisionKind k) {
  auto name = enumName(CollisionKindNames, k);
  if (!name)
    return std::unexpected(std::move(name.error()));
  return engine.createString(**name);
}

std::expected<std::optional<CollisionKind>, StarException> LuaConverter<CollisionKind>::to(LuaEngine&, LuaValue const& v) {
  if (auto str = std::get_if<LuaString>(&v))
    return enumValue(CollisionKindNames, str->toString());
  return std::nullopt;
}

std::expected<LuaValue, StarException> LuaConverter<CollisionSet>::from(LuaEngine& engine, CollisionSet const& s) {
  if (!CollisionKindNames)
    return std::unexpected(CollisionKindNames.error());
  auto created = engine.createTable();
  if (!created)
    return std::unexpected(std::move(created.error()));
  auto table = std::move(*created);
  int i = 1;
  for (auto const& value : *CollisionKindNames) {
    if (s.contains(value.first)) {
      auto result = table.set(i++, value.second);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  }
  return table;
}

std::expected<std::optional<CollisionSet>, StarException> LuaConverter<CollisionSet>::to(LuaEngine& engine, LuaValue const& v) {
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  CollisionSet result;
  bool compatible = true;
  auto iterated = table->iterate([&](LuaValue, LuaValue value) -> std::expected<bool, StarException> {
    auto kind = engine.luaMaybeTo<CollisionKind>(std::move(value));
    if (!kind)
      return std::unexpected(std::move(kind.error()));
    if (!*kind) {
      compatible = false;
      return false;
    }
    result.insert(**kind);
    return true;
  });
  if (!iterated)
    return std::unexpected(std::move(iterated.error()));
  if (!compatible)
    return std::nullopt;
  return result;
}

std::expected<LuaValue, StarException> LuaConverter<PlatformerAStar::Path>::from(LuaEngine& engine, PlatformerAStar::Path const& path) {
  auto convertNode = [&engine](PlatformerAStar::Node const& node) {
    return makeTable(engine, "position", node.position, "velocity", node.velocity);
  };
  auto created = engine.createTable();
  if (!created)
    return std::unexpected(std::move(created.error()));
  auto pathTable = std::move(*created);
  int index = 1;
  for (auto const& edge : path) {
    auto action = PlatformerAStar::actionName(edge.action);
    if (!action)
      return std::unexpected(std::move(action.error()));
    auto source = convertNode(edge.source);
    if (!source)
      return std::unexpected(std::move(source.error()));
    auto target = convertNode(edge.target);
    if (!target)
      return std::unexpected(std::move(target.error()));
    auto converted = makeTable(engine, "cost", edge.cost, "action", **action,
        "jumpVelocity", edge.jumpVelocity, "source", std::move(*source), "target", std::move(*target));
    if (!converted)
      return std::unexpected(std::move(converted.error()));
    auto inserted = pathTable.set(index++, std::move(*converted));
    if (!inserted)
      return std::unexpected(std::move(inserted.error()));
  }
  return pathTable;
}

std::expected<LuaMethods<std::shared_ptr<PlatformerAStar::PathFinder>>, StarException> LuaUserDataMethods<std::shared_ptr<PlatformerAStar::PathFinder>>::make() {
  LuaMethods<std::shared_ptr<PlatformerAStar::PathFinder>> methods;
  auto explore = methods.registerMethodWithSignature<std::optional<bool>, std::shared_ptr<PlatformerAStar::PathFinder> const&, std::optional<unsigned>>(
      "explore", std::mem_fn(&PlatformerAStar::PathFinder::explore));
  if (!explore)
    return std::unexpected(std::move(explore.error()));
  auto result = methods.registerMethodWithSignature<std::optional<PlatformerAStar::Path>, std::shared_ptr<PlatformerAStar::PathFinder> const&>(
      "result", std::mem_fn(&PlatformerAStar::PathFinder::result));
  if (!result)
    return std::unexpected(std::move(result.error()));
  return methods;
}

std::expected<std::optional<PlatformerAStar::Parameters>, StarException> LuaConverter<PlatformerAStar::Parameters>::to(LuaEngine& engine, LuaValue const& v) {
  PlatformerAStar::Parameters p;
  p.returnBest = false;
  p.mustEndOnGround = false;
  p.enableWalkSpeedJumps = false;
  p.enableVerticalJumpAirControl = false;
  if (std::holds_alternative<LuaNilType>(v))
    return p;
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  std::optional<bool> returnBest, mustEndOnGround, enableWalkSpeedJumps, enableVerticalJumpAirControl;
  auto read = readFields(engine, *table,
      "maxDistance", p.maxDistance, "returnBest", returnBest, "mustEndOnGround", mustEndOnGround,
      "enableWalkSpeedJumps", enableWalkSpeedJumps, "enableVerticalJumpAirControl", enableVerticalJumpAirControl,
      "swimCost", p.swimCost, "jumpCost", p.jumpCost, "liquidJumpCost", p.liquidJumpCost,
      "dropCost", p.dropCost, "boundBox", p.boundBox, "standingBoundBox", p.standingBoundBox,
      "droppingBoundBox", p.droppingBoundBox, "smallJumpMultiplier", p.smallJumpMultiplier,
      "jumpDropXMultiplier", p.jumpDropXMultiplier, "maxFScore", p.maxFScore,
      "maxNodesToSearch", p.maxNodesToSearch, "maxLandingVelocity", p.maxLandingVelocity);
  if (!read)
    return std::unexpected(std::move(read.error()));
  if (!*read)
    return std::nullopt;
  p.returnBest = returnBest.value_or(false);
  p.mustEndOnGround = mustEndOnGround.value_or(false);
  p.enableWalkSpeedJumps = enableWalkSpeedJumps.value_or(false);
  p.enableVerticalJumpAirControl = enableVerticalJumpAirControl.value_or(false);
  return p;
}

std::expected<LuaValue, StarException> LuaConverter<ActorJumpProfile>::from(LuaEngine& engine, ActorJumpProfile const& v) {
  return makeTable(engine, "jumpSpeed", v.jumpSpeed, "jumpControlForce", v.jumpControlForce,
      "jumpInitialPercentage", v.jumpInitialPercentage, "jumpHoldTime", v.jumpHoldTime,
      "jumpTotalHoldTime", v.jumpTotalHoldTime, "multiJump", v.multiJump, "reJumpDelay", v.reJumpDelay,
      "autoJump", v.autoJump, "collisionCancelled", v.collisionCancelled);
}

std::expected<std::optional<ActorJumpProfile>, StarException> LuaConverter<ActorJumpProfile>::to(LuaEngine& engine, LuaValue const& v) {
  if (std::holds_alternative<LuaNilType>(v))
    return ActorJumpProfile();
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  ActorJumpProfile result;
  auto read = readFields(engine, *table, "jumpSpeed", result.jumpSpeed, "jumpControlForce", result.jumpControlForce,
      "jumpInitialPercentage", result.jumpInitialPercentage, "jumpHoldTime", result.jumpHoldTime,
      "jumpTotalHoldTime", result.jumpTotalHoldTime, "multiJump", result.multiJump, "reJumpDelay", result.reJumpDelay,
      "autoJump", result.autoJump, "collisionCancelled", result.collisionCancelled);
  if (!read)
    return std::unexpected(std::move(read.error()));
  if (!*read)
    return std::nullopt;
  return result;
}

std::expected<LuaValue, StarException> LuaConverter<ActorMovementParameters>::from(LuaEngine& engine, ActorMovementParameters const& v) {
  return makeTable(engine, "mass", v.mass, "gravityMultiplier", v.gravityMultiplier,
      "liquidBuoyancy", v.liquidBuoyancy, "airBuoyancy", v.airBuoyancy, "bounceFactor", v.bounceFactor,
      "slopeSlidingFactor", v.slopeSlidingFactor, "maxMovementPerStep", v.maxMovementPerStep,
      "maximumCorrection", v.maximumCorrection, "speedLimit", v.speedLimit, "standingPoly", v.standingPoly,
      "crouchingPoly", v.crouchingPoly, "stickyCollision", v.stickyCollision, "stickyForce", v.stickyForce,
      "walkSpeed", v.walkSpeed, "runSpeed", v.runSpeed, "flySpeed", v.flySpeed,
      "airFriction", v.airFriction, "liquidFriction", v.liquidFriction,
      "minimumLiquidPercentage", v.minimumLiquidPercentage, "liquidImpedance", v.liquidImpedance,
      "normalGroundFriction", v.normalGroundFriction, "ambulatingGroundFriction", v.ambulatingGroundFriction,
      "groundForce", v.groundForce, "airForce", v.airForce, "liquidForce", v.liquidForce,
      "airJumpProfile", v.airJumpProfile, "liquidJumpProfile", v.liquidJumpProfile,
      "fallStatusSpeedMin", v.fallStatusSpeedMin, "fallThroughSustainFrames", v.fallThroughSustainFrames,
      "maximumPlatformCorrection", v.maximumPlatformCorrection,
      "maximumPlatformCorrectionVelocityFactor", v.maximumPlatformCorrectionVelocityFactor,
      "physicsEffectCategories", v.physicsEffectCategories, "groundMovementMinimumSustain", v.groundMovementMinimumSustain,
      "groundMovementMaximumSustain", v.groundMovementMaximumSustain, "groundMovementCheckDistance", v.groundMovementCheckDistance,
      "collisionEnabled", v.collisionEnabled, "frictionEnabled", v.frictionEnabled, "gravityEnabled", v.gravityEnabled);
}

std::expected<std::optional<ActorMovementParameters>, StarException> LuaConverter<ActorMovementParameters>::to(LuaEngine& engine, LuaValue const& v) {
  if (std::holds_alternative<LuaNilType>(v))
    return ActorMovementParameters();
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  ActorMovementParameters result;
  std::optional<PolyF> standingFallback, crouchingFallback;
  auto read = readFields(engine, *table,
      "mass", result.mass, "gravityMultiplier", result.gravityMultiplier,
      "liquidBuoyancy", result.liquidBuoyancy, "airBuoyancy", result.airBuoyancy, "bounceFactor", result.bounceFactor,
      "slopeSlidingFactor", result.slopeSlidingFactor, "maxMovementPerStep", result.maxMovementPerStep,
      "maximumCorrection", result.maximumCorrection, "speedLimit", result.speedLimit,
      "standingPoly", result.standingPoly, "collisionPoly", standingFallback,
      "crouchingPoly", result.crouchingPoly, "collisionPoly", crouchingFallback,
      "stickyCollision", result.stickyCollision, "stickyForce", result.stickyForce,
      "walkSpeed", result.walkSpeed, "runSpeed", result.runSpeed, "flySpeed", result.flySpeed,
      "airFriction", result.airFriction, "liquidFriction", result.liquidFriction,
      "minimumLiquidPercentage", result.minimumLiquidPercentage, "liquidImpedance", result.liquidImpedance,
      "normalGroundFriction", result.normalGroundFriction, "ambulatingGroundFriction", result.ambulatingGroundFriction,
      "groundForce", result.groundForce, "airForce", result.airForce, "liquidForce", result.liquidForce,
      "airJumpProfile", result.airJumpProfile, "liquidJumpProfile", result.liquidJumpProfile,
      "fallStatusSpeedMin", result.fallStatusSpeedMin, "fallThroughSustainFrames", result.fallThroughSustainFrames,
      "maximumPlatformCorrection", result.maximumPlatformCorrection,
      "maximumPlatformCorrectionVelocityFactor", result.maximumPlatformCorrectionVelocityFactor,
      "physicsEffectCategories", result.physicsEffectCategories, "groundMovementMinimumSustain", result.groundMovementMinimumSustain,
      "groundMovementMaximumSustain", result.groundMovementMaximumSustain, "groundMovementCheckDistance", result.groundMovementCheckDistance,
      "collisionEnabled", result.collisionEnabled, "frictionEnabled", result.frictionEnabled, "gravityEnabled", result.gravityEnabled);
  if (!read)
    return std::unexpected(std::move(read.error()));
  if (!*read)
    return std::nullopt;
  if (!result.standingPoly)
    result.standingPoly = std::move(standingFallback);
  if (!result.crouchingPoly)
    result.crouchingPoly = std::move(crouchingFallback);
  return result;
}

std::expected<LuaValue, StarException> LuaConverter<ActorMovementModifiers>::from(LuaEngine& engine, ActorMovementModifiers const& v) {
  return makeTable(engine, "groundMovementModifier", v.groundMovementModifier,
      "liquidMovementModifier", v.liquidMovementModifier, "speedModifier", v.speedModifier,
      "airJumpModifier", v.airJumpModifier, "liquidJumpModifier", v.liquidJumpModifier,
      "runningSuppressed", v.runningSuppressed, "jumpingSuppressed", v.jumpingSuppressed,
      "facingSuppressed", v.facingSuppressed, "movementSuppressed", v.movementSuppressed);
}

std::expected<std::optional<ActorMovementModifiers>, StarException> LuaConverter<ActorMovementModifiers>::to(LuaEngine& engine, LuaValue const& v) {
  if (std::holds_alternative<LuaNilType>(v))
    return ActorMovementModifiers();
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  std::optional<float> ground, liquid, speed, airJump, liquidJump;
  std::optional<bool> running, jumping, facing, movement;
  auto read = readFields(engine, *table, "groundMovementModifier", ground, "liquidMovementModifier", liquid,
      "speedModifier", speed, "airJumpModifier", airJump, "liquidJumpModifier", liquidJump,
      "runningSuppressed", running, "jumpingSuppressed", jumping,
      "facingSuppressed", facing, "movementSuppressed", movement);
  if (!read)
    return std::unexpected(std::move(read.error()));
  if (!*read)
    return std::nullopt;
  ActorMovementModifiers result;
  result.groundMovementModifier = ground.value_or(1.0f);
  result.liquidMovementModifier = liquid.value_or(1.0f);
  result.speedModifier = speed.value_or(1.0f);
  result.airJumpModifier = airJump.value_or(1.0f);
  result.liquidJumpModifier = liquidJump.value_or(1.0f);
  result.runningSuppressed = running.value_or(false);
  result.jumpingSuppressed = jumping.value_or(false);
  result.facingSuppressed = facing.value_or(false);
  result.movementSuppressed = movement.value_or(false);
  return result;
}

std::expected<LuaValue, StarException> LuaConverter<StatModifier>::from(LuaEngine& engine, StatModifier const& v) {
  auto json = jsonFromStatModifier(v);
  if (!json)
    return std::unexpected(std::move(json.error()));
  return engine.luaFrom(std::move(*json));
}

std::expected<std::optional<StatModifier>, StarException> LuaConverter<StatModifier>::to(LuaEngine& engine, LuaValue v) {
  auto json = engine.luaMaybeTo<Json>(std::move(v));
  if (!json)
    return std::unexpected(std::move(json.error()));
  if (!*json)
    return std::nullopt;
  auto modifier = jsonToStatModifier(**json);
  if (!modifier) {
    if (modifier.error().is<JsonException>())
      return std::nullopt;
    return std::unexpected(std::move(modifier.error()));
  }
  return std::move(*modifier);
}

std::expected<LuaValue, StarException> LuaConverter<EphemeralStatusEffect>::from(LuaEngine& engine, EphemeralStatusEffect const& v) {
  return makeTable(engine, "effect", v.uniqueEffect, "duration", v.duration);
}

std::expected<std::optional<EphemeralStatusEffect>, StarException> LuaConverter<EphemeralStatusEffect>::to(LuaEngine& engine, LuaValue const& v) {
  if (auto str = std::get_if<LuaString>(&v))
    return EphemeralStatusEffect{UniqueStatusEffect(str->ptr()), {}};
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  auto effect = maybeField<String>(engine, *table, "effect");
  if (!effect)
    return std::unexpected(std::move(effect.error()));
  auto duration = maybeField<std::optional<float>>(engine, *table, "duration");
  if (!duration)
    return std::unexpected(std::move(duration.error()));
  if (!*duration) {
    duration = maybeField<std::optional<float>>(engine, *table, "duratino");
    if (!duration)
      return std::unexpected(std::move(duration.error()));
  }
  if (*effect && *duration)
    return EphemeralStatusEffect{std::move(**effect), std::move(**duration)};
  return std::nullopt;
}

std::expected<LuaValue, StarException> LuaConverter<DamageRequest>::from(LuaEngine& engine, DamageRequest const& v) {
  auto hitType = enumName(HitTypeNames, v.hitType);
  if (!hitType)
    return std::unexpected(std::move(hitType.error()));
  auto damageType = enumName(DamageTypeNames, v.damageType);
  if (!damageType)
    return std::unexpected(std::move(damageType.error()));
  return makeTable(engine, "hitType", **hitType, "damageType", **damageType, "damage", v.damage,
      "knockbackMomentum", v.knockbackMomentum, "sourceEntityId", v.sourceEntityId,
      "damageSourceKind", v.damageSourceKind, "statusEffects", v.statusEffects);
}

std::expected<std::optional<DamageRequest>, StarException> LuaConverter<DamageRequest>::to(LuaEngine& engine, LuaValue const& v) {
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  DamageRequest result;
  std::optional<String> hitType, damageType, damageSourceKind;
  std::optional<Vec2F> knockback;
  std::optional<EntityId> source;
  std::optional<List<EphemeralStatusEffect>> effects;
  auto read = readFields(engine, *table, "hitType", hitType, "damageType", damageType,
      "damage", result.damage, "knockbackMomentum", knockback, "sourceEntityId", source,
      "damageSourceKind", damageSourceKind, "statusEffects", effects);
  if (!read)
    return std::unexpected(std::move(read.error()));
  if (!*read)
    return std::nullopt;
  if (hitType) {
    auto converted = enumValue(HitTypeNames, *hitType);
    if (!converted)
      return std::unexpected(std::move(converted.error()));
    if (!*converted)
      return std::nullopt;
    result.hitType = **converted;
  }
  if (damageType) {
    auto converted = enumValue(DamageTypeNames, *damageType);
    if (!converted)
      return std::unexpected(std::move(converted.error()));
    if (!*converted)
      return std::nullopt;
    result.damageType = **converted;
  }
  if (knockback)
    result.knockbackMomentum = *knockback;
  if (source)
    result.sourceEntityId = *source;
  if (damageSourceKind)
    result.damageSourceKind = std::move(*damageSourceKind);
  if (effects)
    result.statusEffects = std::move(*effects);
  return result;
}

std::expected<LuaValue, StarException> LuaConverter<DamageNotification>::from(LuaEngine& engine, DamageNotification const& v) {
  auto hitType = enumName(HitTypeNames, v.hitType);
  if (!hitType)
    return std::unexpected(std::move(hitType.error()));
  return makeTable(engine, "sourceEntityId", v.sourceEntityId, "targetEntityId", v.targetEntityId,
      "position", v.position, "damageDealt", v.damageDealt, "healthLost", v.healthLost,
      "hitType", **hitType, "damageSourceKind", v.damageSourceKind, "targetMaterialKind", v.targetMaterialKind);
}

std::expected<std::optional<DamageNotification>, StarException> LuaConverter<DamageNotification>::to(LuaEngine& engine, LuaValue const& v) {
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  DamageNotification result;
  String hitType;
  auto read = readFields(engine, *table, "sourceEntityId", result.sourceEntityId, "targetEntityId", result.targetEntityId,
      "position", result.position, "damageDealt", result.damageDealt, "healthLost", result.healthLost,
      "hitType", hitType, "damageSourceKind", result.damageSourceKind, "targetMaterialKind", result.targetMaterialKind);
  if (!read)
    return std::unexpected(std::move(read.error()));
  if (!*read)
    return std::nullopt;
  auto converted = enumValue(HitTypeNames, hitType);
  if (!converted)
    return std::unexpected(std::move(converted.error()));
  if (!*converted)
    return std::nullopt;
  result.hitType = **converted;
  return result;
}

std::expected<LuaValue, StarException> LuaConverter<LiquidLevel>::from(LuaEngine& engine, LiquidLevel const& v) {
  return makeTable(engine, 1, v.liquid, 2, v.level);
}

std::expected<std::optional<LiquidLevel>, StarException> LuaConverter<LiquidLevel>::to(LuaEngine& engine, LuaValue const& v) {
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  LiquidId liquid;
  std::uint8_t level;
  auto read = readFields(engine, *table, 1, liquid, 2, level);
  if (!read)
    return std::unexpected(std::move(read.error()));
  if (!*read)
    return std::nullopt;
  return LiquidLevel(liquid, level);
}

std::expected<LuaValue, StarException> LuaConverter<Drawable>::from(LuaEngine& engine, Drawable const& v) {
  auto created = engine.createTable();
  if (!created)
    return std::unexpected(std::move(created.error()));
  auto table = std::move(*created);
  if (auto line = std::get_if<Drawable::LinePart>(&v.part)) {
    auto result = setFields(table, "line", line->line, "width", line->width);
    if (!result)
      return std::unexpected(std::move(result.error()));
  } else if (auto poly = std::get_if<Drawable::PolyPart>(&v.part)) {
    auto result = table.set("poly", poly->poly);
    if (!result)
      return std::unexpected(std::move(result.error()));
  } else if (auto image = std::get_if<Drawable::ImagePart>(&v.part)) {
    auto result = setFields(table, "image", AssetPath::join(image->image), "transformation", image->transformation);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  auto result = setFields(table, "position", v.position, "color", v.color, "fullbright", v.fullbright);
  if (!result)
    return std::unexpected(std::move(result.error()));
  return table;
}

std::expected<std::optional<Drawable>, StarException> LuaConverter<Drawable>::to(LuaEngine& engine, LuaValue const& v) {
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  std::optional<Color> color;
  std::optional<Line2F> line;
  auto read = readFields(engine, *table, "color", color, "line", line);
  if (!read)
    return std::unexpected(std::move(read.error()));
  if (!*read)
    return std::nullopt;
  Drawable drawable;
  if (line) {
    float width;
    auto readWidth = readFields(engine, *table, "width", width);
    if (!readWidth)
      return std::unexpected(std::move(readWidth.error()));
    if (!*readWidth)
      return std::nullopt;
    drawable = Drawable::makeLine(std::move(*line), width, color.value_or(Color::White));
  } else {
    std::optional<PolyF> poly;
    auto readPoly = readFields(engine, *table, "poly", poly);
    if (!readPoly)
      return std::unexpected(std::move(readPoly.error()));
    if (!*readPoly)
      return std::nullopt;
    if (poly) {
      drawable = Drawable::makePoly(std::move(*poly), color.value_or(Color::White));
    } else {
      std::optional<String> image;
      auto readImage = readFields(engine, *table, "image", image);
      if (!readImage)
        return std::unexpected(std::move(readImage.error()));
      if (!*readImage || !image)
        return std::nullopt;
      std::optional<bool> centered;
      auto readCentered = readFields(engine, *table, "centered", centered);
      if (!readCentered)
        return std::unexpected(std::move(readCentered.error()));
      if (!*readCentered)
        return std::nullopt;
      auto root = luaExecutionRoot(engine);
      if (!root)
        return std::unexpected(std::move(root.error()));
      auto metadata = (*root)->imageMetadataDatabase();
      if (!metadata)
        return std::unexpected(std::move(metadata.error()));
      auto converted = Drawable::makeImage(**metadata, std::move(*image), 1.0f, centered.value_or(true), Vec2F(), color.value_or(Color::White));
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      drawable = std::move(*converted);
    }
  }
  std::optional<Mat3F> transformation;
  auto readTransformation = readFields(engine, *table, "transformation", transformation);
  if (!readTransformation)
    return std::unexpected(std::move(readTransformation.error()));
  if (!*readTransformation)
    return std::nullopt;
  if (transformation)
    drawable.transform(*transformation);
  std::optional<float> rotation;
  auto readRotation = readFields(engine, *table, "rotation", rotation);
  if (!readRotation)
    return std::unexpected(std::move(readRotation.error()));
  if (!*readRotation)
    return std::nullopt;
  if (rotation)
    drawable.rotate(*rotation);
  bool mirrored;
  auto readMirrored = readFields(engine, *table, "mirrored", mirrored);
  if (!readMirrored)
    return std::unexpected(std::move(readMirrored.error()));
  if (!*readMirrored)
    return std::nullopt;
  if (mirrored)
    drawable.scale(Vec2F(-1, 1));
  std::optional<float> scale;
  auto readScale = readFields(engine, *table, "scale", scale);
  if (!readScale)
    return std::unexpected(std::move(readScale.error()));
  if (!*readScale)
    return std::nullopt;
  if (scale)
    drawable.scale(*scale);
  std::optional<Vec2F> position;
  auto readPosition = readFields(engine, *table, "position", position);
  if (!readPosition)
    return std::unexpected(std::move(readPosition.error()));
  if (!*readPosition)
    return std::nullopt;
  if (position)
    drawable.translate(*position);
  auto readFullbright = readFields(engine, *table, "fullbright", drawable.fullbright);
  if (!readFullbright)
    return std::unexpected(std::move(readFullbright.error()));
  if (!*readFullbright)
    return std::nullopt;
  return drawable;
}

std::expected<LuaValue, StarException> LuaConverter<Collection>::from(LuaEngine& engine, Collection const& c) {
  auto type = enumName(CollectionTypeNames, c.type);
  if (!type)
    return std::unexpected(std::move(type.error()));
  return makeTable(engine, "name", c.name, "type", **type, "title", c.title);
}

std::expected<std::optional<Collection>, StarException> LuaConverter<Collection>::to(LuaEngine& engine, LuaValue const& v) {
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  String name, type, title;
  auto read = readFields(engine, *table, "name", name, "type", type, "title", title);
  if (!read)
    return std::unexpected(std::move(read.error()));
  if (!*read)
    return std::nullopt;
  auto converted = enumValue(CollectionTypeNames, type);
  if (!converted)
    return std::unexpected(std::move(converted.error()));
  if (!*converted)
    return std::nullopt;
  return Collection(name, **converted, title);
}

std::expected<LuaValue, StarException> LuaConverter<Collectable>::from(LuaEngine& engine, Collectable const& c) {
  return makeTable(engine, "name", c.name, "order", c.order, "title", c.title, "description", c.description, "icon", c.icon);
}

std::expected<std::optional<Collectable>, StarException> LuaConverter<Collectable>::to(LuaEngine& engine, LuaValue const& v) {
  auto table = std::get_if<LuaTable>(&v);
  if (!table)
    return std::nullopt;
  auto name = maybeField<String>(engine, *table, "name");
  if (!name)
    return std::unexpected(std::move(name.error()));
  if (!*name)
    return std::nullopt;
  auto order = maybeField<int>(engine, *table, "order");
  if (!order)
    return std::unexpected(std::move(order.error()));
  auto title = maybeField<String>(engine, *table, "title");
  if (!title)
    return std::unexpected(std::move(title.error()));
  auto description = maybeField<String>(engine, *table, "description");
  if (!description)
    return std::unexpected(std::move(description.error()));
  auto icon = maybeField<String>(engine, *table, "icon");
  if (!icon)
    return std::unexpected(std::move(icon.error()));
  return Collectable(std::move(**name), order->value_or(0), title->value_or(""), description->value_or(""), icon->value_or(""));
}

std::expected<LuaValue, StarException> LuaConverter<PhysicsMovingCollision>::from(LuaEngine& engine, PhysicsMovingCollision const& v) {
  auto categoryTable = makeTable(engine,
      v.categoryFilter.type == PhysicsCategoryFilter::Type::Whitelist ? "categoryWhitelist" : "categoryBlacklist",
      v.categoryFilter.categories);
  if (!categoryTable)
    return std::unexpected(std::move(categoryTable.error()));
  return makeTable(engine, "position", v.position, "collision", v.collision, "collisionKind", v.collisionKind,
      "categoryFilter", std::move(*categoryTable));
}

std::expected<LuaMethods<std::weak_ptr<BehaviorState>>, StarException> LuaUserDataMethods<std::weak_ptr<BehaviorState>>::make() {
  LuaMethods<std::weak_ptr<BehaviorState>> methods;
  auto run = methods.registerMethodWithSignature<NodeStatus, std::weak_ptr<BehaviorState>, float>(
    "run", [](std::weak_ptr<BehaviorState> const& behavior, float dt) -> std::expected<NodeStatus, StarException> {
      auto state = behavior.lock();
      if (!state)
        return std::unexpected(StarException("Use of expired blackboard"));
      return state->run(dt);
    });
  if (!run)
    return std::unexpected(std::move(run.error()));
  auto clear = methods.registerMethodWithSignature<void, std::weak_ptr<BehaviorState>>(
    "clear", [](std::weak_ptr<BehaviorState> const& behavior) -> std::expected<void, StarException> {
      auto state = behavior.lock();
      if (!state)
        return std::unexpected(StarException("Use of expired blackboard"));
      state->clear();
      return {};
    });
  if (!clear)
    return std::unexpected(std::move(clear.error()));
  auto blackboard = methods.registerMethodWithSignature<std::weak_ptr<Blackboard>, std::weak_ptr<BehaviorState>>(
    "blackboard", [](std::weak_ptr<BehaviorState> const& behavior) -> std::expected<std::weak_ptr<Blackboard>, StarException> {
      auto state = behavior.lock();
      if (!state)
        return std::unexpected(StarException("Use of expired blackboard"));
      return state->blackboardPtr();
    });
  if (!blackboard)
    return std::unexpected(std::move(blackboard.error()));
  return methods;
}

std::expected<LuaValue, StarException> LuaConverter<NodeStatus>::from(LuaEngine&, NodeStatus const& status) {
  if (status == NodeStatus::Success)
    return true;
  else if (status == NodeStatus::Failure)
    return false;
  return LuaNil;
}

std::expected<NodeStatus, StarException> LuaConverter<NodeStatus>::to(LuaEngine&, LuaValue const& v) {
  if (auto boolean = std::get_if<LuaBoolean>(&v))
    return *boolean ? NodeStatus::Success : NodeStatus::Failure;
  return NodeStatus::Running;
}

std::expected<LuaMethods<std::weak_ptr<Blackboard>>, StarException> LuaUserDataMethods<std::weak_ptr<Blackboard>>::make() {
  LuaMethods<std::weak_ptr<Blackboard>> methods;
  auto get = [](std::weak_ptr<Blackboard> const& board, NodeParameterType type, String const& key) -> std::expected<LuaValue, StarException> {
    auto locked = board.lock();
    if (!locked)
      return std::unexpected(StarException("Use of expired blackboard"));
    return locked->get(type, key);
  };
  auto set = [](std::weak_ptr<Blackboard> const& board, NodeParameterType type, String const& key, LuaValue const& value) -> std::expected<void, StarException> {
    auto locked = board.lock();
    if (!locked)
      return std::unexpected(StarException("Use of expired blackboard"));
    return locked->set(type, key, value);
  };
  auto getMethod = methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String, String>("get",
    [get](std::weak_ptr<Blackboard> const& board, String const& type, String const& key) -> std::expected<LuaValue, StarException> {
      auto converted = requiredEnum(NodeParameterTypeNames, type);
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      return get(board, *converted, key);
    });
  if (!getMethod)
    return std::unexpected(std::move(getMethod.error()));
  auto setMethod = methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, String, LuaValue>("set",
    [set](std::weak_ptr<Blackboard> const& board, String const& type, String const& key, LuaValue const& value) -> std::expected<void, StarException> {
      auto converted = requiredEnum(NodeParameterTypeNames, type);
      if (!converted)
        return std::unexpected(std::move(converted.error()));
      return set(board, *converted, key, value);
    });
  if (!setMethod)
    return std::unexpected(std::move(setMethod.error()));
  std::array const typedMethods{
      std::tuple{"getEntity", "setEntity", NodeParameterType::Entity},
      std::tuple{"getPosition", "setPosition", NodeParameterType::Position},
      std::tuple{"getVec2", "setVec2", NodeParameterType::Vec2},
      std::tuple{"getNumber", "setNumber", NodeParameterType::Number},
      std::tuple{"getBool", "setBool", NodeParameterType::Bool},
      std::tuple{"getList", "setList", NodeParameterType::List},
      std::tuple{"getTable", "setTable", NodeParameterType::Table},
      std::tuple{"getString", "setString", NodeParameterType::String}};
  for (auto const& [getName, setName, type] : typedMethods) {
    auto getter = methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String>(
        getName, [get, type](std::weak_ptr<Blackboard> const& board, String const& key) {
          return get(board, type, key);
        });
    if (!getter)
      return std::unexpected(std::move(getter.error()));
    auto setter = methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, LuaValue>(
        setName, [set, type](std::weak_ptr<Blackboard> const& board, String const& key, LuaValue const& value) {
          return set(board, type, key, value);
        });
    if (!setter)
      return std::unexpected(std::move(setter.error()));
  }
  return methods;
}

std::expected<LuaMethods<std::shared_ptr<AudioInstance>>, StarException> LuaUserDataMethods<std::shared_ptr<AudioInstance>>::make() {
  LuaMethods<std::shared_ptr<AudioInstance>> methods;
  auto position = methods.registerMethod("position", [](std::shared_ptr<AudioInstance> const& audio) {
    return audio->position();
  });
  if (!position)
    return std::unexpected(std::move(position.error()));
  auto setPosition = methods.registerMethod("setPosition", [](std::shared_ptr<AudioInstance> const& audio, std::optional<Vec2F> const& pos) {
    return audio->setPosition(pos);
  });
  if (!setPosition)
    return std::unexpected(std::move(setPosition.error()));
  auto translate = methods.registerMethod("translate", [](std::shared_ptr<AudioInstance> const& audio, Vec2F const& distance) {
    return audio->translate(distance);
  });
  if (!translate)
    return std::unexpected(std::move(translate.error()));
  auto rangeMultiplier = methods.registerMethod("rangeMultiplier", [](std::shared_ptr<AudioInstance> const& audio) {
    return audio->rangeMultiplier();
  });
  if (!rangeMultiplier)
    return std::unexpected(std::move(rangeMultiplier.error()));
  auto setRangeMultiplier = methods.registerMethod("setRangeMultiplier", [](std::shared_ptr<AudioInstance> const& audio, float const& value) {
    return audio->setRangeMultiplier(value);
  });
  if (!setRangeMultiplier)
    return std::unexpected(std::move(setRangeMultiplier.error()));
  auto setVolume = methods.registerMethod("setVolume", [](std::shared_ptr<AudioInstance> const& audio, float const& value, std::optional<float> const& rampTime) {
    return audio->setVolume(value, rampTime.value_or(0.0f));
  });
  if (!setVolume)
    return std::unexpected(std::move(setVolume.error()));
  auto setPitch = methods.registerMethod("setPitchMultiplier", [](std::shared_ptr<AudioInstance> const& audio, float const& value, std::optional<float> const& rampTime) {
    return audio->setPitchMultiplier(value, rampTime.value_or(0.0f));
  });
  if (!setPitch)
    return std::unexpected(std::move(setPitch.error()));
  auto loops = methods.registerMethod("loops", [](std::shared_ptr<AudioInstance> const& audio) {
    return audio->loops();
  });
  if (!loops)
    return std::unexpected(std::move(loops.error()));
  auto setLoops = methods.registerMethod("setLoops", [](std::shared_ptr<AudioInstance> const& audio, int const& value) {
    return audio->setLoops(value);
  });
  if (!setLoops)
    return std::unexpected(std::move(setLoops.error()));
  auto currentTime = methods.registerMethod("currentTime", [](std::shared_ptr<AudioInstance> const& audio) {
    return audio->currentTime();
  });
  if (!currentTime)
    return std::unexpected(std::move(currentTime.error()));
  auto totalTime = methods.registerMethod("totalTime", [](std::shared_ptr<AudioInstance> const& audio) {
    return audio->totalTime();
  });
  if (!totalTime)
    return std::unexpected(std::move(totalTime.error()));
  auto seekTime = methods.registerMethod("seekTime", [](std::shared_ptr<AudioInstance> const& audio, double const& time) {
    return audio->seekTime(time);
  });
  if (!seekTime)
    return std::unexpected(std::move(seekTime.error()));
  auto mixerGroup = methods.registerMethod("mixerGroup", [](std::shared_ptr<AudioInstance> const& audio) -> std::expected<String, StarException> {
    auto name = enumName(MixerGroupNames, audio->mixerGroup());
    if (!name)
      return std::unexpected(std::move(name.error()));
    return **name;
  });
  if (!mixerGroup)
    return std::unexpected(std::move(mixerGroup.error()));
  auto setMixerGroup = methods.registerMethod("setMixerGroup", [](std::shared_ptr<AudioInstance> const& audio, String const& group) -> std::expected<void, StarException> {
    auto converted = requiredEnum(MixerGroupNames, group);
    if (!converted)
      return std::unexpected(std::move(converted.error()));
    audio->setMixerGroup(*converted);
    return {};
  });
  if (!setMixerGroup)
    return std::unexpected(std::move(setMixerGroup.error()));
  auto setClockStart = methods.registerMethod("setClockStart", [](std::shared_ptr<AudioInstance> const& audio, std::optional<std::int64_t> const& time) {
    return audio->setClockStart(time);
  });
  if (!setClockStart)
    return std::unexpected(std::move(setClockStart.error()));
  auto setClockStop = methods.registerMethod("setClockStop", [](std::shared_ptr<AudioInstance> const& audio, std::optional<std::int64_t> const& time, std::optional<std::int64_t> const& fadeOutTime) {
    return audio->setClockStop(time, fadeOutTime.value_or(0));
  });
  if (!setClockStop)
    return std::unexpected(std::move(setClockStop.error()));
  auto stop = methods.registerMethod("stop", [](std::shared_ptr<AudioInstance> const& audio, std::optional<float> const& rampTime) {
    return audio->stop(rampTime.value_or(0.0f));
  });
  if (!stop)
    return std::unexpected(std::move(stop.error()));
  auto finished = methods.registerMethod("finished", [](std::shared_ptr<AudioInstance> const& audio) {
    return audio->finished();
  });
  if (!finished)
    return std::unexpected(std::move(finished.error()));
  return methods;
}

}
