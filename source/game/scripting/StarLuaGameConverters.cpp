#include "StarLuaGameConverters.hpp"

import std;

namespace Star {

auto LuaConverter<InventorySlot>::from(LuaEngine& engine, InventorySlot k) -> LuaValue {
  if (auto equipment = k.ptr<EquipmentSlot>())
    return engine.createString(EquipmentSlotNames.getRight(*equipment));
  else if (auto bag = k.ptr<BagSlot>()) {
    auto table = engine.createTable(2, 0);
    table.set(1, bag->first);
    table.set(2, bag->second);
    return table;
  } else if (k.is<SwapSlot>())
    return engine.createString("swap");
  else if (k.is<TrashSlot>())
    return engine.createString("trash");
  else
    return {};// avoid UB if every accounted-for case fails
}

auto LuaConverter<InventorySlot>::to(LuaEngine&, LuaValue const& v) -> std::optional<InventorySlot> {
  if (auto str = v.ptr<LuaString>()) {
    auto string = str->toString();
    if (string.equalsIgnoreCase("swap"))
      return {SwapSlot()};
    else if (string.equalsIgnoreCase("trash"))
      return {TrashSlot()};
    else if (auto equipment = EquipmentSlotNames.leftPtr(str->toString()))
      return {*equipment};
    else
      return std::nullopt;
  } else if (auto table = v.ptr<LuaTable>())
    return {BagSlot(table->get<LuaString>(1).toString(), (std::uint8_t)table->get<unsigned>(2))};
  else
    return std::nullopt;
}

auto LuaConverter<CollisionKind>::from(LuaEngine& engine, CollisionKind k) -> LuaValue {
  return engine.createString(CollisionKindNames.getRight(k));
}

auto LuaConverter<CollisionKind>::to(LuaEngine&, LuaValue const& v) -> std::optional<CollisionKind> {
  if (auto str = v.ptr<LuaString>())
    return CollisionKindNames.maybeLeft(str->ptr());
  return {};
}

auto LuaConverter<CollisionSet>::from(LuaEngine& engine, CollisionSet const& s) -> LuaValue {
  auto collisionTable = engine.createTable();
  int i = 1;
  for (auto const& v : CollisionKindNames) {
    if (s.contains(v.first)) {
      collisionTable.set(i++, v.second);
    }
  }
  return collisionTable;
}

auto LuaConverter<CollisionSet>::to(LuaEngine& engine, LuaValue const& v) -> std::optional<CollisionSet> {
  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};

  CollisionSet result;
  bool failed = false;
  table->iterate([&result, &failed, &engine](LuaValue, LuaValue value) -> bool {
    if (auto k = engine.luaMaybeTo<CollisionKind>(std::move(value))) {
      result.insert(*k);
      return true;
    } else {
      failed = true;
      return false;
    }
  });

  if (failed)
    return {};
  return result;
}

auto LuaConverter<PlatformerAStar::Path>::from(LuaEngine& engine, PlatformerAStar::Path const& path) -> LuaValue {
  auto convertNode = [&engine](PlatformerAStar::Node const& node) -> LuaTable {
    auto table = engine.createTable();
    table.set("position", node.position);
    table.set("velocity", node.velocity);
    return table;
  };

  LuaTable pathTable = engine.createTable();
  int pathTableIndex = 1;
  for (auto const& edge : path) {
    auto edgeTable = engine.createTable();
    edgeTable.set("cost", edge.cost);
    edgeTable.set("action", PlatformerAStar::ActionNames.getRight(edge.action));
    edgeTable.set("jumpVelocity", edge.jumpVelocity);
    edgeTable.set("source", convertNode(edge.source));
    edgeTable.set("target", convertNode(edge.target));
    pathTable.set(pathTableIndex++, std::move(edgeTable));
  }
  return pathTable;
}

auto LuaUserDataMethods<PlatformerAStar::PathFinder>::make() -> LuaMethods<PlatformerAStar::PathFinder> {
  LuaMethods<PlatformerAStar::PathFinder> methods;
  methods.registerMethodWithSignature<std::optional<bool>, PlatformerAStar::PathFinder&, std::optional<unsigned>>(
    "explore", mem_fn(&PlatformerAStar::PathFinder::explore));
  methods.registerMethodWithSignature<std::optional<PlatformerAStar::Path>, PlatformerAStar::PathFinder&>(
    "result", mem_fn(&PlatformerAStar::PathFinder::result));
  return methods;
}

