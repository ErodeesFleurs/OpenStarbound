import std;
import star.string;
import star.list;
import star.map;
import star.json;
import star.lua;
import star.maybe;
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

import star.behavior_database;
import star.behavior_state;
import star.platformer_astar;

import star.vector;
import star.matrix3;
import star.line;
import star.rect;
import star.poly;
namespace Star {

LuaValue LuaConverter<InventorySlot>::from(LuaEngine& engine, InventorySlot k) {
  if (auto equipment = k.ptr<EquipmentSlot>())
    return engine.createString(EquipmentSlotNames.getRight(*equipment));
  else if (auto bag = k.ptr<BagSlot>()) {
    auto table = engine.createTable(2, 0);
    table.set(1, bag->first);
    table.set(2, bag->second);
    return table;
  }
  else if (k.is<SwapSlot>())
    return engine.createString("swap");
  else if (k.is<TrashSlot>())
    return engine.createString("trash");
  else return {}; // avoid UB if every accounted-for case fails
}

Maybe<InventorySlot> LuaConverter<InventorySlot>::to(LuaEngine&, LuaValue const& v) {
  if (auto str = v.ptr<LuaString>()) {
    auto string = str->toString();
    if (string.equalsIgnoreCase("swap"))
      return {SwapSlot()};
    else if (string.equalsIgnoreCase("trash"))
      return {TrashSlot()};
    else if (auto equipment = EquipmentSlotNames.leftPtr(str->toString()))
      return {*equipment};
    else
      return {};
  }
  else if (auto table = v.ptr<LuaTable>())
    return {BagSlot(table->get<LuaString>(1).toString(), (std::uint8_t)table->get<unsigned>(2))};
  else
    return {};
}

LuaValue LuaConverter<CollisionKind>::from(LuaEngine& engine, CollisionKind k) {
  return engine.createString(CollisionKindNames.getRight(k));
}

Maybe<CollisionKind> LuaConverter<CollisionKind>::to(LuaEngine&, LuaValue const& v) {
  if (auto str = v.ptr<LuaString>())
    return CollisionKindNames.maybeLeft(str->ptr());
  return {};
}

LuaValue LuaConverter<CollisionSet>::from(LuaEngine& engine, CollisionSet const& s) {
  auto collisionTable = engine.createTable();
  int i = 1;
  for (auto const& v : CollisionKindNames) {
    if (s.contains(v.first)) {
      collisionTable.set(i++, v.second);
    }
  }
  return collisionTable;
}

Maybe<CollisionSet> LuaConverter<CollisionSet>::to(LuaEngine& engine, LuaValue const& v) {
  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};

