module;
#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarGameTypes.hpp"
#include "StarMathCommon.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarEither.hpp"
#include "StarVariant.hpp"
#include "StarColor.hpp"
#include "StarMaybe.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarArray.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetCompatibility.hpp"
#include <functional>
#include "StarSet.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarThread.hpp"
#include "StarInterpolation.hpp"
import star.perlin;
#include "StarBTree.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
#include "StarDataStreamDevices.hpp"
import star.btree_database;
#include "StarOrderedSet.hpp"
#include "StarRpcPromise.hpp"
#include "StarSet.hpp"
#include "StarNetElementFloatFields.hpp"

import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.item_descriptor;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.warping;
import star.world_geometry;
import star.wiring;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
#include "StarLuaRoot.hpp"
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;

namespace Star {

STAR_STRUCT(Packet);

struct StarPacketExceptionTag {
  static constexpr char const* name() { return "StarPacketException"; }
};
using StarPacketException = StarError<StarPacketExceptionTag, IOException>;

extern VersionNumber const StarProtocolVersion;

// Packet types sent between the client and server over a NetSocket.  Does not
// correspond to actual packets, simply logical portions of NetSocket data.
enum class PacketType : uint8_t {
  // Packets used as part of the initial handshake
  ProtocolRequest,
  ProtocolResponse,

  // Packets sent universe server -> universe client
  ServerDisconnect,
  ConnectSuccess,
  ConnectFailure,
  HandshakeChallenge,
  ChatReceive,
  UniverseTimeUpdate,
  CelestialResponse,
  PlayerWarpResult,
  PlanetTypeUpdate,
  Pause,
  ServerInfo,

  // Packets sent universe client -> universe server
  ClientConnect,
  ClientDisconnectRequest,
  HandshakeResponse,
  PlayerWarp,
  FlyShip,
  ChatSend,
  CelestialRequest,

  // Packets sent bidirectionally between the universe client and the universe
  // server
  ClientContextUpdate,

  // Packets sent world server -> world client
  WorldStart,
  WorldStop,
  WorldLayoutUpdate,
  WorldParametersUpdate,
  CentralStructureUpdate,
  TileArrayUpdate,
  TileUpdate,
  TileLiquidUpdate,
  TileDamageUpdate,
  TileModificationFailure,
  GiveItem,
  EnvironmentUpdate,
  UpdateTileProtection,
  SetDungeonGravity,
  SetDungeonBreathable,
  SetPlayerStart,
  FindUniqueEntityResponse,
  Pong,

  // Packets sent world client -> world server
  ModifyTileList,
  DamageTileGroup,
  CollectLiquid,
  RequestDrop,
  SpawnEntity,
  ConnectWire,
  DisconnectAllWires,
  WorldClientStateUpdate,
  FindUniqueEntity,
  WorldStartAcknowledge,
  Ping,

  // Packets sent bidirectionally between world client and world server
  EntityCreate,
  EntityUpdateSet,
  EntityDestroy,
  EntityInteract,
  EntityInteractResult,
  HitRequest,
  DamageRequest,
  DamageNotification,
  EntityMessage,
  EntityMessageResponse,
  UpdateWorldProperties,
  StepUpdate,

  // Packets sent system server -> system client
  SystemWorldStart,
  SystemWorldUpdate,
  SystemObjectCreate,
  SystemObjectDestroy,
  SystemShipCreate,
  SystemShipDestroy,

  // Packets sent system client -> system server
  SystemObjectSpawn,

  // OpenStarbound packets
  ReplaceTileList,
  UpdateWorldTemplate,
  
  ClientCustomWorldRequest,
  ClientCustomWorldResponse,
  ClientCustomWorldCreate,
  
  ClientSubWorldPackets,
  ClientSubWorldRequest,
  ClientSubWorldReject,
  NotifyWorldLoad,
  
  LogMapUpdate,
  