auto LuaConverter<PlatformerAStar::Parameters>::to(LuaEngine&, LuaValue const& v) -> std::optional<PlatformerAStar::Parameters> {
  PlatformerAStar::Parameters p;
  p.returnBest = false;
  p.mustEndOnGround = false;
  p.enableWalkSpeedJumps = false;
  p.enableVerticalJumpAirControl = false;
  if (v == LuaNil)
    return p;

  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};

  try {
    p.maxDistance = table->get<std::optional<float>>("maxDistance");
    p.returnBest = table->get<std::optional<bool>>("returnBest").value_or(false);
    p.mustEndOnGround = table->get<std::optional<bool>>("mustEndOnGround").value_or(false);
    p.enableWalkSpeedJumps = table->get<std::optional<bool>>("enableWalkSpeedJumps").value_or(false);
    p.enableVerticalJumpAirControl = table->get<std::optional<bool>>("enableVerticalJumpAirControl").value_or(false);
    p.swimCost = table->get<std::optional<float>>("swimCost");
    p.jumpCost = table->get<std::optional<float>>("jumpCost");
    p.liquidJumpCost = table->get<std::optional<float>>("liquidJumpCost");
    p.dropCost = table->get<std::optional<float>>("dropCost");
    p.boundBox = table->get<RectF>("boundBox");
    p.standingBoundBox = table->get<RectF>("standingBoundBox");
    p.droppingBoundBox = table->get<RectF>("droppingBoundBox");
    p.smallJumpMultiplier = table->get<std::optional<float>>("smallJumpMultiplier");
    p.jumpDropXMultiplier = table->get<std::optional<float>>("jumpDropXMultiplier");
    p.maxFScore = table->get<double>("maxFScore");
    p.maxNodesToSearch = table->get<unsigned>("maxNodesToSearch");
    p.maxLandingVelocity = table->get<std::optional<float>>("maxLandingVelocity");
  } catch (LuaConversionException const&) {
    return {};
  }

  return p;
}

auto LuaConverter<ActorJumpProfile>::from(LuaEngine& engine, ActorJumpProfile const& v) -> LuaValue {
  auto table = engine.createTable();
  table.set("jumpSpeed", v.jumpSpeed);
  table.set("jumpControlForce", v.jumpControlForce);
  table.set("jumpInitialPercentage", v.jumpInitialPercentage);
  table.set("jumpHoldTime", v.jumpHoldTime);
  table.set("jumpTotalHoldTime", v.jumpTotalHoldTime);
  table.set("multiJump", v.multiJump);
  table.set("reJumpDelay", v.reJumpDelay);
  table.set("autoJump", v.autoJump);
  table.set("collisionCancelled", v.collisionCancelled);
  return table;
}

auto LuaConverter<ActorJumpProfile>::to(LuaEngine&, LuaValue const& v) -> std::optional<ActorJumpProfile> {
  if (v == LuaNil)
    return ActorJumpProfile();

  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};

  try {
    ActorJumpProfile ajp;
    ajp.jumpSpeed = table->get<std::optional<float>>("jumpSpeed");
    ajp.jumpControlForce = table->get<std::optional<float>>("jumpControlForce");
    ajp.jumpInitialPercentage = table->get<std::optional<float>>("jumpInitialPercentage");
    ajp.jumpHoldTime = table->get<std::optional<float>>("jumpHoldTime");
    ajp.jumpTotalHoldTime = table->get<std::optional<float>>("jumpTotalHoldTime");
    ajp.multiJump = table->get<std::optional<bool>>("multiJump");
    ajp.reJumpDelay = table->get<std::optional<float>>("reJumpDelay");
    ajp.autoJump = table->get<std::optional<bool>>("autoJump");
    ajp.collisionCancelled = table->get<std::optional<bool>>("collisionCancelled");
    return ajp;
  } catch (LuaConversionException const&) {
    return {};
  }
}

