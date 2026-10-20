module;
import std;
import star.lua;
import star.lua_converters;
import star.inventory_types;
import star.collision_block;
import star.platformer_astar_types;
import star.status_types;

namespace Star {

class Entity;
class AudioInstance;
struct ActorJumpProfile;
struct ActorMovementParameters;
struct ActorMovementModifiers;
struct DamageRequest;
struct DamageNotification;
struct LiquidLevel;
struct Drawable;
struct PhysicsMovingCollision;

struct Collection;
struct Collectable;
class BehaviorState;
class Blackboard;
enum class NodeStatus;

template <>
struct LuaConverter<InventorySlot> {
  static LuaValue from(LuaEngine& engine, InventorySlot k);
  static std::optional<InventorySlot> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<CollisionKind> {
  static LuaValue from(LuaEngine& engine, CollisionKind k);
  static std::optional<CollisionKind> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<CollisionSet> {
  static LuaValue from(LuaEngine& engine, CollisionSet const& s);
  static std::optional<CollisionSet> to(LuaEngine& engine, LuaValue const& v);
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
  static std::optional<PlatformerAStar::Parameters> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<ActorJumpProfile> {
  static LuaValue from(LuaEngine& engine, ActorJumpProfile const& v);
  static std::optional<ActorJumpProfile> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<ActorMovementParameters> {
  static LuaValue from(LuaEngine& engine, ActorMovementParameters const& v);
  static std::optional<ActorMovementParameters> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<ActorMovementModifiers> {
  static LuaValue from(LuaEngine& engine, ActorMovementModifiers const& v);
  static std::optional<ActorMovementModifiers> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<StatModifier> {
  static LuaValue from(LuaEngine& engine, StatModifier const& v);
  static std::optional<StatModifier> to(LuaEngine& engine, LuaValue v);
};

template <>
struct LuaConverter<EphemeralStatusEffect> {
  static LuaValue from(LuaEngine& engine, EphemeralStatusEffect const& v);
  static std::optional<EphemeralStatusEffect> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<DamageRequest> {
  static LuaValue from(LuaEngine& engine, DamageRequest const& v);
  static std::optional<DamageRequest> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<DamageNotification> {
  static LuaValue from(LuaEngine& engine, DamageNotification const& v);
  static std::optional<DamageNotification> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<LiquidLevel> {
  static LuaValue from(LuaEngine& engine, LiquidLevel const& v);
  static std::optional<LiquidLevel> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<Drawable> {
  static LuaValue from(LuaEngine& engine, Drawable const& v);
  static std::optional<Drawable> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<Collection> {
  static LuaValue from(LuaEngine& engine, Collection const& c);
  static std::optional<Collection> to(LuaEngine& engine, LuaValue const& v);
};

template <>
struct LuaConverter<Collectable> {
  static LuaValue from(LuaEngine& engine, Collectable const& c);
  static std::optional<Collectable> to(LuaEngine& engine, LuaValue const& v);
};

// BehaviorState contains Lua references, putting it in a UserData violates
// the "don't put lua references in userdata, just don't" rule. We get around it by keeping
// a weak pointer to the behavior state, forcing it to be destroyed elsewhere.
template <>
struct LuaConverter<std::weak_ptr<BehaviorState>> : LuaUserDataConverter<std::weak_ptr<BehaviorState>> {};

template <>
struct LuaUserDataMethods<std::weak_ptr<BehaviorState>> {
  static LuaMethods<std::weak_ptr<BehaviorState>> make();
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
struct LuaConverter<std::weak_ptr<Blackboard>> : LuaUserDataConverter<std::weak_ptr<Blackboard>> {};

template <>
struct LuaUserDataMethods<std::weak_ptr<Blackboard>> {
  static LuaMethods<std::weak_ptr<Blackboard>> make();
};

template <>
struct LuaConverter<std::shared_ptr<Entity>> : LuaUserDataConverter<std::shared_ptr<Entity>> {};

template <>
struct LuaUserDataMethods<std::shared_ptr<Entity>> {
  static LuaMethods<std::shared_ptr<Entity>> make();
};

template <>
struct LuaConverter<std::shared_ptr<AudioInstance>> : LuaUserDataConverter<std::shared_ptr<AudioInstance>> {};

template <>
struct LuaUserDataMethods<std::shared_ptr<AudioInstance>> {
  static LuaMethods<std::shared_ptr<AudioInstance>> make();
};

}

export module star.lua_game_converters;

export namespace Star {
  template <>
  struct LuaConverter<InventorySlot>;
  template <>
  struct LuaConverter<CollisionKind>;
  template <>
  struct LuaConverter<CollisionSet>;
  template <>
  struct LuaConverter<PlatformerAStar::Path>;
  template <>
  struct LuaConverter<PlatformerAStar::PathFinder>;
  template <>
  struct LuaUserDataMethods<PlatformerAStar::PathFinder>;
  template <>
  struct LuaConverter<PlatformerAStar::Parameters>;
  template <>
  struct LuaConverter<ActorJumpProfile>;
  template <>
  struct LuaConverter<ActorMovementParameters>;
  template <>
  struct LuaConverter<ActorMovementModifiers>;
  template <>
  struct LuaConverter<StatModifier>;
  template <>
  struct LuaConverter<EphemeralStatusEffect>;
  template <>
  struct LuaConverter<DamageRequest>;
  template <>
  struct LuaConverter<DamageNotification>;
  template <>
  struct LuaConverter<LiquidLevel>;
  template <>
  struct LuaConverter<Drawable>;
  template <>
  struct LuaConverter<Collection>;
  template <>
  struct LuaConverter<Collectable>;
  template <>
  struct LuaConverter<std::weak_ptr<BehaviorState>>;
  template <>
  struct LuaUserDataMethods<std::weak_ptr<BehaviorState>>;
  template <>
  struct LuaConverter<NodeStatus>;
  template <>
  struct LuaConverter<PhysicsMovingCollision>;
  template <>
  struct LuaConverter<std::weak_ptr<Blackboard>>;
  template <>
  struct LuaUserDataMethods<std::weak_ptr<Blackboard>>;
  template <>
  struct LuaConverter<std::shared_ptr<Entity>>;
  template <>
  struct LuaUserDataMethods<std::shared_ptr<Entity>>;
  template <>
  struct LuaConverter<std::shared_ptr<AudioInstance>>;
  template <>
  struct LuaUserDataMethods<std::shared_ptr<AudioInstance>>;

  using ::Star::LuaConverter;
  using ::Star::LuaUserDataMethods;
}
