#pragma once

#include "StarPhysicsEntity.hpp"
#include "StarLuaConverters.hpp"
#include "StarInventoryTypes.hpp"
#include "StarCollisionBlock.hpp"
#include "StarPlatformerAStar.hpp"
#include "StarActorMovementController.hpp"
#include "StarDamage.hpp"
#include "StarCollectionDatabase.hpp"
#include "StarBehaviorState.hpp"
#include "StarSystemWorld.hpp"
#include "StarDrawable.hpp"
#include "StarRpcThreadPromise.hpp"
#include "StarEntity.hpp"

namespace Star {

template <>
struct LuaConverter<InventorySlot> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, InventorySlot k);
  [[nodiscard]] static Maybe<InventorySlot> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<CollisionKind> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, CollisionKind k);
  [[nodiscard]] static Maybe<CollisionKind> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<CollisionSet> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, CollisionSet const& s);
  [[nodiscard]] static Maybe<CollisionSet> to(LuaEngine& engine, LuaValue const& v);
};

template <typename T>
struct LuaConverter<RpcPromise<T>> : LuaUserDataConverter<RpcPromise<T>> {};

template <typename T>
struct LuaUserDataMethods<RpcPromise<T>> {
  [[nodiscard]] static LuaMethods<RpcPromise<T>> make();
};

template <typename T>
struct LuaConverter<RpcThreadPromise<T>> : LuaUserDataConverter<RpcThreadPromise<T>> {};

template <typename T>
struct LuaUserDataMethods<RpcThreadPromise<T>> {
  [[nodiscard]] static LuaMethods<RpcThreadPromise<T>> make();
};

template <>
struct LuaConverter<PlatformerAStar::Path> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, PlatformerAStar::Path const& path);
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
  [[nodiscard]] static LuaValue from(LuaEngine& engine, ActorJumpProfile const& v);
  [[nodiscard]] static Maybe<ActorJumpProfile> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<ActorMovementParameters> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, ActorMovementParameters const& v);
  [[nodiscard]] static Maybe<ActorMovementParameters> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<ActorMovementModifiers> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, ActorMovementModifiers const& v);
  [[nodiscard]] static Maybe<ActorMovementModifiers> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<StatModifier> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, StatModifier const& v);
  [[nodiscard]] static Maybe<StatModifier> to(LuaEngine& engine, LuaValue v);
};

template <>
struct LuaConverter<EphemeralStatusEffect> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, EphemeralStatusEffect const& v);
  [[nodiscard]] static Maybe<EphemeralStatusEffect> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<DamageRequest> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, DamageRequest const& v);
  [[nodiscard]] static Maybe<DamageRequest> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<DamageNotification> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, DamageNotification const& v);
  [[nodiscard]] static Maybe<DamageNotification> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<LiquidLevel> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, LiquidLevel const& v);
  [[nodiscard]] static Maybe<LiquidLevel> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<Drawable> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, Drawable const& v);
  [[nodiscard]] static Maybe<Drawable> to(LuaEngine& engine, LuaValue const& v);
};

template <typename T>
LuaMethods<RpcPromise<T>> LuaUserDataMethods<RpcPromise<T>>::make() {
  LuaMethods<RpcPromise<T>> methods;
  methods.template registerMethodWithSignature<bool, RpcPromise<T>&>("finished", mem_fn(&RpcPromise<T>::finished));
  methods.template registerMethodWithSignature<bool, RpcPromise<T>&>("succeeded", mem_fn(&RpcPromise<T>::succeeded));
  methods.template registerMethodWithSignature<Maybe<T>, RpcPromise<T>&>("result", mem_fn(&RpcPromise<T>::result));
  methods.template registerMethodWithSignature<Maybe<String>, RpcPromise<T>&>("error", mem_fn(&RpcPromise<T>::error));
  return methods;
}

template <typename T>
LuaMethods<RpcThreadPromise<T>> LuaUserDataMethods<RpcThreadPromise<T>>::make() {
  LuaMethods<RpcThreadPromise<T>> methods;
  methods.template registerMethodWithSignature<bool, RpcThreadPromise<T>&>("finished", mem_fn(&RpcThreadPromise<T>::finished));
  methods.template registerMethodWithSignature<bool, RpcThreadPromise<T>&>("succeeded", mem_fn(&RpcThreadPromise<T>::succeeded));
  methods.template registerMethodWithSignature<Maybe<T>, RpcThreadPromise<T>&>("result", mem_fn(&RpcThreadPromise<T>::result));
  methods.template registerMethodWithSignature<Maybe<String>, RpcThreadPromise<T>&>("error", mem_fn(&RpcThreadPromise<T>::error));
  return methods;
}

template <>
struct LuaConverter<Collection> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, Collection const& c);
  [[nodiscard]] static Maybe<Collection> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<Collectable> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, Collectable const& c);
  [[nodiscard]] static Maybe<Collectable> to(LuaEngine& engine, LuaValue const& v);
};

// BehaviorState contains Lua references, putting it in a UserData violates
// the "don't put lua references in userdata, just don't" rule. We get around it by keeping
// a weak pointer to the behavior state, forcing it to be destroyed elsewhere.
template <>
struct LuaConverter<BehaviorStateWeakPtr> : LuaUserDataConverter<BehaviorStateWeakPtr> {};

template <>
struct LuaUserDataMethods<BehaviorStateWeakPtr> {
  [[nodiscard]] static LuaMethods<BehaviorStateWeakPtr> make();
};

template <>
struct LuaConverter<NodeStatus> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, NodeStatus const& status);
  [[nodiscard]] static NodeStatus to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<PhysicsMovingCollision> {
  [[nodiscard]] static LuaValue from(LuaEngine& engine, PhysicsMovingCollision const& v);
};

// Weak pointer for the same reasons as BehaviorState.
template <>
struct LuaConverter<BlackboardWeakPtr> : LuaUserDataConverter<BlackboardWeakPtr> {};

template <>
struct LuaUserDataMethods<BlackboardWeakPtr> {
  [[nodiscard]] static LuaMethods<BlackboardWeakPtr> make();
};

template <>
struct LuaConverter<EntityPtr> : LuaUserDataConverter<EntityPtr> {};

template <>
struct LuaUserDataMethods<EntityPtr> {
  [[nodiscard]] static LuaMethods<EntityPtr> make();
};

}