auto LuaConverter<ActorMovementParameters>::from(LuaEngine& engine, ActorMovementParameters const& v) -> LuaValue {
  auto table = engine.createTable();
  table.set("mass", v.mass);
  table.set("gravityMultiplier", v.gravityMultiplier);
  table.set("liquidBuoyancy", v.liquidBuoyancy);
  table.set("airBuoyancy", v.airBuoyancy);
  table.set("bounceFactor", v.bounceFactor);
  table.set("slopeSlidingFactor", v.slopeSlidingFactor);
  table.set("maxMovementPerStep", v.maxMovementPerStep);
  table.set("maximumCorrection", v.maximumCorrection);
  table.set("speedLimit", v.speedLimit);
  table.set("standingPoly", v.standingPoly);
  table.set("crouchingPoly", v.crouchingPoly);
  table.set("stickyCollision", v.stickyCollision);
  table.set("stickyForce", v.stickyForce);
  table.set("walkSpeed", v.walkSpeed);
  table.set("runSpeed", v.runSpeed);
  table.set("flySpeed", v.flySpeed);
  table.set("airFriction", v.airFriction);
  table.set("liquidFriction", v.liquidFriction);
  table.set("minimumLiquidPercentage", v.minimumLiquidPercentage);
  table.set("liquidImpedance", v.liquidImpedance);
  table.set("normalGroundFriction", v.normalGroundFriction);
  table.set("ambulatingGroundFriction", v.ambulatingGroundFriction);
  table.set("groundForce", v.groundForce);
  table.set("airForce", v.airForce);
  table.set("liquidForce", v.liquidForce);
  table.set("airJumpProfile", v.airJumpProfile);
  table.set("liquidJumpProfile", v.liquidJumpProfile);
  table.set("fallStatusSpeedMin", v.fallStatusSpeedMin);
  table.set("fallThroughSustainFrames", v.fallThroughSustainFrames);
  table.set("maximumPlatformCorrection", v.maximumPlatformCorrection);
  table.set("maximumPlatformCorrectionVelocityFactor", v.maximumPlatformCorrectionVelocityFactor);
  table.set("physicsEffectCategories", v.physicsEffectCategories);
  table.set("groundMovementMinimumSustain", v.groundMovementMinimumSustain);
  table.set("groundMovementMaximumSustain", v.groundMovementMaximumSustain);
  table.set("groundMovementCheckDistance", v.groundMovementCheckDistance);
  table.set("collisionEnabled", v.collisionEnabled);
  table.set("frictionEnabled", v.frictionEnabled);
  table.set("gravityEnabled", v.gravityEnabled);
  return table;
}