  UniverseMessage,
  UniverseMessageResponse
};
extern EnumMap<PacketType> const PacketTypeNames;

enum class NetCompressionMode : uint8_t {
  None,
  Zstd
};
extern EnumMap<NetCompressionMode> const NetCompressionModeNames;

enum class PacketCompressionMode : uint8_t {
  Disabled,
  Automatic,
  Enabled
};

struct Packet {
  virtual ~Packet();

  virtual PacketType type() const = 0;

  virtual void read(DataStream& ds, NetCompatibilityRules netRules);
  virtual void read(DataStream& ds);
  virtual void write(DataStream& ds, NetCompatibilityRules netRules) const;
  virtual void write(DataStream& ds) const;

  virtual void readJson(Json const& json);
  virtual Json writeJson() const;

  PacketCompressionMode compressionMode() const;
  void setCompressionMode(PacketCompressionMode compressionMode);

  PacketCompressionMode m_compressionMode = PacketCompressionMode::Automatic;
};

PacketPtr createPacket(PacketType type);
PacketPtr createPacket(PacketType type, Maybe<Json> const& args);

template <PacketType PacketT>
struct PacketBase : public Packet {
  static PacketType const Type = PacketT;

  PacketType type() const override { return Type; }
};

struct ProtocolRequestPacket : PacketBase<PacketType::ProtocolRequest> {
  ProtocolRequestPacket();
  ProtocolRequestPacket(VersionNumber requestProtocolVersion);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  VersionNumber requestProtocolVersion;
};

struct ProtocolResponsePacket : PacketBase<PacketType::ProtocolResponse> {
  ProtocolResponsePacket(bool allowed = false, Json info = {});

