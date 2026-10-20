module;
import star.variant;
import star.maybe;
import star.json;

import star.string;
import star.bi_map;
import star.vector;
import star.rpc_promise;
import star.math_common;
import std;

namespace Star {

typedef Variant<String,RpcPromise<String>> ServerCommandResult;
typedef Variant<Json,RpcPromise<Json>> ChainableJsonMessageResponse;

enum class Direction : std::uint8_t {
  Left,
  Right
};
extern EnumMap<Direction> const DirectionNames;

inline Direction operator-(Direction dir) {
  if (dir == Direction::Left)
    return Direction::Right;
  return Direction::Left;
}

inline int numericalDirection(Maybe<Direction> direction) {
  if (!direction)
    return 0;
  else
    return *direction == Direction::Left ? -1 : 1;
}

template <typename NumType>
inline Maybe<Direction> directionOf(NumType const& n) {
  if (n == 0)
    return {};
  else
    return n < 0 ? Direction::Left : Direction::Right;
}

enum class Gender : std::uint8_t {
  Male,
  Female
};
extern EnumMap<Gender> const GenderNames;

enum class FireMode : std::uint8_t {
  None,
  Primary,
  Alt
};
extern EnumMap<FireMode> const FireModeNames;

enum class ToolHand : std::uint8_t {
  Primary,
  Alt
};
extern EnumMap<ToolHand> const ToolHandNames;

enum class TileLayer : std::uint8_t {
  Foreground,
  Background
};
extern EnumMap<TileLayer> const TileLayerNames;

enum class MoveControlType : std::uint8_t {
  Left,
  Right,
  Down,
  Up,
  Jump
};
extern EnumMap<MoveControlType> const MoveControlTypeNames;

enum class PortraitMode {
  Head,
  Bust,
  Full,
  FullNeutral,
  FullNude,
  FullNeutralNude
};
extern EnumMap<PortraitMode> const PortraitModeNames;

enum class Rarity {
  Common,
  Uncommon,
  Rare,
  Legendary,
  Essential
};
extern EnumMap<Rarity> const RarityNames;

// Transformation from tile space to pixel space.  Number of pixels in 1.0
// distance (one tile).
inline constexpr unsigned TilePixels = 8;

struct GameTiming {
  float timescale = 1.0f;
  float timestep = 1.0f / 60.0f;
  float serverTimestep = 1.0f / 60.0f;
};
inline constexpr float SystemWorldTimestep = 1.0f / 20.0f;

inline constexpr std::size_t WorldSectorSize = 32;

typedef std::int32_t EntityId;
inline constexpr EntityId NullEntityId = 0;
inline constexpr EntityId MinServerEntityId = 1;
inline constexpr EntityId MaxServerEntityId = std::numeric_limits<EntityId>::max();

// Whether this entity is controlled by it's world, or synced from a different
// world.  Does not necessarily correspond to client / server world (player is
// master on client).
enum class EntityMode {
  Master,
  Slave
};

typedef std::uint16_t ConnectionId;
inline constexpr ConnectionId ServerConnectionId = 0;
// Minimum and maximum valid client ids
inline constexpr ConnectionId MinClientConnectionId = 1;
inline constexpr ConnectionId MaxClientConnectionId = 16383;
inline constexpr ConnectionId MinClientSubWorldConnectionId = MaxClientConnectionId+1;
inline constexpr ConnectionId MaxClientSubWorldConnectionId = MaxClientConnectionId-MinClientConnectionId+MinClientSubWorldConnectionId;

inline ConnectionId mainToSubWorldConnectionId(ConnectionId const& id) {
  return id-MinClientConnectionId+MinClientSubWorldConnectionId;
}
inline ConnectionId maybeMainToSubWorldConnectionId(ConnectionId const& id) {
  return id >= MinClientSubWorldConnectionId ? id-MinClientConnectionId+MinClientSubWorldConnectionId : id;
}
inline ConnectionId subWorldToMainConnectionId(ConnectionId const& id) {
  return id-MinClientSubWorldConnectionId+MinClientConnectionId;
}

// for client sub-world threads. indexes by *world* rather than by thread.
// this should allow the client to possibly split the subworlds to multiple threads in the future.
typedef std::uint16_t ClientSubWorldId;
inline constexpr ConnectionId MainClientWorldId = 0;
inline constexpr ConnectionId MinClientSubWorldId = 1;
inline constexpr ConnectionId MaxClientSubWorldId = std::numeric_limits<ClientSubWorldId>::max();

template <typename Vec2T>
inline Vec2F centerOfTile(Vec2T const& tile) {
  return Vec2F(tile.floor()) + Vec2F::filled(0.5);
}

typedef std::uint16_t DungeonId;

inline constexpr DungeonId NoDungeonId = 65535;
inline constexpr DungeonId SpawnDungeonId = 65534;
inline constexpr DungeonId BiomeMicroDungeonId = 65533;
// meta dungeon signalling player built structures
inline constexpr DungeonId ConstructionDungeonId = 65532;
// indicates a block that has been destroyed
inline constexpr DungeonId DestroyedBlockDungeonId = 65531;

// dungeonId for zero-g areas with and without tile protection
inline constexpr DungeonId ZeroGDungeonId = 65525;
inline constexpr DungeonId ProtectedZeroGDungeonId = 65524;

// The first dungeon id that is reserved for special hard-coded dungeon values.
inline constexpr DungeonId FirstMetaDungeonId = 65520;

inline bool isRealDungeon(DungeonId dungeon) {
  return dungeon < FirstMetaDungeonId;
}

// Returns the inclusive beginning and end of the entity id space for the
// given connection.  All client connection id spaces will be within the range
// [-2^31, -1].
std::pair<EntityId, EntityId> connectionEntitySpace(ConnectionId connectionId);

bool entityIdInSpace(EntityId entityId, ConnectionId connectionId);

ConnectionId connectionForEntity(EntityId entityId);

// Returns an angle in the range [-pi / 2, pi / 2], and the horizontal
// hemisphere of the angle.  The angle is specified as positive being upward
// rotation and negative being downward rotation, unless ccRotation is true, in
// which case the angle is always positive == counter-clocwise.
std::pair<float, Direction> getAngleSide(float angle, bool ccRotation = false);

enum class TileDamageResult {
  None = 0,
  Protected = 1,
  Normal = 2,
};

}