auto LuaConverter<ActorMovementParameters>::to(LuaEngine&, LuaValue const& v) -> std::optional<ActorMovementParameters> {
  if (v == LuaNil)
    return ActorMovementParameters();

  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};

  try {
    ActorMovementParameters amp;
    amp.mass = table->get<std::optional<float>>("mass");
    amp.gravityMultiplier = table->get<std::optional<float>>("gravityMultiplier");
    amp.liquidBuoyancy = table->get<std::optional<float>>("liquidBuoyancy");
    amp.airBuoyancy = table->get<std::optional<float>>("airBuoyancy");
    amp.bounceFactor = table->get<std::optional<float>>("bounceFactor");
    amp.slopeSlidingFactor = table->get<std::optional<float>>("slopeSlidingFactor");
    amp.maxMovementPerStep = table->get<std::optional<float>>("maxMovementPerStep");
    amp.maximumCorrection = table->get<std::optional<float>>("maximumCorrection");
    amp.speedLimit = table->get<std::optional<float>>("speedLimit");
    amp.standingPoly = table->get<std::optional<PolyF>>("standingPoly").or_else([&] -> std::optional<Star::Polygon<float>> { return table->get<std::optional<PolyF>>("collisionPoly"); });
    amp.crouchingPoly = table->get<std::optional<PolyF>>("crouchingPoly").or_else([&] -> std::optional<Star::Polygon<float>> { return table->get<std::optional<PolyF>>("collisionPoly"); });
    amp.stickyCollision = table->get<std::optional<bool>>("stickyCollision");
    amp.stickyForce = table->get<std::optional<float>>("stickyForce");
    amp.walkSpeed = table->get<std::optional<float>>("walkSpeed");
    amp.runSpeed = table->get<std::optional<float>>("runSpeed");
    amp.flySpeed = table->get<std::optional<float>>("flySpeed");
    amp.airFriction = table->get<std::optional<float>>("airFriction");
    amp.liquidFriction = table->get<std::optional<float>>("liquidFriction");
    amp.minimumLiquidPercentage = table->get<std::optional<float>>("minimumLiquidPercentage");
    amp.liquidImpedance = table->get<std::optional<float>>("liquidImpedance");
    amp.normalGroundFriction = table->get<std::optional<float>>("normalGroundFriction");
    amp.ambulatingGroundFriction = table->get<std::optional<float>>("ambulatingGroundFriction");
    amp.groundForce = table->get<std::optional<float>>("groundForce");
    amp.airForce = table->get<std::optional<float>>("airForce");
    amp.liquidForce = table->get<std::optional<float>>("liquidForce");
    amp.airJumpProfile = table->get<ActorJumpProfile>("airJumpProfile");
    amp.liquidJumpProfile = table->get<ActorJumpProfile>("liquidJumpProfile");
    amp.fallStatusSpeedMin = table->get<std::optional<float>>("fallStatusSpeedMin");
    amp.fallThroughSustainFrames = table->get<std::optional<int>>("fallThroughSustainFrames");
    amp.maximumPlatformCorrection = table->get<std::optional<float>>("maximumPlatformCorrection");
    amp.maximumPlatformCorrectionVelocityFactor = table->get<std::optional<float>>("maximumPlatformCorrectionVelocityFactor");
    amp.physicsEffectCategories = table->get<std::optional<StringSet>>("physicsEffectCategories");
    amp.groundMovementMinimumSustain = table->get<std::optional<float>>("groundMovementMinimumSustain");
    amp.groundMovementMaximumSustain = table->get<std::optional<float>>("groundMovementMaximumSustain");
    amp.groundMovementCheckDistance = table->get<std::optional<float>>("groundMovementCheckDistance");
    amp.collisionEnabled = table->get<std::optional<bool>>("collisionEnabled");
    amp.frictionEnabled = table->get<std::optional<bool>>("frictionEnabled");
    amp.gravityEnabled = table->get<std::optional<bool>>("gravityEnabled");
    return amp;
  } catch (LuaConversionException const&) {
    return {};
  }
}

auto LuaConverter<ActorMovementModifiers>::from(LuaEngine& engine, ActorMovementModifiers const& v) -> LuaValue {
  auto table = engine.createTable();
  table.set("groundMovementModifier", v.groundMovementModifier);
  table.set("liquidMovementModifier", v.liquidMovementModifier);
  table.set("speedModifier", v.speedModifier);
  table.set("airJumpModifier", v.airJumpModifier);
  table.set("liquidJumpModifier", v.liquidJumpModifier);
  table.set("runningSuppressed", v.runningSuppressed);
  table.set("jumpingSuppressed", v.jumpingSuppressed);
  table.set("facingSuppressed", v.facingSuppressed);
  table.set("movementSuppressed", v.movementSuppressed);
  return table;
}

auto LuaConverter<ActorMovementModifiers>::to(LuaEngine&, LuaValue const& v) -> std::optional<ActorMovementModifiers> {
  if (v == LuaNil)
    return ActorMovementModifiers();

  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};

  try {
    ActorMovementModifiers amm;
    amm.groundMovementModifier = table->get<std::optional<float>>("groundMovementModifier").value_or(1.0f);
    amm.liquidMovementModifier = table->get<std::optional<float>>("liquidMovementModifier").value_or(1.0f);
    amm.speedModifier = table->get<std::optional<float>>("speedModifier").value_or(1.0f);
    amm.airJumpModifier = table->get<std::optional<float>>("airJumpModifier").value_or(1.0f);
    amm.liquidJumpModifier = table->get<std::optional<float>>("liquidJumpModifier").value_or(1.0f);
    amm.runningSuppressed = table->get<std::optional<bool>>("runningSuppressed").value_or(false);
    amm.jumpingSuppressed = table->get<std::optional<bool>>("jumpingSuppressed").value_or(false);
    amm.facingSuppressed = table->get<std::optional<bool>>("facingSuppressed").value_or(false);
    amm.movementSuppressed = table->get<std::optional<bool>>("movementSuppressed").value_or(false);
    return amm;
  } catch (LuaConversionException const&) {
    return {};
  }
}