  void read(DataStream& ds) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  bool allowed;
  Json info;
};

struct ServerDisconnectPacket : PacketBase<PacketType::ServerDisconnect> {
  ServerDisconnectPacket();
  ServerDisconnectPacket(String reason);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String reason;
};

struct ConnectSuccessPacket : PacketBase<PacketType::ConnectSuccess> {
  ConnectSuccessPacket();
  ConnectSuccessPacket(ConnectionId clientId, Uuid serverUuid, CelestialBaseInformation celestialInformation);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ConnectionId clientId;
  Uuid serverUuid;
  CelestialBaseInformation celestialInformation;
};

struct ConnectFailurePacket : PacketBase<PacketType::ConnectFailure> {
  ConnectFailurePacket();
  ConnectFailurePacket(String reason);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String reason;
};

struct HandshakeChallengePacket : PacketBase<PacketType::HandshakeChallenge> {
  HandshakeChallengePacket();
  HandshakeChallengePacket(ByteArray const& passwordSalt);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray passwordSalt;
};

struct ChatReceivePacket : PacketBase<PacketType::ChatReceive> {
  ChatReceivePacket();
  ChatReceivePacket(ChatReceivedMessage receivedMessage);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  ChatReceivedMessage receivedMessage;
};

struct UniverseTimeUpdatePacket : PacketBase<PacketType::UniverseTimeUpdate> {
  UniverseTimeUpdatePacket();
  UniverseTimeUpdatePacket(double universeTime);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  double universeTime;
  float timescale;
};

struct CelestialResponsePacket : PacketBase<PacketType::CelestialResponse> {
  CelestialResponsePacket();
  CelestialResponsePacket(List<CelestialResponse> responses);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  List<CelestialResponse> responses;
};

struct PlayerWarpResultPacket : PacketBase<PacketType::PlayerWarpResult> {
  PlayerWarpResultPacket();
  PlayerWarpResultPacket(bool success, WarpAction warpAction, bool warpActionInvalid);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  bool success;
  WarpAction warpAction;
  bool warpActionInvalid;
};

struct PlanetTypeUpdatePacket : PacketBase<PacketType::PlanetTypeUpdate> {
  PlanetTypeUpdatePacket();
  PlanetTypeUpdatePacket(CelestialCoordinate coordinate);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  CelestialCoordinate coordinate;
};

struct PausePacket : PacketBase<PacketType::Pause> {
  PausePacket();
  PausePacket(bool pause, float timescale = 1.0f);

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  bool pause = false;
  float timescale = 1.0f;
};

struct ServerInfoPacket : PacketBase<PacketType::ServerInfo> {
  ServerInfoPacket();
  ServerInfoPacket(uint16_t players, uint16_t maxPlayers);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  uint16_t players;
  uint16_t maxPlayers;
};

struct ClientConnectPacket : PacketBase<PacketType::ClientConnect> {
  ClientConnectPacket();
  ClientConnectPacket(ByteArray assetsDigest, bool allowAssetsMismatch, Uuid playerUuid, String playerName,
      String shipSpecies, WorldChunks shipChunks, ShipUpgrades shipUpgrades, bool introComplete,
      String account, Json info = {});

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  ByteArray assetsDigest;
  bool allowAssetsMismatch;
  Uuid playerUuid;
  String playerName;
  String shipSpecies;
  WorldChunks shipChunks;
  ShipUpgrades shipUpgrades;
  bool introComplete;
  String account;
  Json info;
};

struct ClientDisconnectRequestPacket : PacketBase<PacketType::ClientDisconnectRequest> {
  ClientDisconnectRequestPacket();

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;
};

struct HandshakeResponsePacket : PacketBase<PacketType::HandshakeResponse> {
  HandshakeResponsePacket();
  HandshakeResponsePacket(ByteArray const& passHash);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray passHash;
};

struct PlayerWarpPacket : PacketBase<PacketType::PlayerWarp> {
  PlayerWarpPacket();
  PlayerWarpPacket(WarpAction action, bool deploy);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  WarpAction action;
  bool deploy;
};

struct FlyShipPacket : PacketBase<PacketType::FlyShip> {
  FlyShipPacket();
  FlyShipPacket(Vec3I system, SystemLocation location, Json settings = {});

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  Vec3I system;
  SystemLocation location;
  Json settings;
};

struct ChatSendPacket : PacketBase<PacketType::ChatSend> {
  ChatSendPacket();
  ChatSendPacket(String text, ChatSendMode sendMode);
  ChatSendPacket(String text, ChatSendMode sendMode, JsonObject data);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String text;
  ChatSendMode sendMode;
  JsonObject data;
};

struct CelestialRequestPacket : PacketBase<PacketType::CelestialRequest> {
  CelestialRequestPacket();
  CelestialRequestPacket(List<CelestialRequest> requests);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  List<CelestialRequest> requests;
};

struct ClientContextUpdatePacket : PacketBase<PacketType::ClientContextUpdate> {
  ClientContextUpdatePacket();
  ClientContextUpdatePacket(ByteArray updateData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray updateData;
};

// Sent when a client should initialize themselves on a new world
struct WorldStartPacket : PacketBase<PacketType::WorldStart> {
  WorldStartPacket();

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Json templateData;
  ByteArray skyData;
  ByteArray weatherData;
  Vec2F playerStart;
  Vec2F playerRespawn;
  bool respawnInWorld;
  HashMap<DungeonId, float> dungeonIdGravity;
  HashMap<DungeonId, bool> dungeonIdBreathable;
  StableHashSet<DungeonId> protectedDungeonIds;
  Json worldProperties;
  ConnectionId clientId;
  bool localInterpolationMode;
};

// Sent when a client is leaving a world
struct WorldStopPacket : PacketBase<PacketType::WorldStop> {
  WorldStopPacket();
  WorldStopPacket(String const& reason);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String reason;
};

// Sent when the region data for the client's current world changes
struct WorldLayoutUpdatePacket : PacketBase<PacketType::WorldLayoutUpdate> {
  WorldLayoutUpdatePacket();
  WorldLayoutUpdatePacket(Json const& layoutData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Json layoutData;
};

// Sent when the environment status effect list for the client's current world changes
struct WorldParametersUpdatePacket : PacketBase<PacketType::WorldParametersUpdate> {
  WorldParametersUpdatePacket();
  WorldParametersUpdatePacket(ByteArray const& parametersData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray parametersData;
};

struct CentralStructureUpdatePacket : PacketBase<PacketType::CentralStructureUpdate> {
  CentralStructureUpdatePacket();
  CentralStructureUpdatePacket(Json structureData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Json structureData;
};

struct TileArrayUpdatePacket : PacketBase<PacketType::TileArrayUpdate> {
  typedef MultiArray<NetTile, 2> TileArray;