export module star.game_types;

export namespace Star {
  using ::Star::ServerCommandResult;
  using ::Star::ChainableJsonMessageResponse;
  using ::Star::Direction;
  using ::Star::DirectionNames;
  using ::Star::operator-;
  using ::Star::numericalDirection;
  using ::Star::directionOf;
  using ::Star::Gender;
  using ::Star::GenderNames;
  using ::Star::FireMode;
  using ::Star::FireModeNames;
  using ::Star::ToolHand;
  using ::Star::ToolHandNames;
  using ::Star::TileLayer;
  using ::Star::TileLayerNames;
  using ::Star::MoveControlType;
  using ::Star::MoveControlTypeNames;
  using ::Star::PortraitMode;
  using ::Star::PortraitModeNames;
  using ::Star::Rarity;
  using ::Star::RarityNames;
  using ::Star::TilePixels;
  using ::Star::GameTiming;
  using ::Star::SystemWorldTimestep;
  using ::Star::WorldSectorSize;
  using ::Star::EntityId;
  using ::Star::NullEntityId;
  using ::Star::MinServerEntityId;
  using ::Star::MaxServerEntityId;
  using ::Star::EntityMode;
  using ::Star::ConnectionId;
  using ::Star::ServerConnectionId;
  using ::Star::MinClientConnectionId;
  using ::Star::MaxClientConnectionId;
  using ::Star::MinClientSubWorldConnectionId;
  using ::Star::MaxClientSubWorldConnectionId;
  using ::Star::mainToSubWorldConnectionId;
  using ::Star::maybeMainToSubWorldConnectionId;
  using ::Star::subWorldToMainConnectionId;
  using ::Star::ClientSubWorldId;
  using ::Star::MainClientWorldId;
  using ::Star::MinClientSubWorldId;
  using ::Star::MaxClientSubWorldId;
  using ::Star::centerOfTile;
  using ::Star::DungeonId;
  using ::Star::NoDungeonId;
  using ::Star::SpawnDungeonId;
  using ::Star::BiomeMicroDungeonId;
  using ::Star::ConstructionDungeonId;
  using ::Star::DestroyedBlockDungeonId;
  using ::Star::ZeroGDungeonId;
  using ::Star::ProtectedZeroGDungeonId;
  using ::Star::FirstMetaDungeonId;
  using ::Star::isRealDungeon;
  using ::Star::connectionEntitySpace;
  using ::Star::entityIdInSpace;
  using ::Star::connectionForEntity;
  using ::Star::getAngleSide;
  using ::Star::TileDamageResult;
}