auto LuaConverter<StatModifier>::from(LuaEngine& engine, StatModifier const& v) -> LuaValue {
  return engine.luaFrom(jsonFromStatModifier(v));
}

auto LuaConverter<StatModifier>::to(LuaEngine& engine, LuaValue v) -> std::optional<StatModifier> {
  auto json = engine.luaMaybeTo<Json>(std::move(v));
  if (!json)
    return std::nullopt;

  try {
    return jsonToStatModifier(std::move(*json));
  } catch (JsonException const&) {
    return std::nullopt;
  }
}

auto LuaConverter<EphemeralStatusEffect>::from(LuaEngine& engine, EphemeralStatusEffect const& v) -> LuaValue {
  auto table = engine.createTable();
  table.set("effect", v.uniqueEffect);
  table.set("duration", v.duration);
  return table;
}

auto LuaConverter<EphemeralStatusEffect>::to(LuaEngine& engine, LuaValue const& v) -> std::optional<EphemeralStatusEffect> {
  if (auto s = v.ptr<LuaString>()) {
    return EphemeralStatusEffect{.uniqueEffect = UniqueStatusEffect(s->ptr()), .duration = {}};
  } else if (auto table = v.ptr<LuaTable>()) {
    auto effect = engine.luaMaybeTo<String>(table->get("effect"));
    auto duration = engine.luaMaybeTo<std::optional<float>>(table->get("duratino"));
    if (effect && duration)
      return EphemeralStatusEffect{.uniqueEffect = std::move(*effect), .duration = std::move(*duration)};
  }

  return std::nullopt;
}

auto LuaConverter<DamageRequest>::from(LuaEngine& engine, DamageRequest const& v) -> LuaValue {
  auto table = engine.createTable();
  table.set("hitType", HitTypeNames.getRight(v.hitType));
  table.set("damageType", DamageTypeNames.getRight(v.damageType));
  table.set("damage", v.damage);
  table.set("knockbackMomentum", v.knockbackMomentum);
  table.set("sourceEntityId", v.sourceEntityId);
  table.set("damageSourceKind", v.damageSourceKind);
  table.set("statusEffects", v.statusEffects);
  return table;
}

auto LuaConverter<DamageRequest>::to(LuaEngine&, LuaValue const& v) -> std::optional<DamageRequest> {
  auto table = v.ptr<LuaTable>();
  if (!table)
    return std::nullopt;

  try {
    DamageRequest dr;
    if (auto hitType = table->get<std::optional<String>>("hitType"))
      dr.hitType = HitTypeNames.getLeft(*hitType);
    if (auto damageType = table->get<std::optional<String>>("damageType"))
      dr.damageType = DamageTypeNames.getLeft(*damageType);
    dr.damage = table->get<float>("damage");
    if (auto knockbackMomentum = table->get<std::optional<Vec2F>>("knockbackMomentum"))
      dr.knockbackMomentum = *knockbackMomentum;
    if (auto sourceEntityId = table->get<std::optional<EntityId>>("sourceEntityId"))
      dr.sourceEntityId = *sourceEntityId;
    if (auto damageSourceKind = table->get<std::optional<String>>("damageSourceKind"))
      dr.damageSourceKind = std::move(*damageSourceKind);
    if (auto statusEffects = table->get<std::optional<List<EphemeralStatusEffect>>>("statusEffects"))
      dr.statusEffects = std::move(*statusEffects);
    return dr;
  } catch (LuaConversionException const&) {
    return std::nullopt;
  } catch (MapException const&) {
    return std::nullopt;
  }
}

auto LuaConverter<DamageNotification>::from(LuaEngine& engine, DamageNotification const& v) -> LuaValue {
  auto table = engine.createTable();
  table.set("sourceEntityId", v.sourceEntityId);
  table.set("targetEntityId", v.targetEntityId);
  table.set("position", v.position);
  table.set("damageDealt", v.damageDealt);
  table.set("healthLost", v.healthLost);
  table.set("hitType", HitTypeNames.getRight(v.hitType));
  table.set("damageSourceKind", v.damageSourceKind);
  table.set("targetMaterialKind", v.targetMaterialKind);
  return table;
}