  TileArrayUpdatePacket();

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I min;
  TileArray array;
};

struct TileUpdatePacket : PacketBase<PacketType::TileUpdate> {
  TileUpdatePacket() {}
  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I position;
  NetTile tile;
};

struct TileLiquidUpdatePacket : PacketBase<PacketType::TileLiquidUpdate> {
  TileLiquidUpdatePacket();
  TileLiquidUpdatePacket(Vec2I const& position, LiquidNetUpdate liquidUpdate);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I position;
  LiquidNetUpdate liquidUpdate;
};

struct TileDamageUpdatePacket : PacketBase<PacketType::TileDamageUpdate> {
  TileDamageUpdatePacket();
  TileDamageUpdatePacket(Vec2I const& position, TileLayer layer, TileDamageStatus const& tileDamage);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I position;
  TileLayer layer;

  TileDamageStatus tileDamage;
};

struct TileModificationFailurePacket : PacketBase<PacketType::TileModificationFailure> {
  TileModificationFailurePacket();
  TileModificationFailurePacket(TileModificationList modifications);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  TileModificationList modifications;
};

struct GiveItemPacket : PacketBase<PacketType::GiveItem> {
  GiveItemPacket();
  GiveItemPacket(ItemDescriptor const& item);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  ItemDescriptor item;
};

struct EnvironmentUpdatePacket : PacketBase<PacketType::EnvironmentUpdate> {
  EnvironmentUpdatePacket();
  EnvironmentUpdatePacket(ByteArray skyDelta, ByteArray weatherDelta);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray skyDelta;
  ByteArray weatherDelta;
};

struct UpdateTileProtectionPacket : PacketBase<PacketType::UpdateTileProtection> {
  UpdateTileProtectionPacket();
  UpdateTileProtectionPacket(DungeonId dungeonId, bool isProtected);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  DungeonId dungeonId;
  bool isProtected;
};

struct SetDungeonGravityPacket : PacketBase<PacketType::SetDungeonGravity> {
  SetDungeonGravityPacket();
  SetDungeonGravityPacket(DungeonId dungeonId, Maybe<float> gravity);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  DungeonId dungeonId;
  Maybe<float> gravity;
};

struct SetDungeonBreathablePacket : PacketBase<PacketType::SetDungeonBreathable> {
  SetDungeonBreathablePacket();
  SetDungeonBreathablePacket(DungeonId dungeonId, Maybe<bool> breathable);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  DungeonId dungeonId;
  Maybe<bool> breathable;
};

struct SetPlayerStartPacket : PacketBase<PacketType::SetPlayerStart> {
  SetPlayerStartPacket();
  SetPlayerStartPacket(Vec2F playerStart, bool respawnInWorld);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  Vec2F playerStart;
  bool respawnInWorld;
};

struct FindUniqueEntityResponsePacket : PacketBase<PacketType::FindUniqueEntityResponse> {
  FindUniqueEntityResponsePacket();
  FindUniqueEntityResponsePacket(String uniqueEntityId, Maybe<Vec2F> entityPosition);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String uniqueEntityId;
  Maybe<Vec2F> entityPosition;
};

struct PongPacket : PacketBase<PacketType::Pong> {
  PongPacket();
  PongPacket(int64_t time);

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  int64_t time = 0;
};

struct ModifyTileListPacket : PacketBase<PacketType::ModifyTileList> {
  ModifyTileListPacket();
  ModifyTileListPacket(TileModificationList modifications, bool allowEntityOverlap);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  TileModificationList modifications;
  bool allowEntityOverlap;
};

struct ReplaceTileListPacket : PacketBase<PacketType::ReplaceTileList> {
  ReplaceTileListPacket();
  ReplaceTileListPacket(TileModificationList modifications, TileDamage tileDamage, bool applyDamage);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  TileModificationList modifications;
  TileDamage tileDamage;
  bool applyDamage;
};

struct DamageTileGroupPacket : PacketBase<PacketType::DamageTileGroup> {
  DamageTileGroupPacket();
  DamageTileGroupPacket(List<Vec2I> tilePositions, TileLayer layer, Vec2F sourcePosition, TileDamage tileDamage, Maybe<EntityId> sourceEntity);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  List<Vec2I> tilePositions;
  TileLayer layer;
  Vec2F sourcePosition;
  TileDamage tileDamage;
  Maybe<EntityId> sourceEntity;
};

struct CollectLiquidPacket : PacketBase<PacketType::CollectLiquid> {
  CollectLiquidPacket();
  CollectLiquidPacket(List<Vec2I> tilePositions, LiquidId liquidId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  List<Vec2I> tilePositions;
  LiquidId liquidId;
};

struct RequestDropPacket : PacketBase<PacketType::RequestDrop> {
  RequestDropPacket();
  RequestDropPacket(EntityId dropEntityId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  EntityId dropEntityId;
};

struct SpawnEntityPacket : PacketBase<PacketType::SpawnEntity> {
  SpawnEntityPacket();
  SpawnEntityPacket(EntityType entityType, ByteArray storeData, ByteArray firstNetState);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  EntityType entityType;
  ByteArray storeData;
  ByteArray firstNetState;
};

struct ConnectWirePacket : PacketBase<PacketType::ConnectWire> {
  ConnectWirePacket();
  ConnectWirePacket(WireConnection outputConnection, WireConnection inputConnection);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  WireConnection outputConnection;
  WireConnection inputConnection;
};

struct DisconnectAllWiresPacket : PacketBase<PacketType::DisconnectAllWires> {
  DisconnectAllWiresPacket();
  DisconnectAllWiresPacket(Vec2I entityPosition, WireNode wireNode);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I entityPosition;
  WireNode wireNode;
};

struct WorldClientStateUpdatePacket : PacketBase<PacketType::WorldClientStateUpdate> {
  WorldClientStateUpdatePacket();
  WorldClientStateUpdatePacket(ByteArray const& worldClientStateDelta);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray worldClientStateDelta;
};

struct FindUniqueEntityPacket : PacketBase<PacketType::FindUniqueEntity> {
  FindUniqueEntityPacket();
  FindUniqueEntityPacket(String uniqueEntityId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String uniqueEntityId;
};

struct WorldStartAcknowledgePacket : PacketBase<PacketType::WorldStartAcknowledge> {
  WorldStartAcknowledgePacket();

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;
};

struct PingPacket : PacketBase<PacketType::Ping> {
  PingPacket();
  PingPacket(int64_t time);

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  int64_t time = 0;
};

struct EntityCreatePacket : PacketBase<PacketType::EntityCreate> {
  EntityCreatePacket();
  EntityCreatePacket(EntityType entityType, ByteArray storeData, ByteArray firstNetState, EntityId entityId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  EntityType entityType;
  ByteArray storeData;
  ByteArray firstNetState;
  EntityId entityId;
};

// All entity deltas will be sent at the same time for the same connection
// where they are master, any entities whose master is from that connection can
// be assumed to have produced a blank delta.
struct EntityUpdateSetPacket : PacketBase<PacketType::EntityUpdateSet> {
  EntityUpdateSetPacket(ConnectionId forConnection = ServerConnectionId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ConnectionId forConnection;
  HashMap<EntityId, ByteArray> deltas;
};

struct EntityDestroyPacket : PacketBase<PacketType::EntityDestroy> {
  EntityDestroyPacket();
  EntityDestroyPacket(EntityId entityId, ByteArray finalNetState, bool death);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  EntityId entityId;
  ByteArray finalNetState;
  // If true, the entity removal is due to death rather simply for example
  // going out of range of the entity monitoring window.
  bool death;
};

struct EntityInteractPacket : PacketBase<PacketType::EntityInteract> {
  EntityInteractPacket();
  EntityInteractPacket(InteractRequest interactRequest, Uuid requestId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  InteractRequest interactRequest;
  Uuid requestId;
};

struct EntityInteractResultPacket : PacketBase<PacketType::EntityInteractResult> {
  EntityInteractResultPacket();
  EntityInteractResultPacket(InteractAction action, Uuid requestId, EntityId sourceEntityId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  InteractAction action;
  Uuid requestId;
  EntityId sourceEntityId;
};

struct HitRequestPacket : PacketBase<PacketType::HitRequest> {
  HitRequestPacket();
  HitRequestPacket(RemoteHitRequest remoteHitRequest);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  RemoteHitRequest remoteHitRequest;
};

struct DamageRequestPacket : PacketBase<PacketType::DamageRequest> {
  DamageRequestPacket();
  DamageRequestPacket(RemoteDamageRequest remoteDamageRequest);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  RemoteDamageRequest remoteDamageRequest;
};

struct DamageNotificationPacket : PacketBase<PacketType::DamageNotification> {
  DamageNotificationPacket();
  DamageNotificationPacket(RemoteDamageNotification remoteDamageNotification);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  RemoteDamageNotification remoteDamageNotification;
};

struct EntityMessagePacket : PacketBase<PacketType::EntityMessage> {
  EntityMessagePacket();
  EntityMessagePacket(Variant<EntityId, String> entityId, String message, JsonArray args, Uuid uuid, ConnectionId fromConnection = ServerConnectionId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  Variant<EntityId, String> entityId;
  String message;
  JsonArray args;
  Uuid uuid;
  ConnectionId fromConnection;
};

struct EntityMessageResponsePacket : PacketBase<PacketType::EntityMessageResponse> {
  EntityMessageResponsePacket();
  EntityMessageResponsePacket(Either<String, Json> response, Uuid uuid);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Either<String, Json> response;
  Uuid uuid;
};

struct UpdateWorldPropertiesPacket : PacketBase<PacketType::UpdateWorldProperties> {
  UpdateWorldPropertiesPacket();
  UpdateWorldPropertiesPacket(JsonObject const& updatedProperties);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  JsonObject updatedProperties;
};

struct StepUpdatePacket : PacketBase<PacketType::StepUpdate> {
  StepUpdatePacket();
  StepUpdatePacket(double remoteTime);

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  double remoteTime;
};

struct SystemWorldStartPacket : PacketBase<PacketType::SystemWorldStart> {
  SystemWorldStartPacket();
  SystemWorldStartPacket(Vec3I location, List<ByteArray> objectStores, List<ByteArray> shipStores, pair<Uuid, SystemLocation> clientShip);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec3I location;
  List<ByteArray> objectStores;
  List<ByteArray> shipStores;
  pair<Uuid, SystemLocation> clientShip;
};

struct SystemWorldUpdatePacket : PacketBase<PacketType::SystemWorldUpdate> {
  SystemWorldUpdatePacket();
  SystemWorldUpdatePacket(HashMap<Uuid, ByteArray> objectUpdates, HashMap<Uuid, ByteArray> shipUpdates);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  HashMap<Uuid, ByteArray> objectUpdates;
  HashMap<Uuid, ByteArray> shipUpdates;
};

struct SystemObjectCreatePacket : PacketBase<PacketType::SystemObjectCreate> {
  SystemObjectCreatePacket();
  SystemObjectCreatePacket(ByteArray objectStore);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray objectStore;
};

struct SystemObjectDestroyPacket : PacketBase<PacketType::SystemObjectDestroy> {
  SystemObjectDestroyPacket();
  SystemObjectDestroyPacket(Uuid objectUuid);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Uuid objectUuid;
};

struct SystemShipCreatePacket : PacketBase<PacketType::SystemShipCreate> {
  SystemShipCreatePacket();
  SystemShipCreatePacket(ByteArray shipStore);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray shipStore;
};

struct SystemShipDestroyPacket : PacketBase<PacketType::SystemShipDestroy> {
  SystemShipDestroyPacket();
  SystemShipDestroyPacket(Uuid shipUuid);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Uuid shipUuid;
};

struct SystemObjectSpawnPacket : PacketBase<PacketType::SystemObjectSpawn> {
  SystemObjectSpawnPacket();
  SystemObjectSpawnPacket(String typeName, Uuid uuid, Maybe<Vec2F> position, JsonObject parameters);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String typeName;
  Uuid uuid;
  Maybe<Vec2F> position;
  JsonObject parameters;
};

struct UpdateWorldTemplatePacket : PacketBase<PacketType::UpdateWorldTemplate> {
  UpdateWorldTemplatePacket();
  UpdateWorldTemplatePacket(Json templateData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Json templateData;
};

struct ClientCustomWorldRequest : PacketBase<PacketType::ClientCustomWorldRequest> {
  ClientCustomWorldRequest();
  ClientCustomWorldRequest(String name);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String name;
};

struct ClientCustomWorldResponse : PacketBase<PacketType::ClientCustomWorldResponse> {
  ClientCustomWorldResponse();
  ClientCustomWorldResponse(String name, WorldChunks chunks);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String name;
  WorldChunks chunks;
};

struct ClientCustomWorldCreate : PacketBase<PacketType::ClientCustomWorldCreate> {
  ClientCustomWorldCreate();
  ClientCustomWorldCreate(String name, Json templateData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String name;
  Json templateData;
};

// wrapper packet for client world threads
struct ClientSubWorldPackets : PacketBase<PacketType::ClientSubWorldPackets> {
  ClientSubWorldPackets();
  ClientSubWorldPackets(ClientSubWorldId const& subWorldId, List<PacketPtr> packets);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ClientSubWorldId subWorldId;
  List<PacketPtr> packets;
};

struct ClientSubWorldRequest : PacketBase<PacketType::ClientSubWorldRequest> {
  ClientSubWorldRequest();
  ClientSubWorldRequest(ClientSubWorldId const& subWorldId, WorldId worldId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ClientSubWorldId subWorldId;
  WorldId worldId;
};

struct ClientSubWorldReject : PacketBase<PacketType::ClientSubWorldReject> {
  ClientSubWorldReject();
  ClientSubWorldReject(ClientSubWorldId const& subWorldId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ClientSubWorldId subWorldId;
};

struct NotifyWorldLoad : PacketBase<PacketType::NotifyWorldLoad> {
  NotifyWorldLoad();
  NotifyWorldLoad(WorldId worldId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  WorldId worldId;
};

struct LogMapUpdate : PacketBase<PacketType::LogMapUpdate> {
  LogMapUpdate();
  LogMapUpdate(Map<String,String> map);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Map<String,String> map;
};

struct UniverseMessage : PacketBase<PacketType::UniverseMessage> {
  UniverseMessage();
  UniverseMessage(ConnectionId connection, String message, JsonArray args, Uuid uuid, ConnectionId fromConnection = ServerConnectionId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  ConnectionId connection;
  String message;
  JsonArray args;
  Uuid uuid;
  ConnectionId fromConnection;
};

struct UniverseMessageResponse : PacketBase<PacketType::UniverseMessageResponse> {
  UniverseMessageResponse();
  UniverseMessageResponse(Either<String,Json> response, Uuid uuid);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Either<String, Json> response;
  Uuid uuid;
};
}

export module star.net_packets;

export namespace Star {
  using ::Star::Packet;
  using ::Star::PacketPtr;
  using ::Star::PacketConstPtr;
  using ::Star::PacketWeakPtr;
  using ::Star::PacketConstWeakPtr;
  using ::Star::PacketUPtr;
  using ::Star::PacketConstUPtr;
  using ::Star::StarPacketExceptionTag;
  using ::Star::StarPacketException;
  using ::Star::StarProtocolVersion;
  using ::Star::PacketType;
  using ::Star::PacketTypeNames;
  using ::Star::NetCompressionMode;
  using ::Star::NetCompressionModeNames;
  using ::Star::PacketCompressionMode;
  using ::Star::createPacket;
  using ::Star::PacketBase;
  using ::Star::ProtocolRequestPacket;
  using ::Star::ProtocolResponsePacket;
  using ::Star::ServerDisconnectPacket;
  using ::Star::ConnectSuccessPacket;
  using ::Star::ConnectFailurePacket;
  using ::Star::HandshakeChallengePacket;
  using ::Star::ChatReceivePacket;
  using ::Star::UniverseTimeUpdatePacket;
  using ::Star::CelestialResponsePacket;
  using ::Star::PlayerWarpResultPacket;
  using ::Star::PlanetTypeUpdatePacket;
  using ::Star::PausePacket;
  using ::Star::ServerInfoPacket;
  using ::Star::ClientConnectPacket;
  using ::Star::ClientDisconnectRequestPacket;
  using ::Star::HandshakeResponsePacket;
  using ::Star::PlayerWarpPacket;
  using ::Star::FlyShipPacket;
  using ::Star::ChatSendPacket;
  using ::Star::CelestialRequestPacket;
  using ::Star::ClientContextUpdatePacket;
  using ::Star::WorldStartPacket;
  using ::Star::WorldStopPacket;
  using ::Star::WorldLayoutUpdatePacket;
  using ::Star::WorldParametersUpdatePacket;
  using ::Star::CentralStructureUpdatePacket;
  using ::Star::TileArrayUpdatePacket;
  using ::Star::TileUpdatePacket;
  using ::Star::TileLiquidUpdatePacket;
  using ::Star::TileDamageUpdatePacket;
  using ::Star::TileModificationFailurePacket;
  using ::Star::GiveItemPacket;
  using ::Star::EnvironmentUpdatePacket;
  using ::Star::UpdateTileProtectionPacket;
  using ::Star::SetDungeonGravityPacket;
  using ::Star::SetDungeonBreathablePacket;
  using ::Star::SetPlayerStartPacket;
  using ::Star::FindUniqueEntityResponsePacket;
  using ::Star::PongPacket;
  using ::Star::ModifyTileListPacket;
  using ::Star::ReplaceTileListPacket;
  using ::Star::DamageTileGroupPacket;
  using ::Star::CollectLiquidPacket;
  using ::Star::RequestDropPacket;
  using ::Star::SpawnEntityPacket;
  using ::Star::ConnectWirePacket;
  using ::Star::DisconnectAllWiresPacket;
  using ::Star::WorldClientStateUpdatePacket;
  using ::Star::FindUniqueEntityPacket;
  using ::Star::WorldStartAcknowledgePacket;
  using ::Star::PingPacket;
  using ::Star::EntityCreatePacket;
  using ::Star::EntityUpdateSetPacket;
  using ::Star::EntityDestroyPacket;
  using ::Star::EntityInteractPacket;
  using ::Star::EntityInteractResultPacket;
  using ::Star::HitRequestPacket;
  using ::Star::DamageRequestPacket;
  using ::Star::DamageNotificationPacket;
  using ::Star::EntityMessagePacket;
  using ::Star::EntityMessageResponsePacket;
  using ::Star::UpdateWorldPropertiesPacket;
  using ::Star::StepUpdatePacket;
  using ::Star::SystemWorldStartPacket;
  using ::Star::SystemWorldUpdatePacket;
  using ::Star::SystemObjectCreatePacket;
  using ::Star::SystemObjectDestroyPacket;
  using ::Star::SystemShipCreatePacket;
  using ::Star::SystemShipDestroyPacket;
  using ::Star::SystemObjectSpawnPacket;
  using ::Star::UpdateWorldTemplatePacket;
  using ::Star::ClientCustomWorldRequest;
  using ::Star::ClientCustomWorldResponse;
  using ::Star::ClientCustomWorldCreate;
  using ::Star::ClientSubWorldPackets;
  using ::Star::ClientSubWorldRequest;
  using ::Star::ClientSubWorldReject;
  using ::Star::NotifyWorldLoad;
  using ::Star::LogMapUpdate;
  using ::Star::UniverseMessage;
  using ::Star::UniverseMessageResponse;
}
