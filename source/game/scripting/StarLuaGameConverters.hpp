#pragma once

#include "StarActorMovementController.hpp"
#include "StarBehaviorState.hpp"
#include "StarCollectionDatabase.hpp"
#include "StarCollisionBlock.hpp"
#include "StarConfig.hpp"
#include "StarDamage.hpp"
#include "StarDrawable.hpp"
#include "StarEntity.hpp"
#include "StarInventoryTypes.hpp"
#include "StarLuaConverters.hpp"// IWYU pragma: keep
#include "StarPhysicsEntity.hpp"
#include "StarPlatformerAStar.hpp"
#include "StarRpcThreadPromise.hpp"

import std;

namespace Star {

template <>
struct LuaConverter<InventorySlot> {
  static auto from(LuaEngine& engine, InventorySlot k) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<InventorySlot>;
};

template <>
struct LuaConverter<CollisionKind> {
  static auto from(LuaEngine& engine, CollisionKind k) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<CollisionKind>;
};

template <>
struct LuaConverter<CollisionSet> {
  static auto from(LuaEngine& engine, CollisionSet const& s) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<CollisionSet>;
};

template <typename T>
struct LuaConverter<RpcPromise<T>> : LuaUserDataConverter<RpcPromise<T>> {};

template <typename T>
struct LuaUserDataMethods<RpcPromise<T>> {
  static auto make() -> LuaMethods<RpcPromise<T>>;
};

template <typename T>
struct LuaConverter<RpcThreadPromise<T>> : LuaUserDataConverter<RpcThreadPromise<T>> {};

template <typename T>
struct LuaUserDataMethods<RpcThreadPromise<T>> {
  static auto make() -> LuaMethods<RpcThreadPromise<T>>;
};

template <>
struct LuaConverter<PlatformerAStar::Path> {
  static auto from(LuaEngine& engine, PlatformerAStar::Path const& path) -> LuaValue;
};

template <>
struct LuaConverter<PlatformerAStar::PathFinder> : LuaUserDataConverter<PlatformerAStar::PathFinder> {};

template <>
struct LuaUserDataMethods<PlatformerAStar::PathFinder> {
  static auto make() -> LuaMethods<PlatformerAStar::PathFinder>;
};

template <>
struct LuaConverter<PlatformerAStar::Parameters> {
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<PlatformerAStar::Parameters>;
};

template <>
struct LuaConverter<ActorJumpProfile> {
  static auto from(LuaEngine& engine, ActorJumpProfile const& v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<ActorJumpProfile>;
};

template <>
struct LuaConverter<ActorMovementParameters> {
  static auto from(LuaEngine& engine, ActorMovementParameters const& v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<ActorMovementParameters>;
};

template <>
struct LuaConverter<ActorMovementModifiers> {
  static auto from(LuaEngine& engine, ActorMovementModifiers const& v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<ActorMovementModifiers>;
};

template <>
struct LuaConverter<StatModifier> {
  static auto from(LuaEngine& engine, StatModifier const& v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue v) -> std::optional<StatModifier>;
};

template <>
struct LuaConverter<EphemeralStatusEffect> {
  static auto from(LuaEngine& engine, EphemeralStatusEffect const& v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<EphemeralStatusEffect>;
};

template <>
struct LuaConverter<DamageRequest> {
  static auto from(LuaEngine& engine, DamageRequest const& v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<DamageRequest>;
};

template <>
struct LuaConverter<DamageNotification> {
  static auto from(LuaEngine& engine, DamageNotification const& v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<DamageNotification>;
};

template <>
struct LuaConverter<LiquidLevel> {
  static auto from(LuaEngine& engine, LiquidLevel const& v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<LiquidLevel>;
};

template <>
struct LuaConverter<Drawable> {
  static auto from(LuaEngine& engine, Drawable const& v) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Drawable>;
};

template <typename T>
auto LuaUserDataMethods<RpcPromise<T>>::make() -> LuaMethods<RpcPromise<T>> {
  LuaMethods<RpcPromise<T>> methods;
  methods.template registerMethodWithSignature<bool, RpcPromise<T>&>("finished", mem_fn(&RpcPromise<T>::finished));
  methods.template registerMethodWithSignature<bool, RpcPromise<T>&>("succeeded", mem_fn(&RpcPromise<T>::succeeded));
  methods.template registerMethodWithSignature<std::optional<T>, RpcPromise<T>&>("result", mem_fn(&RpcPromise<T>::result));
  methods.template registerMethodWithSignature<std::optional<String>, RpcPromise<T>&>("error", mem_fn(&RpcPromise<T>::error));
  return methods;
}

template <typename T>
auto LuaUserDataMethods<RpcThreadPromise<T>>::make() -> LuaMethods<RpcThreadPromise<T>> {
  LuaMethods<RpcThreadPromise<T>> methods;
  methods.template registerMethodWithSignature<bool, RpcThreadPromise<T>&>("finished", mem_fn(&RpcThreadPromise<T>::finished));
  methods.template registerMethodWithSignature<bool, RpcThreadPromise<T>&>("succeeded", mem_fn(&RpcThreadPromise<T>::succeeded));
  methods.template registerMethodWithSignature<std::optional<T>, RpcThreadPromise<T>&>("result", mem_fn(&RpcThreadPromise<T>::result));
  methods.template registerMethodWithSignature<std::optional<String>, RpcThreadPromise<T>&>("error", mem_fn(&RpcThreadPromise<T>::error));
  return methods;
}

template <>
struct LuaConverter<Collection> {
  static auto from(LuaEngine& engine, Collection const& c) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Collection>;
};

template <>
struct LuaConverter<Collectable> {
  static auto from(LuaEngine& engine, Collectable const& c) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> std::optional<Collectable>;
};

// BehaviorState contains Lua references, putting it in a UserData violates
// the "don't put lua references in userdata, just don't" rule. We get around it by keeping
// a weak pointer to the behavior state, forcing it to be destroyed elsewhere.
template <>
struct LuaConverter<WeakPtr<BehaviorState>> : LuaUserDataConverter<WeakPtr<BehaviorState>> {};

template <>
struct LuaUserDataMethods<WeakPtr<BehaviorState>> {
  static auto make() -> LuaMethods<WeakPtr<BehaviorState>>;
};

template <>
struct LuaConverter<NodeStatus> {
  static auto from(LuaEngine& engine, NodeStatus const& status) -> LuaValue;
  static auto to(LuaEngine& engine, LuaValue const& v) -> NodeStatus;
};

template <>
struct LuaConverter<PhysicsMovingCollision> {
  static auto from(LuaEngine& engine, PhysicsMovingCollision const& v) -> LuaValue;
};

// Weak pointer for the same reasons as BehaviorState.
template <>
struct LuaConverter<WeakPtr<Blackboard>> : LuaUserDataConverter<WeakPtr<Blackboard>> {};

template <>
struct LuaUserDataMethods<WeakPtr<Blackboard>> {
  static auto make() -> LuaMethods<WeakPtr<Blackboard>>;
};

template <>
struct LuaConverter<Ptr<Entity>> : LuaUserDataConverter<Ptr<Entity>> {};

template <>
struct LuaUserDataMethods<Ptr<Entity>> {
  static auto make() -> LuaMethods<Ptr<Entity>>;
};

}// namespace Star