auto LuaConverter<DamageNotification>::to(LuaEngine&, LuaValue const& v) -> std::optional<DamageNotification> {
  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};
  try {
    DamageNotification dn;
    dn.sourceEntityId = table->get<EntityId>("sourceEntityId");
    dn.targetEntityId = table->get<EntityId>("targetEntityId");
    dn.position = table->get<Vec2F>("position");
    dn.damageDealt = table->get<float>("damageDealt");
    dn.healthLost = table->get<float>("healthLost");
    dn.hitType = HitTypeNames.getLeft(table->get<String>("hitType"));
    dn.damageSourceKind = table->get<String>("damageSourceKind");
    dn.targetMaterialKind = table->get<String>("targetMaterialKind");
    return dn;
  } catch (LuaConversionException const&) {
    return {};
  } catch (MapException const&) {
    return {};
  }
}

auto LuaConverter<LiquidLevel>::from(LuaEngine& engine, LiquidLevel const& v) -> LuaValue {
  auto table = engine.createTable();
  table.set(1, v.liquid);
  table.set(2, v.level);
  return table;
}

auto LuaConverter<LiquidLevel>::to(LuaEngine& engine, LuaValue const& v) -> std::optional<LiquidLevel> {
  if (auto table = v.ptr<LuaTable>()) {
    auto liquid = engine.luaMaybeTo<LiquidId>(table->get(1));
    auto level = engine.luaMaybeTo<std::uint8_t>(table->get(2));
    if (liquid && level)
      return LiquidLevel(std::move(*liquid), std::move(*level));
  }
  return {};
}

auto LuaConverter<Drawable>::from(LuaEngine& engine, Drawable const& v) -> LuaValue {
  auto table = engine.createTable();
  if (auto line = v.part.ptr<Drawable::LinePart>()) {
    table.set("line", line->line);
    table.set("width", line->width);
  } else if (auto poly = v.part.ptr<Drawable::PolyPart>()) {
    table.set("poly", poly->poly);
  } else if (auto image = v.part.ptr<Drawable::ImagePart>()) {
    table.set("image", AssetPath::join(image->image));
    table.set("transformation", image->transformation);
  }

  table.set("position", v.position);
  table.set("color", v.color);
  table.set("fullbright", v.fullbright);

  return table;
}

auto LuaConverter<Drawable>::to(LuaEngine&, LuaValue const& v) -> std::optional<Drawable> {
  if (auto table = v.ptr<LuaTable>()) {
    std::optional<Drawable> result;
    result.emplace();
    Drawable& drawable = result.value();

    Color color = table->get<std::optional<Color>>("color").value_or(Color::White);

    if (auto line = table->get<std::optional<Line2F>>("line"))
      drawable = Drawable::makeLine(std::move(*line), table->get<float>("width"), color);
    else if (auto poly = table->get<std::optional<PolyF>>("poly"))
      drawable = Drawable::makePoly(std::move(*poly), color);
    else if (auto image = table->get<std::optional<String>>("image"))
      drawable = Drawable::makeImage(std::move(*image), 1.0f, table->get<std::optional<bool>>("centered").value_or(true), Vec2F(), color);
    else
      return std::nullopt;// throw LuaAnimationComponentException("Drawable table must have 'line', 'poly', or 'image'");

    if (auto transformation = table->get<std::optional<Mat3F>>("transformation"))
      drawable.transform(*transformation);
    if (auto rotation = table->get<std::optional<float>>("rotation"))
      drawable.rotate(*rotation);
    if (table->get<bool>("mirrored"))
      drawable.scale(Vec2F(-1, 1));
    if (auto scale = table->get<std::optional<float>>("scale"))
      drawable.scale(*scale);
    if (auto position = table->get<std::optional<Vec2F>>("position"))
      drawable.translate(*position);

    drawable.fullbright = table->get<bool>("fullbright");

    return result;
  }
  return {};
}

auto LuaConverter<Collection>::from(LuaEngine& engine, Collection const& c) -> LuaValue {
  auto table = engine.createTable();
  table.set("name", c.name);
  table.set("type", CollectionTypeNames.getRight(c.type));
  table.set("title", c.title);
  return table;
}

auto LuaConverter<Collection>::to(LuaEngine& engine, LuaValue const& v) -> std::optional<Collection> {
  if (auto table = v.ptr<LuaTable>()) {
    auto name = engine.luaMaybeTo<String>(table->get("name"));
    auto type = engine.luaMaybeTo<String>(table->get("type"));
    auto title = engine.luaMaybeTo<String>(table->get("title"));
    if (name && type && CollectionTypeNames.hasRightValue(*type) && title)
      return Collection(*name, CollectionTypeNames.getLeft(*type), *title);
  }

  return {};
}