  CollisionSet result;
  bool failed = false;
  table->iterate([&result, &failed, &engine](LuaValue, LuaValue value) {
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

LuaValue LuaConverter<PlatformerAStar::Path>::from(LuaEngine& engine, PlatformerAStar::Path const& path) {
  auto convertNode = [&engine](PlatformerAStar::Node const& node) {
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

LuaMethods<PlatformerAStar::PathFinder> LuaUserDataMethods<PlatformerAStar::PathFinder>::make() {
  LuaMethods<PlatformerAStar::PathFinder> methods;
  methods.registerMethodWithSignature<Maybe<bool>, PlatformerAStar::PathFinder&, Maybe<unsigned>>(
      "explore", std::mem_fn(&PlatformerAStar::PathFinder::explore));
  methods.registerMethodWithSignature<Maybe<PlatformerAStar::Path>, PlatformerAStar::PathFinder&>(
      "result", std::mem_fn(&PlatformerAStar::PathFinder::result));
  return methods;
}

Maybe<PlatformerAStar::Parameters> LuaConverter<PlatformerAStar::Parameters>::to(LuaEngine&, LuaValue const& v) {
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
    p.maxDistance = table->get<Maybe<float>>("maxDistance");
    p.returnBest = table->get<Maybe<bool>>("returnBest").value(false);
    p.mustEndOnGround = table->get<Maybe<bool>>("mustEndOnGround").value(false);
    p.enableWalkSpeedJumps = table->get<Maybe<bool>>("enableWalkSpeedJumps").value(false);
    p.enableVerticalJumpAirControl = table->get<Maybe<bool>>("enableVerticalJumpAirControl").value(false);
    p.swimCost = table->get<Maybe<float>>("swimCost");
    p.jumpCost = table->get<Maybe<float>>("jumpCost");
    p.liquidJumpCost = table->get<Maybe<float>>("liquidJumpCost");
    p.dropCost = table->get<Maybe<float>>("dropCost");
    p.boundBox = table->get<RectF>("boundBox");
    p.standingBoundBox = table->get<RectF>("standingBoundBox");
    p.droppingBoundBox = table->get<RectF>("droppingBoundBox");
    p.smallJumpMultiplier = table->get<Maybe<float>>("smallJumpMultiplier");
    p.jumpDropXMultiplier = table->get<Maybe<float>>("jumpDropXMultiplier");
    p.maxFScore = table->get<double>("maxFScore");
    p.maxNodesToSearch = table->get<unsigned>("maxNodesToSearch");
    p.maxLandingVelocity = table->get<Maybe<float>>("maxLandingVelocity");
  } catch (LuaConversionException const&) {
    return {};
  }

  return p;
}

LuaValue LuaConverter<ActorJumpProfile>::from(LuaEngine& engine, ActorJumpProfile const& v) {
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

Maybe<ActorJumpProfile> LuaConverter<ActorJumpProfile>::to(LuaEngine&, LuaValue const& v) {
  if (v == LuaNil)
    return ActorJumpProfile();

  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};

  try {
    ActorJumpProfile ajp;
    ajp.jumpSpeed = table->get<Maybe<float>>("jumpSpeed");
    ajp.jumpControlForce = table->get<Maybe<float>>("jumpControlForce");
    ajp.jumpInitialPercentage = table->get<Maybe<float>>("jumpInitialPercentage");
    ajp.jumpHoldTime = table->get<Maybe<float>>("jumpHoldTime");
    ajp.jumpTotalHoldTime = table->get<Maybe<float>>("jumpTotalHoldTime");
    ajp.multiJump = table->get<Maybe<bool>>("multiJump");
    ajp.reJumpDelay = table->get<Maybe<float>>("reJumpDelay");
    ajp.autoJump = table->get<Maybe<bool>>("autoJump");
    ajp.collisionCancelled = table->get<Maybe<bool>>("collisionCancelled");
    return ajp;
  } catch (LuaConversionException const&) {
    return {};
  }
}

LuaValue LuaConverter<ActorMovementParameters>::from(LuaEngine& engine, ActorMovementParameters const& v) {
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

Maybe<ActorMovementParameters> LuaConverter<ActorMovementParameters>::to(LuaEngine&, LuaValue const& v) {
  if (v == LuaNil)
    return ActorMovementParameters();

  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};

  try {
    ActorMovementParameters amp;
    amp.mass = table->get<Maybe<float>>("mass");
    amp.gravityMultiplier = table->get<Maybe<float>>("gravityMultiplier");
    amp.liquidBuoyancy = table->get<Maybe<float>>("liquidBuoyancy");
    amp.airBuoyancy = table->get<Maybe<float>>("airBuoyancy");
    amp.bounceFactor = table->get<Maybe<float>>("bounceFactor");
    amp.slopeSlidingFactor = table->get<Maybe<float>>("slopeSlidingFactor");
    amp.maxMovementPerStep = table->get<Maybe<float>>("maxMovementPerStep");
    amp.maximumCorrection = table->get<Maybe<float>>("maximumCorrection");
    amp.speedLimit = table->get<Maybe<float>>("speedLimit");
    amp.standingPoly = table->get<Maybe<PolyF>>("standingPoly").orMaybe(table->get<Maybe<PolyF>>("collisionPoly"));
    amp.crouchingPoly = table->get<Maybe<PolyF>>("crouchingPoly").orMaybe(table->get<Maybe<PolyF>>("collisionPoly"));
    amp.stickyCollision = table->get<Maybe<bool>>("stickyCollision");
    amp.stickyForce = table->get<Maybe<float>>("stickyForce");
    amp.walkSpeed = table->get<Maybe<float>>("walkSpeed");
    amp.runSpeed = table->get<Maybe<float>>("runSpeed");
    amp.flySpeed = table->get<Maybe<float>>("flySpeed");
    amp.airFriction = table->get<Maybe<float>>("airFriction");
    amp.liquidFriction = table->get<Maybe<float>>("liquidFriction");
    amp.minimumLiquidPercentage = table->get<Maybe<float>>("minimumLiquidPercentage");
    amp.liquidImpedance = table->get<Maybe<float>>("liquidImpedance");
    amp.normalGroundFriction = table->get<Maybe<float>>("normalGroundFriction");
    amp.ambulatingGroundFriction = table->get<Maybe<float>>("ambulatingGroundFriction");
    amp.groundForce = table->get<Maybe<float>>("groundForce");
    amp.airForce = table->get<Maybe<float>>("airForce");
    amp.liquidForce = table->get<Maybe<float>>("liquidForce");
    amp.airJumpProfile = table->get<ActorJumpProfile>("airJumpProfile");
    amp.liquidJumpProfile = table->get<ActorJumpProfile>("liquidJumpProfile");
    amp.fallStatusSpeedMin = table->get<Maybe<float>>("fallStatusSpeedMin");
    amp.fallThroughSustainFrames = table->get<Maybe<int>>("fallThroughSustainFrames");
    amp.maximumPlatformCorrection = table->get<Maybe<float>>("maximumPlatformCorrection");
    amp.maximumPlatformCorrectionVelocityFactor = table->get<Maybe<float>>("maximumPlatformCorrectionVelocityFactor");
    amp.physicsEffectCategories = table->get<Maybe<StringSet>>("physicsEffectCategories");
    amp.groundMovementMinimumSustain = table->get<Maybe<float>>("groundMovementMinimumSustain");
    amp.groundMovementMaximumSustain = table->get<Maybe<float>>("groundMovementMaximumSustain");
    amp.groundMovementCheckDistance = table->get<Maybe<float>>("groundMovementCheckDistance");
    amp.collisionEnabled = table->get<Maybe<bool>>("collisionEnabled");
    amp.frictionEnabled = table->get<Maybe<bool>>("frictionEnabled");
    amp.gravityEnabled = table->get<Maybe<bool>>("gravityEnabled");
    return amp;
  } catch (LuaConversionException const&) {
    return {};
  }
}

LuaValue LuaConverter<ActorMovementModifiers>::from(LuaEngine& engine, ActorMovementModifiers const& v) {
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

Maybe<ActorMovementModifiers> LuaConverter<ActorMovementModifiers>::to(LuaEngine&, LuaValue const& v) {
  if (v == LuaNil)
    return ActorMovementModifiers();

  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};

  try {
    ActorMovementModifiers amm;
    amm.groundMovementModifier = table->get<Maybe<float>>("groundMovementModifier").value(1.0f);
    amm.liquidMovementModifier = table->get<Maybe<float>>("liquidMovementModifier").value(1.0f);
    amm.speedModifier = table->get<Maybe<float>>("speedModifier").value(1.0f);
    amm.airJumpModifier = table->get<Maybe<float>>("airJumpModifier").value(1.0f);
    amm.liquidJumpModifier = table->get<Maybe<float>>("liquidJumpModifier").value(1.0f);
    amm.runningSuppressed = table->get<Maybe<bool>>("runningSuppressed").value(false);
    amm.jumpingSuppressed = table->get<Maybe<bool>>("jumpingSuppressed").value(false);
    amm.facingSuppressed = table->get<Maybe<bool>>("facingSuppressed").value(false);
    amm.movementSuppressed = table->get<Maybe<bool>>("movementSuppressed").value(false);
    return amm;
  } catch (LuaConversionException const&) {
    return {};
  }
}

LuaValue LuaConverter<StatModifier>::from(LuaEngine& engine, StatModifier const& v) {
  return engine.luaFrom(jsonFromStatModifier(v));
}

Maybe<StatModifier> LuaConverter<StatModifier>::to(LuaEngine& engine, LuaValue v) {
  auto json = engine.luaMaybeTo<Json>(std::move(v));
  if (!json)
    return {};

  try {
    return jsonToStatModifier(json.take());
  } catch (JsonException const&) {
    return {};
  }
}

LuaValue LuaConverter<EphemeralStatusEffect>::from(LuaEngine& engine, EphemeralStatusEffect const& v) {
  auto table = engine.createTable();
  table.set("effect", v.uniqueEffect);
  table.set("duration", v.duration);
  return table;
}

Maybe<EphemeralStatusEffect> LuaConverter<EphemeralStatusEffect>::to(LuaEngine& engine, LuaValue const& v) {
  if (auto s = v.ptr<LuaString>()) {
    return EphemeralStatusEffect{UniqueStatusEffect(s->ptr()), {}};
  } else if (auto table = v.ptr<LuaTable>()) {
    auto effect = engine.luaMaybeTo<String>(table->get("effect"));
    auto duration = engine.luaMaybeTo<Maybe<float>>(table->get("duration"));
    if (!duration)
      duration = engine.luaMaybeTo<Maybe<float>>(table->get("duratino"));
    if (effect && duration)
      return EphemeralStatusEffect{effect.take(), duration.take()};
  }

  return {};
}

LuaValue LuaConverter<DamageRequest>::from(LuaEngine& engine, DamageRequest const& v) {
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

Maybe<DamageRequest> LuaConverter<DamageRequest>::to(LuaEngine&, LuaValue const& v) {
  auto table = v.ptr<LuaTable>();
  if (!table)
    return {};

  try {
    DamageRequest dr;
    if (auto hitType = table->get<Maybe<String>>("hitType"))
      dr.hitType = HitTypeNames.getLeft(*hitType);
    if (auto damageType = table->get<Maybe<String>>("damageType"))
      dr.damageType = DamageTypeNames.getLeft(*damageType);
    dr.damage = table->get<float>("damage");
    if (auto knockbackMomentum = table->get<Maybe<Vec2F>>("knockbackMomentum"))
      dr.knockbackMomentum = *knockbackMomentum;
    if (auto sourceEntityId = table->get<Maybe<EntityId>>("sourceEntityId"))
      dr.sourceEntityId = *sourceEntityId;
    if (auto damageSourceKind = table->get<Maybe<String>>("damageSourceKind"))
      dr.damageSourceKind = damageSourceKind.take();
    if (auto statusEffects = table->get<Maybe<List<EphemeralStatusEffect>>>("statusEffects"))
      dr.statusEffects = statusEffects.take();
    return dr;
  } catch (LuaConversionException const&) {
    return {};
  } catch (MapException const&) {
    return {};
  }
}

LuaValue LuaConverter<DamageNotification>::from(LuaEngine& engine, DamageNotification const& v) {
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

Maybe<DamageNotification> LuaConverter<DamageNotification>::to(LuaEngine&, LuaValue const& v) {
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

LuaValue LuaConverter<LiquidLevel>::from(LuaEngine& engine, LiquidLevel const& v) {
  auto table = engine.createTable();
  table.set(1, v.liquid);
  table.set(2, v.level);
  return table;
}

Maybe<LiquidLevel> LuaConverter<LiquidLevel>::to(LuaEngine& engine, LuaValue const& v) {
  if (auto table = v.ptr<LuaTable>()) {
    auto liquid = engine.luaMaybeTo<LiquidId>(table->get(1));
    auto level = engine.luaMaybeTo<std::uint8_t>(table->get(2));
    if (liquid && level)
      return LiquidLevel(liquid.take(), level.take());
  }
  return {};
}

LuaValue LuaConverter<Drawable>::from(LuaEngine& engine, Drawable const& v) {
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

Maybe<Drawable> LuaConverter<Drawable>::to(LuaEngine&, LuaValue const& v) {
  if (auto table = v.ptr<LuaTable>()) {
    Maybe<Drawable> result;
    result.emplace();
    Drawable& drawable = result.get();

    Color color = table->get<Maybe<Color>>("color").value(Color::White);

    if (auto line = table->get<Maybe<Line2F>>("line"))
      drawable = Drawable::makeLine(line.take(), table->get<float>("width"), color);
    else if (auto poly = table->get<Maybe<PolyF>>("poly"))
      drawable = Drawable::makePoly(poly.take(), color);
    else if (auto image = table->get<Maybe<String>>("image"))
      drawable = Drawable::makeImage(image.take(), 1.0f, table->get<Maybe<bool>>("centered").value(true), Vec2F(), color);
    else
      return {}; // throw LuaAnimationComponentException("Drawable table must have 'line', 'poly', or 'image'");

    if (auto transformation = table->get<Maybe<Mat3F>>("transformation"))
      drawable.transform(*transformation);
    if (auto rotation = table->get<Maybe<float>>("rotation"))
      drawable.rotate(*rotation);
    if (table->get<bool>("mirrored"))
      drawable.scale(Vec2F(-1, 1));
    if (auto scale = table->get<Maybe<float>>("scale"))
      drawable.scale(*scale);
    if (auto position = table->get<Maybe<Vec2F>>("position"))
      drawable.translate(*position);

    drawable.fullbright = table->get<bool>("fullbright");

    return result;
  }
  return {};
}

LuaValue LuaConverter<Collection>::from(LuaEngine& engine, Collection const& c) {
  auto table = engine.createTable();
  table.set("name", c.name);
  table.set("type", CollectionTypeNames.getRight(c.type));
  table.set("title", c.title);
  return table;
}

Maybe<Collection> LuaConverter<Collection>::to(LuaEngine& engine, LuaValue const& v) {
  if (auto table = v.ptr<LuaTable>()) {
    auto name = engine.luaMaybeTo<String>(table->get("name"));
    auto type = engine.luaMaybeTo<String>(table->get("type"));
    auto title = engine.luaMaybeTo<String>(table->get("title"));
    if (name && type && CollectionTypeNames.hasRightValue(*type) && title)
      return Collection(*name, CollectionTypeNames.getLeft(*type), *title);
  }

  return {};
}

LuaValue LuaConverter<Collectable>::from(LuaEngine& engine, Collectable const& c) {
  auto table = engine.createTable();
  table.set("name", c.name);
  table.set("order", c.order);
  table.set("title", c.title);
  table.set("description", c.description);
  table.set("icon", c.icon);
  return table;
}

Maybe<Collectable> LuaConverter<Collectable>::to(LuaEngine& engine, LuaValue const& v) {
  if (auto table = v.ptr<LuaTable>()) {
    auto name = engine.luaMaybeTo<String>(table->get("name"));
    if (name) {
      return Collectable(*name,
        engine.luaMaybeTo<int>(table->get("order")).value(0),
        engine.luaMaybeTo<String>(table->get("title")).value(""),
        engine.luaMaybeTo<String>(table->get("description")).value(""),
        engine.luaMaybeTo<String>(table->get("icon")).value(""));
    }
  }

  return {};
}

LuaValue LuaConverter<PhysicsMovingCollision>::from(LuaEngine& engine, PhysicsMovingCollision const& v) {
  auto table = engine.createTable();
  table.set("position", v.position);
  table.set("collision", v.collision);
  table.set("collisionKind", v.collisionKind);
  auto categoryTable = engine.createTable();
  table.set("categoryFilter", categoryTable);
  // see jsonToPhysicsCategoryFilter
  categoryTable.set(
    v.categoryFilter.type == PhysicsCategoryFilter::Type::Whitelist ? "categoryWhitelist" : "categoryBlacklist",
    v.categoryFilter.categories
  );
  return table;
}

LuaMethods<std::weak_ptr<BehaviorState>> LuaUserDataMethods<std::weak_ptr<BehaviorState>>::make() {
  LuaMethods<std::weak_ptr<BehaviorState>> methods;
  methods.registerMethodWithSignature<NodeStatus, std::weak_ptr<BehaviorState>, float>(
    "run", [](std::weak_ptr<BehaviorState> const& behavior, float dt) -> NodeStatus {
      if (behavior.expired())
        throw StarException("Use of expired blackboard");

      return behavior.lock()->run(dt);
    });
  methods.registerMethodWithSignature<void, std::weak_ptr<BehaviorState>>(
    "clear", [](std::weak_ptr<BehaviorState> const& behavior) {
      if (behavior.expired())
        throw StarException("Use of expired blackboard");

      behavior.lock()->clear();
    });
  methods.registerMethodWithSignature<std::weak_ptr<Blackboard>, std::weak_ptr<BehaviorState>>(
    "blackboard", [](std::weak_ptr<BehaviorState> const& behavior) -> std::weak_ptr<Blackboard> {
      if (behavior.expired())
        throw StarException("Use of expired blackboard");

      return behavior.lock()->blackboardPtr();
    });
  return methods;
}

LuaValue LuaConverter<NodeStatus>::from(LuaEngine&, NodeStatus const& status) {
  if (status == NodeStatus::Success)
    return true;
  else if (status == NodeStatus::Failure)
    return false;
  else
    return {};
}

NodeStatus LuaConverter<NodeStatus>::to(LuaEngine&, LuaValue const& v) {
  if (v.is<LuaBoolean>())
    return v.get<LuaBoolean>() == true ? NodeStatus::Success : NodeStatus::Failure;
  else
    return NodeStatus::Running;
}

LuaMethods<std::weak_ptr<Blackboard>> LuaUserDataMethods<std::weak_ptr<Blackboard>>::make() {
  LuaMethods<std::weak_ptr<Blackboard>> methods;

  auto get =[](std::weak_ptr<Blackboard> const& board, NodeParameterType const& type, String const& key) -> LuaValue {
    if (board.expired())
      throw StarException("Use of expired blackboard");

    return board.lock()->get(type, key);
  };
  auto set = [](std::weak_ptr<Blackboard> const& board, NodeParameterType const& type, String const& key, LuaValue const& value) {
    if (board.expired())
      throw StarException("Use of expired blackboard");

    board.lock()->set(type, key, value);
  };

  methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String, String>("get",
    [&](std::weak_ptr<Blackboard> const& board, String const& type, String const& key) -> LuaValue {
      return get(board, NodeParameterTypeNames.getLeft(type), key);
    });
  methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, String, LuaValue>("set",
    [&](std::weak_ptr<Blackboard> const& board, String const& type, String const& key, LuaValue const& value) {
      set(board, NodeParameterTypeNames.getLeft(type), key, value);
    });

  methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String>(
    "getEntity", [&](std::weak_ptr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Entity, key);
    });
  methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String>(
    "getPosition", [&](std::weak_ptr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Position, key);
    });
  methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String>(
    "getVec2", [&](std::weak_ptr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Vec2, key);
    });
  methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String>(
    "getNumber", [&](std::weak_ptr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Number, key);
    });
  methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String>(
    "getBool", [&](std::weak_ptr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Bool, key);
    });
  methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String>(
    "getList", [&](std::weak_ptr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::List, key);
    });
  methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String>(
    "getTable", [&](std::weak_ptr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::Table, key);
    });
  methods.registerMethodWithSignature<LuaValue, std::weak_ptr<Blackboard>, String>(
    "getString", [&](std::weak_ptr<Blackboard> const& board, String const& key) -> LuaValue {
      return get(board, NodeParameterType::String, key);
    });


  methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, LuaValue>(
    "setEntity", [&](std::weak_ptr<Blackboard> const& board, String const& key, LuaValue const& value) {
      set(board, NodeParameterType::Entity, key, value);
    });
  methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, LuaValue>(
    "setPosition", [&](std::weak_ptr<Blackboard> const& board, String const& key, LuaValue const& value) {
      set(board, NodeParameterType::Position, key, value);
    });
  methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, LuaValue>(
    "setVec2", [&](std::weak_ptr<Blackboard> const& board, String const& key, LuaValue const& value) {
      set(board, NodeParameterType::Vec2, key, value);
    });
  methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, LuaValue>(
    "setNumber", [&](std::weak_ptr<Blackboard> const& board, String const& key, LuaValue const& value) {
      set(board, NodeParameterType::Number, key, value);
    });
  methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, LuaValue>(
    "setBool", [&](std::weak_ptr<Blackboard> const& board, String const& key, LuaValue const& value) {
      set(board, NodeParameterType::Bool, key, value);
    });
  methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, LuaValue>(
    "setList", [&](std::weak_ptr<Blackboard> const& board, String const& key, LuaValue const& value) {
      set(board, NodeParameterType::List, key, value);
    });
  methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, LuaValue>(
    "setTable", [&](std::weak_ptr<Blackboard> const& board, String const& key, LuaValue const& value) {
      set(board, NodeParameterType::Table, key, value);
    });
  methods.registerMethodWithSignature<void, std::weak_ptr<Blackboard>, String, LuaValue>(
    "setString", [&](std::weak_ptr<Blackboard> const& board, String const& key, LuaValue const& value) {
      set(board, NodeParameterType::String, key, value);
    });
  return methods;
}

LuaMethods<std::shared_ptr<AudioInstance>> LuaUserDataMethods<std::shared_ptr<AudioInstance>>::make() {
    LuaMethods<std::shared_ptr<AudioInstance>> methods;

    methods.registerMethod("position",
    [&](std::shared_ptr<AudioInstance> const& audio) -> Maybe<Vec2F> {
        return audio->position();
    });
    
    methods.registerMethod("setPosition",
    [&](std::shared_ptr<AudioInstance> const& audio, Maybe<Vec2F> const& pos) {
        audio->setPosition(pos);
    });
    
    methods.registerMethod("translate",
    [&](std::shared_ptr<AudioInstance> const& audio, Vec2F const& distance) {
        audio->translate(distance);
    });
    
    methods.registerMethod("rangeMultiplier",
    [&](std::shared_ptr<AudioInstance> const& audio) -> float {
        return audio->rangeMultiplier();
    });
    
    methods.registerMethod("setRangeMultiplier",
    [&](std::shared_ptr<AudioInstance> const& audio, float const& rm) {
        audio->setRangeMultiplier(rm);
    });
    
    methods.registerMethod("setVolume",
    [&](std::shared_ptr<AudioInstance> const& audio, float const& targetValue, Maybe<float> const& rampTime) {
        audio->setVolume(targetValue, rampTime.value(0.0f));
    });
    
    methods.registerMethod("setPitchMultiplier",
    [&](std::shared_ptr<AudioInstance> const& audio, float const& targetValue, Maybe<float> const& rampTime) {
        audio->setPitchMultiplier(targetValue, rampTime.value(0.0f));
    });
    
    methods.registerMethod("loops",
    [&](std::shared_ptr<AudioInstance> const& audio) -> int {
        return audio->loops();
    });
    
    methods.registerMethod("setLoops",
    [&](std::shared_ptr<AudioInstance> const& audio, int const& loops) {
        audio->setLoops(loops);
    });
    
    methods.registerMethod("currentTime",
    [&](std::shared_ptr<AudioInstance> const& audio) -> double {
        return audio->currentTime();
    });
    
    methods.registerMethod("totalTime",
    [&](std::shared_ptr<AudioInstance> const& audio) -> double {
        return audio->totalTime();
    });
    
    methods.registerMethod("seekTime",
    [&](std::shared_ptr<AudioInstance> const& audio, double const& time) {
        audio->seekTime(time);
    });
    
    methods.registerMethod("mixerGroup",
    [&](std::shared_ptr<AudioInstance> const& audio) -> String {
        return MixerGroupNames.getRight(audio->mixerGroup());
    });
    
    methods.registerMethod("setMixerGroup",
    [&](std::shared_ptr<AudioInstance> const& audio, String const& group) {
        audio->setMixerGroup(MixerGroupNames.getLeft(group));
    });
    
    methods.registerMethod("setClockStart",
    [&](std::shared_ptr<AudioInstance> const& audio, Maybe<std::int64_t> const& time) {
        audio->setClockStart(time);
    });
    
    methods.registerMethod("setClockStop",
    [&](std::shared_ptr<AudioInstance> const& audio, Maybe<std::int64_t> const& time, Maybe<std::int64_t> const& fadeOutTime) {
        audio->setClockStop(time,fadeOutTime.value(0));
    });
    
    methods.registerMethod("stop",
    [&](std::shared_ptr<AudioInstance> const& audio, Maybe<float> const& rampTime) {
        audio->stop(rampTime.value(0.0f));
    });
    
    methods.registerMethod("finished",
    [&](std::shared_ptr<AudioInstance> const& audio) -> bool {
        return audio->finished();
    });


    return methods;
} 

}
