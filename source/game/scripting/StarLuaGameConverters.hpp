#pragma once
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarLuaConverters.hpp"
#include "StarBiMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"
#include "StarGameTypes.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarRpcPromise.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarArray.hpp"
#include "StarEither.hpp"
#include "StarNetElementFloatFields.hpp"


import star.light_source;
import star.entity;
import star.force_regions;
import star.physics_entity;
import star.inventory_types;
import star.collision_block;
import star.platformer_astar_types;

import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.tile_modification;
#include "StarLuaRoot.hpp"
import star.world;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.world_parameters;
import star.celestial_parameters;
import star.uuid;
import star.warping;
import star.sky_parameters;
import star.system_world;
import star.drawable;
import star.mixer;

namespace Star {

struct Collection;
struct Collectable;
STAR_CLASS(BehaviorState);
STAR_CLASS(Blackboard);
enum class NodeStatus;

template <>
struct LuaConverter<InventorySlot> {
  static LuaValue from(LuaEngine& engine, InventorySlot k);
  static Maybe<InventorySlot> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<CollisionKind> {
  static LuaValue from(LuaEngine& engine, CollisionKind k);
  static Maybe<CollisionKind> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<CollisionSet> {
  static LuaValue from(LuaEngine& engine, CollisionSet const& s);
  static Maybe<CollisionSet> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<PlatformerAStar::Path> {
  static LuaValue from(LuaEngine& engine, PlatformerAStar::Path const& path);
};

template <>
struct LuaConverter<PlatformerAStar::PathFinder> : LuaUserDataConverter<PlatformerAStar::PathFinder> {};

template <>
struct LuaUserDataMethods<PlatformerAStar::PathFinder> {
  static LuaMethods<PlatformerAStar::PathFinder> make();
};

template <>
struct LuaConverter<PlatformerAStar::Parameters> {
  static Maybe<PlatformerAStar::Parameters> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<ActorJumpProfile> {
  static LuaValue from(LuaEngine& engine, ActorJumpProfile const& v);
  static Maybe<ActorJumpProfile> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<ActorMovementParameters> {
  static LuaValue from(LuaEngine& engine, ActorMovementParameters const& v);
  static Maybe<ActorMovementParameters> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<ActorMovementModifiers> {
  static LuaValue from(LuaEngine& engine, ActorMovementModifiers const& v);
  static Maybe<ActorMovementModifiers> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<StatModifier> {
  static LuaValue from(LuaEngine& engine, StatModifier const& v);
  static Maybe<StatModifier> to(LuaEngine& engine, LuaValue v);
};

template <>
struct LuaConverter<EphemeralStatusEffect> {
  static LuaValue from(LuaEngine& engine, EphemeralStatusEffect const& v);
  static Maybe<EphemeralStatusEffect> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<DamageRequest> {
  static LuaValue from(LuaEngine& engine, DamageRequest const& v);
  static Maybe<DamageRequest> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<DamageNotification> {
  static LuaValue from(LuaEngine& engine, DamageNotification const& v);
  static Maybe<DamageNotification> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<LiquidLevel> {
  static LuaValue from(LuaEngine& engine, LiquidLevel const& v);
  static Maybe<LiquidLevel> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<Drawable> {
  static LuaValue from(LuaEngine& engine, Drawable const& v);
  static Maybe<Drawable> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<Collection> {
  static LuaValue from(LuaEngine& engine, Collection const& c);
  static Maybe<Collection> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<Collectable> {
  static LuaValue from(LuaEngine& engine, Collectable const& c);
  static Maybe<Collectable> to(LuaEngine& engine, LuaValue const& v);
};

// BehaviorState contains Lua references, putting it in a UserData violates
// the "don't put lua references in userdata, just don't" rule. We get around it by keeping
// a weak pointer to the behavior state, forcing it to be destroyed elsewhere.
template <>
struct LuaConverter<BehaviorStateWeakPtr> : LuaUserDataConverter<BehaviorStateWeakPtr> {};

template <>
struct LuaUserDataMethods<BehaviorStateWeakPtr> {
  static LuaMethods<BehaviorStateWeakPtr> make();
};

template <>
struct LuaConverter<NodeStatus> {
  static LuaValue from(LuaEngine& engine, NodeStatus const& status);
  static NodeStatus to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<PhysicsMovingCollision> {
  static LuaValue from(LuaEngine& engine, PhysicsMovingCollision const& v);
};

// Weak pointer for the same reasons as BehaviorState.
template <>
struct LuaConverter<BlackboardWeakPtr> : LuaUserDataConverter<BlackboardWeakPtr> {};

template <>
struct LuaUserDataMethods<BlackboardWeakPtr> {
  static LuaMethods<BlackboardWeakPtr> make();
};

template <>
struct LuaConverter<EntityPtr> : LuaUserDataConverter<EntityPtr> {};

template <>
struct LuaUserDataMethods<EntityPtr> {
  static LuaMethods<EntityPtr> make();
};

template <>
struct LuaConverter<AudioInstancePtr> : LuaUserDataConverter<AudioInstancePtr> {};

template <>
struct LuaUserDataMethods<AudioInstancePtr> {
  static LuaMethods<AudioInstancePtr> make();
};

}