auto LuaConverter<Collectable>::from(LuaEngine& engine, Collectable const& c) -> LuaValue {
  auto table = engine.createTable();
  table.set("name", c.name);
  table.set("order", c.order);
  table.set("title", c.title);
  table.set("description", c.description);
  table.set("icon", c.icon);
  return table;
}

auto LuaConverter<Collectable>::to(LuaEngine& engine, LuaValue const& v) -> std::optional<Collectable> {
  if (auto table = v.ptr<LuaTable>()) {
    auto name = engine.luaMaybeTo<String>(table->get("name"));
    if (name) {
      return Collectable(*name,
                         engine.luaMaybeTo<int>(table->get("order")).value_or(0),
                         engine.luaMaybeTo<String>(table->get("title")).value_or(""),
                         engine.luaMaybeTo<String>(table->get("description")).value_or(""),
                         engine.luaMaybeTo<String>(table->get("icon")).value_or(""));
    }
  }

  return {};
}

auto LuaConverter<PhysicsMovingCollision>::from(LuaEngine& engine, PhysicsMovingCollision const& v) -> LuaValue {
  auto table = engine.createTable();
  table.set("position", v.position);
  table.set("collision", v.collision);
  table.set("collisionKind", v.collisionKind);
  auto categoryTable = engine.createTable();
  table.set("categoryFilter", categoryTable);
  // see jsonToPhysicsCategoryFilter
  categoryTable.set(
    v.categoryFilter.type == PhysicsCategoryFilter::Type::Whitelist ? "categoryWhitelist" : "categoryBlacklist",
    v.categoryFilter.categories);
  return table;
}

auto LuaUserDataMethods<WeakPtr<BehaviorState>>::make() -> LuaMethods<WeakPtr<BehaviorState>> {
  LuaMethods<WeakPtr<BehaviorState>> methods;
  methods.registerMethodWithSignature<NodeStatus, WeakPtr<BehaviorState>, float>(
    "run", [](WeakPtr<BehaviorState> const& behavior, float dt) -> NodeStatus {
      if (behavior.expired())
        throw StarException("Use of expired blackboard");

      return behavior.lock()->run(dt);
    });
  methods.registerMethodWithSignature<void, WeakPtr<BehaviorState>>(
    "clear", [](WeakPtr<BehaviorState> const& behavior) -> void {
      if (behavior.expired())
        throw StarException("Use of expired blackboard");

      behavior.lock()->clear();
    });
  methods.registerMethodWithSignature<WeakPtr<Blackboard>, WeakPtr<BehaviorState>>(
    "blackboard", [](WeakPtr<BehaviorState> const& behavior) -> WeakPtr<Blackboard> {
      if (behavior.expired())
        throw StarException("Use of expired blackboard");

      return behavior.lock()->blackboardPtr();
    });
  return methods;
}

auto LuaConverter<NodeStatus>::from(LuaEngine&, NodeStatus const& status) -> LuaValue {
  if (status == NodeStatus::Success)
    return true;
  else if (status == NodeStatus::Failure)
    return false;
  else
    return {};
}

auto LuaConverter<NodeStatus>::to(LuaEngine&, LuaValue const& v) -> NodeStatus {
  if (v.is<LuaBoolean>())
    return v.get<LuaBoolean>() == true ? NodeStatus::Success : NodeStatus::Failure;
  else
    return NodeStatus::Running;
}

auto LuaUserDataMethods<WeakPtr<Blackboard>>::make() -> LuaMethods<WeakPtr<Blackboard>> {
  LuaMethods<WeakPtr<Blackboard>> methods;

  auto get = [](WeakPtr<Blackboard> const& board, NodeParameterType const& type, String const& key) -> LuaValue {
    if (board.expired())
      throw StarException("Use of expired blackboard");

    return board.lock()->get(type, key);
  };
  auto set = [](WeakPtr<Blackboard> const& board, NodeParameterType const& type, String const& key, LuaValue const& value) -> void {
    if (board.expired())
      throw StarException("Use of expired blackboard");

    board.lock()->set(type, key, value);
  };

  methods.registerMethodWithSignature<LuaValue, WeakPtr<Blackboard>, String, String>("get",
                                                                                     [&](WeakPtr<Blackboard> const& board, String const& type, String const& key) -> LuaValue {
                                                                                       return get(board, NodeParameterTypeNames.getLeft(type), key);
                                                                                     });
  methods.registerMethodWithSignature<void, WeakPtr<Blackboard>, String, String, LuaValue>("set",
                                                                                           [&](WeakPtr<Blackboard> const& board, String const& type, String const& key, LuaValue const& value) -> void {
                                                                                             set(board, NodeParameterTypeNames.getLeft(type), key, value);
                                                                                           });

  methods.registerMethodWithSignature<LuaValue, WeakPtr<Blackboard>, String>(
    "getEntity", [&](WeakPtr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Entity, key);
    });
  methods.registerMethodWithSignature<LuaValue, WeakPtr<Blackboard>, String>(
    "getPosition", [&](WeakPtr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Position, key);
    });
  methods.registerMethodWithSignature<LuaValue, WeakPtr<Blackboard>, String>(
    "getVec2", [&](WeakPtr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Vec2, key);
    });
  methods.registerMethodWithSignature<LuaValue, WeakPtr<Blackboard>, String>(
    "getNumber", [&](WeakPtr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Number, key);
    });
  methods.registerMethodWithSignature<LuaValue, WeakPtr<Blackboard>, String>(
    "getBool", [&](WeakPtr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Bool, key);
    });
  methods.registerMethodWithSignature<LuaValue, WeakPtr<Blackboard>, String>(
    "getList", [&](WeakPtr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::List, key);
    });
  methods.registerMethodWithSignature<LuaValue, WeakPtr<Blackboard>, String>(
    "getTable", [&](WeakPtr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Table, key);
    });
  methods.registerMethodWithSignature<LuaValue, WeakPtr<Blackboard>, String>(
    "getString", [&](WeakPtr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::String, key);
    });

  methods.registerMethodWithSignature<void, WeakPtr<Blackboard>, String, LuaValue>(
    "setEntity", [&](WeakPtr<Blackboard> const& board, String const& key, LuaValue const& value) -> void {
      set(board, NodeParameterType::Entity, key, value);
    });
  methods.registerMethodWithSignature<void, WeakPtr<Blackboard>, String, LuaValue>(
    "setPosition", [&](WeakPtr<Blackboard> const& board, String const& key, LuaValue const& value) -> void {
      set(board, NodeParameterType::Position, key, value);
    });
  methods.registerMethodWithSignature<void, WeakPtr<Blackboard>, String, LuaValue>(
    "setVec2", [&](WeakPtr<Blackboard> const& board, String const& key, LuaValue const& value) -> void {
      set(board, NodeParameterType::Vec2, key, value);
    });
  methods.registerMethodWithSignature<void, WeakPtr<Blackboard>, String, LuaValue>(
    "setNumber", [&](WeakPtr<Blackboard> const& board, String const& key, LuaValue const& value) -> void {
      set(board, NodeParameterType::Number, key, value);
    });
  methods.registerMethodWithSignature<void, WeakPtr<Blackboard>, String, LuaValue>(
    "setBool", [&](WeakPtr<Blackboard> const& board, String const& key, LuaValue const& value) -> void {
      set(board, NodeParameterType::Bool, key, value);
    });
  methods.registerMethodWithSignature<void, WeakPtr<Blackboard>, String, LuaValue>(
    "setList", [&](WeakPtr<Blackboard> const& board, String const& key, LuaValue const& value) -> void {
      set(board, NodeParameterType::List, key, value);
    });
  methods.registerMethodWithSignature<void, WeakPtr<Blackboard>, String, LuaValue>(
    "setTable", [&](WeakPtr<Blackboard> const& board, String const& key, LuaValue const& value) -> void {
      set(board, NodeParameterType::Table, key, value);
    });
  methods.registerMethodWithSignature<void, WeakPtr<Blackboard>, String, LuaValue>(
    "setString", [&](WeakPtr<Blackboard> const& board, String const& key, LuaValue const& value) -> void {
      set(board, NodeParameterType::String, key, value);
    });
  return methods;
}

}// namespace Star
