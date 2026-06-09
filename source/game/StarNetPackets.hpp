#pragma once

#include "StarDataStream.hpp"
#include "StarWorldTiles.hpp"
#include "StarItemDescriptor.hpp"
#include "StarCelestialDatabase.hpp"
#include "StarDamageManager.hpp"
#include "StarChatTypes.hpp"
#include "StarUuid.hpp"
#include "StarTileModification.hpp"
#include "StarEntity.hpp"
#include "StarInteractionTypes.hpp"
#include "StarWarping.hpp"
#include "StarWiring.hpp"
#include "StarClientContext.hpp"
#include "StarSystemWorld.hpp"
#include "StarNetCompatibility.hpp"

namespace Star {

struct Packet;
using PacketPtr = SharedPtr<Packet>;

struct StarPacketExceptionTag { static constexpr char const* typeName = "StarPacketException"; };
using StarPacketException = TypedException<IOException, StarPacketExceptionTag>;

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
  UpdateWorldTemplate
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

template <typename Derived, PacketType PT>
struct AutoPacket : PacketBase<PT> {
  void read(DataStream& ds) final override {
    std::apply([&ds, this](auto... ptrs) {
      ((ds >> (static_cast<Derived&>(*this).*ptrs)), ...);
    }, Derived::serializableFields());
  }
  void write(DataStream& ds) const final override {
    std::apply([&ds, this](auto... ptrs) {
      ((ds << (static_cast<Derived const&>(*this).*ptrs)), ...);
    }, Derived::serializableFields());
  }
};

struct ProtocolRequestPacket : AutoPacket<ProtocolRequestPacket, PacketType::ProtocolRequest> {
  ProtocolRequestPacket();
  explicit ProtocolRequestPacket(VersionNumber requestProtocolVersion);

  VersionNumber requestProtocolVersion;

  static constexpr auto serializableFields() {
    return std::tuple{&ProtocolRequestPacket::requestProtocolVersion};
  }
};

struct ProtocolResponsePacket : PacketBase<PacketType::ProtocolResponse> {
  explicit ProtocolResponsePacket(bool allowed = false, Json info = {});

  void read(DataStream& ds) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  bool allowed;
  Json info;
};

struct ServerDisconnectPacket : AutoPacket<ServerDisconnectPacket, PacketType::ServerDisconnect> {
  ServerDisconnectPacket();
  explicit ServerDisconnectPacket(String reason);

  String reason;

  static constexpr auto serializableFields() {
    return std::tuple{&ServerDisconnectPacket::reason};
  }
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

struct ConnectFailurePacket : AutoPacket<ConnectFailurePacket, PacketType::ConnectFailure> {
  ConnectFailurePacket();
  explicit ConnectFailurePacket(String reason);

  String reason;

  static constexpr auto serializableFields() {
    return std::tuple{&ConnectFailurePacket::reason};
  }
};

struct HandshakeChallengePacket : AutoPacket<HandshakeChallengePacket, PacketType::HandshakeChallenge> {
  HandshakeChallengePacket();
  explicit HandshakeChallengePacket(ByteArray const& passwordSalt);

  ByteArray passwordSalt;

  static constexpr auto serializableFields() {
    return std::tuple{&HandshakeChallengePacket::passwordSalt};
  }
};

struct ChatReceivePacket : AutoPacket<ChatReceivePacket, PacketType::ChatReceive> {
  ChatReceivePacket();
  explicit ChatReceivePacket(ChatReceivedMessage receivedMessage);

  void readJson(Json const& json) override;
  Json writeJson() const override;

  ChatReceivedMessage receivedMessage;

  static constexpr auto serializableFields() {
    return std::tuple{&ChatReceivePacket::receivedMessage};
  }
};

struct UniverseTimeUpdatePacket : PacketBase<PacketType::UniverseTimeUpdate> {
  UniverseTimeUpdatePacket();
  explicit UniverseTimeUpdatePacket(double universeTime);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  double universeTime;
  float timescale;
};

struct CelestialResponsePacket : AutoPacket<CelestialResponsePacket, PacketType::CelestialResponse> {
  CelestialResponsePacket();
  explicit CelestialResponsePacket(List<CelestialResponse> responses);

  List<CelestialResponse> responses;

  static constexpr auto serializableFields() {
    return std::tuple{&CelestialResponsePacket::responses};
  }
};

struct PlayerWarpResultPacket : AutoPacket<PlayerWarpResultPacket, PacketType::PlayerWarpResult> {
  PlayerWarpResultPacket();
  PlayerWarpResultPacket(bool success, WarpAction warpAction, bool warpActionInvalid);

  bool success;
  WarpAction warpAction;
  bool warpActionInvalid;

  static constexpr auto serializableFields() {
    return std::tuple{&PlayerWarpResultPacket::success, &PlayerWarpResultPacket::warpAction, &PlayerWarpResultPacket::warpActionInvalid};
  }
};

struct PlanetTypeUpdatePacket : AutoPacket<PlanetTypeUpdatePacket, PacketType::PlanetTypeUpdate> {
  PlanetTypeUpdatePacket();
  explicit PlanetTypeUpdatePacket(CelestialCoordinate coordinate);

  CelestialCoordinate coordinate;

  static constexpr auto serializableFields() {
    return std::tuple{&PlanetTypeUpdatePacket::coordinate};
  }
};

struct PausePacket : PacketBase<PacketType::Pause> {
  PausePacket();
  explicit PausePacket(bool pause, float timescale = 1.0f);

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  bool pause = false;
  float timescale = 1.0f;
};

struct ServerInfoPacket : AutoPacket<ServerInfoPacket, PacketType::ServerInfo> {
  ServerInfoPacket();
  ServerInfoPacket(uint16_t players, uint16_t maxPlayers);

  void readJson(Json const& json) override;
  Json writeJson() const override;

  uint16_t players;
  uint16_t maxPlayers;

  static constexpr auto serializableFields() {
    return std::tuple{&ServerInfoPacket::players, &ServerInfoPacket::maxPlayers};
  }
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

struct HandshakeResponsePacket : AutoPacket<HandshakeResponsePacket, PacketType::HandshakeResponse> {
  HandshakeResponsePacket();
  explicit HandshakeResponsePacket(ByteArray const& passHash);

  ByteArray passHash;

  static constexpr auto serializableFields() {
    return std::tuple{&HandshakeResponsePacket::passHash};
  }
};

struct PlayerWarpPacket : AutoPacket<PlayerWarpPacket, PacketType::PlayerWarp> {
  PlayerWarpPacket();
  PlayerWarpPacket(WarpAction action, bool deploy);

  WarpAction action;
  bool deploy;

  static constexpr auto serializableFields() {
    return std::tuple{&PlayerWarpPacket::action, &PlayerWarpPacket::deploy};
  }
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

struct CelestialRequestPacket : AutoPacket<CelestialRequestPacket, PacketType::CelestialRequest> {
  CelestialRequestPacket();
  explicit CelestialRequestPacket(List<CelestialRequest> requests);

  List<CelestialRequest> requests;

  static constexpr auto serializableFields() {
    return std::tuple{&CelestialRequestPacket::requests};
  }
};

struct ClientContextUpdatePacket : AutoPacket<ClientContextUpdatePacket, PacketType::ClientContextUpdate> {
  ClientContextUpdatePacket();
  explicit ClientContextUpdatePacket(ByteArray updateData);

  ByteArray updateData;

  static constexpr auto serializableFields() {
    return std::tuple{&ClientContextUpdatePacket::updateData};
  }
};

// Sent when a client should initialize themselves on a new world
struct WorldStartPacket : AutoPacket<WorldStartPacket, PacketType::WorldStart> {
  WorldStartPacket();

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

  static constexpr auto serializableFields() {
    return std::tuple{&WorldStartPacket::templateData, &WorldStartPacket::skyData,
      &WorldStartPacket::weatherData, &WorldStartPacket::playerStart,
      &WorldStartPacket::playerRespawn, &WorldStartPacket::respawnInWorld,
      &WorldStartPacket::dungeonIdGravity, &WorldStartPacket::dungeonIdBreathable,
      &WorldStartPacket::protectedDungeonIds, &WorldStartPacket::worldProperties,
      &WorldStartPacket::clientId, &WorldStartPacket::localInterpolationMode};
  }
};

// Sent when a client is leaving a world
struct WorldStopPacket : AutoPacket<WorldStopPacket, PacketType::WorldStop> {
  WorldStopPacket();
  explicit WorldStopPacket(String const& reason);

  String reason;

  static constexpr auto serializableFields() {
    return std::tuple{&WorldStopPacket::reason};
  }
};

// Sent when the region data for the client's current world changes
struct WorldLayoutUpdatePacket : AutoPacket<WorldLayoutUpdatePacket, PacketType::WorldLayoutUpdate> {
  WorldLayoutUpdatePacket();
  explicit WorldLayoutUpdatePacket(Json const& layoutData);

  Json layoutData;

  static constexpr auto serializableFields() {
    return std::tuple{&WorldLayoutUpdatePacket::layoutData};
  }
};

// Sent when the environment status effect list for the client's current world changes
struct WorldParametersUpdatePacket : AutoPacket<WorldParametersUpdatePacket, PacketType::WorldParametersUpdate> {
  WorldParametersUpdatePacket();
  explicit WorldParametersUpdatePacket(ByteArray const& parametersData);

  ByteArray parametersData;

  static constexpr auto serializableFields() {
    return std::tuple{&WorldParametersUpdatePacket::parametersData};
  }
};

struct CentralStructureUpdatePacket : AutoPacket<CentralStructureUpdatePacket, PacketType::CentralStructureUpdate> {
  CentralStructureUpdatePacket();
  explicit CentralStructureUpdatePacket(Json structureData);

  Json structureData;

  static constexpr auto serializableFields() {
    return std::tuple{&CentralStructureUpdatePacket::structureData};
  }
};

struct TileArrayUpdatePacket : PacketBase<PacketType::TileArrayUpdate> {
  using TileArray = MultiArray<NetTile, 2>;

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

struct TileDamageUpdatePacket : AutoPacket<TileDamageUpdatePacket, PacketType::TileDamageUpdate> {
  TileDamageUpdatePacket();
  TileDamageUpdatePacket(Vec2I const& position, TileLayer layer, TileDamageStatus const& tileDamage);

  Vec2I position;
  TileLayer layer;
  TileDamageStatus tileDamage;

  static constexpr auto serializableFields() {
    return std::tuple{&TileDamageUpdatePacket::position, &TileDamageUpdatePacket::layer, &TileDamageUpdatePacket::tileDamage};
  }
};

struct TileModificationFailurePacket : PacketBase<PacketType::TileModificationFailure> {
  TileModificationFailurePacket();
  explicit TileModificationFailurePacket(TileModificationList modifications);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  TileModificationList modifications;
};

struct GiveItemPacket : AutoPacket<GiveItemPacket, PacketType::GiveItem> {
  GiveItemPacket();
  explicit GiveItemPacket(ItemDescriptor const& item);

  void readJson(Json const& json) override;
  Json writeJson() const override;

  ItemDescriptor item;

  static constexpr auto serializableFields() {
    return std::tuple{&GiveItemPacket::item};
  }
};

struct EnvironmentUpdatePacket : AutoPacket<EnvironmentUpdatePacket, PacketType::EnvironmentUpdate> {
  EnvironmentUpdatePacket();
  EnvironmentUpdatePacket(ByteArray skyDelta, ByteArray weatherDelta);

  ByteArray skyDelta;
  ByteArray weatherDelta;

  static constexpr auto serializableFields() {
    return std::tuple{&EnvironmentUpdatePacket::skyDelta, &EnvironmentUpdatePacket::weatherDelta};
  }
};

struct UpdateTileProtectionPacket : AutoPacket<UpdateTileProtectionPacket, PacketType::UpdateTileProtection> {
  UpdateTileProtectionPacket();
  UpdateTileProtectionPacket(DungeonId dungeonId, bool isProtected);

  void readJson(Json const& json) override;
  Json writeJson() const override;

  DungeonId dungeonId;
  bool isProtected;

  static constexpr auto serializableFields() {
    return std::tuple{&UpdateTileProtectionPacket::dungeonId, &UpdateTileProtectionPacket::isProtected};
  }
};

struct SetDungeonGravityPacket : AutoPacket<SetDungeonGravityPacket, PacketType::SetDungeonGravity> {
  SetDungeonGravityPacket();
  SetDungeonGravityPacket(DungeonId dungeonId, Maybe<float> gravity);

  void readJson(Json const& json) override;
  Json writeJson() const override;

  DungeonId dungeonId;
  Maybe<float> gravity;

  static constexpr auto serializableFields() {
    return std::tuple{&SetDungeonGravityPacket::dungeonId, &SetDungeonGravityPacket::gravity};
  }
};

struct SetDungeonBreathablePacket : AutoPacket<SetDungeonBreathablePacket, PacketType::SetDungeonBreathable> {
  SetDungeonBreathablePacket();
  SetDungeonBreathablePacket(DungeonId dungeonId, Maybe<bool> breathable);

  void readJson(Json const& json) override;
  Json writeJson() const override;

  DungeonId dungeonId;
  Maybe<bool> breathable;

  static constexpr auto serializableFields() {
    return std::tuple{&SetDungeonBreathablePacket::dungeonId, &SetDungeonBreathablePacket::breathable};
  }
};

struct SetPlayerStartPacket : AutoPacket<SetPlayerStartPacket, PacketType::SetPlayerStart> {
  SetPlayerStartPacket();
  SetPlayerStartPacket(Vec2F playerStart, bool respawnInWorld);

  void readJson(Json const& json) override;
  Json writeJson() const override;

  Vec2F playerStart;
  bool respawnInWorld;

  static constexpr auto serializableFields() {
    return std::tuple{&SetPlayerStartPacket::playerStart, &SetPlayerStartPacket::respawnInWorld};
  }
};

struct FindUniqueEntityResponsePacket : AutoPacket<FindUniqueEntityResponsePacket, PacketType::FindUniqueEntityResponse> {
  FindUniqueEntityResponsePacket();
  FindUniqueEntityResponsePacket(String uniqueEntityId, Maybe<Vec2F> entityPosition);

  String uniqueEntityId;
  Maybe<Vec2F> entityPosition;

  static constexpr auto serializableFields() {
    return std::tuple{&FindUniqueEntityResponsePacket::uniqueEntityId, &FindUniqueEntityResponsePacket::entityPosition};
  }
};

struct PongPacket : PacketBase<PacketType::Pong> {
  PongPacket();
  explicit PongPacket(int64_t time);

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
  explicit RequestDropPacket(EntityId dropEntityId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  EntityId dropEntityId;
};

struct SpawnEntityPacket : AutoPacket<SpawnEntityPacket, PacketType::SpawnEntity> {
  SpawnEntityPacket();
  SpawnEntityPacket(EntityType entityType, ByteArray storeData, ByteArray firstNetState);

  EntityType entityType;
  ByteArray storeData;
  ByteArray firstNetState;

  static constexpr auto serializableFields() {
    return std::tuple{&SpawnEntityPacket::entityType, &SpawnEntityPacket::storeData, &SpawnEntityPacket::firstNetState};
  }
};

struct ConnectWirePacket : AutoPacket<ConnectWirePacket, PacketType::ConnectWire> {
  ConnectWirePacket();
  ConnectWirePacket(WireConnection outputConnection, WireConnection inputConnection);

  WireConnection outputConnection;
  WireConnection inputConnection;

  static constexpr auto serializableFields() {
    return std::tuple{&ConnectWirePacket::outputConnection, &ConnectWirePacket::inputConnection};
  }
};

struct DisconnectAllWiresPacket : PacketBase<PacketType::DisconnectAllWires> {
  DisconnectAllWiresPacket();
  DisconnectAllWiresPacket(Vec2I entityPosition, WireNode wireNode);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I entityPosition;
  WireNode wireNode;
};

struct WorldClientStateUpdatePacket : AutoPacket<WorldClientStateUpdatePacket, PacketType::WorldClientStateUpdate> {
  WorldClientStateUpdatePacket();
  explicit WorldClientStateUpdatePacket(ByteArray const& worldClientStateDelta);

  ByteArray worldClientStateDelta;

  static constexpr auto serializableFields() {
    return std::tuple{&WorldClientStateUpdatePacket::worldClientStateDelta};
  }
};

struct FindUniqueEntityPacket : AutoPacket<FindUniqueEntityPacket, PacketType::FindUniqueEntity> {
  FindUniqueEntityPacket();
  explicit FindUniqueEntityPacket(String uniqueEntityId);

  String uniqueEntityId;

  static constexpr auto serializableFields() {
    return std::tuple{&FindUniqueEntityPacket::uniqueEntityId};
  }
};

struct WorldStartAcknowledgePacket : PacketBase<PacketType::WorldStartAcknowledge> {
  WorldStartAcknowledgePacket();

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;
};

struct PingPacket : PacketBase<PacketType::Ping> {
  PingPacket();
  explicit PingPacket(int64_t time);

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
  explicit EntityUpdateSetPacket(ConnectionId forConnection = ServerConnectionId);

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

struct EntityInteractPacket : AutoPacket<EntityInteractPacket, PacketType::EntityInteract> {
  EntityInteractPacket();
  EntityInteractPacket(InteractRequest interactRequest, Uuid requestId);

  InteractRequest interactRequest;
  Uuid requestId;

  static constexpr auto serializableFields() {
    return std::tuple{&EntityInteractPacket::interactRequest, &EntityInteractPacket::requestId};
  }
};

struct EntityInteractResultPacket : AutoPacket<EntityInteractResultPacket, PacketType::EntityInteractResult> {
  EntityInteractResultPacket();
  EntityInteractResultPacket(InteractAction action, Uuid requestId, EntityId sourceEntityId);

  InteractAction action;
  Uuid requestId;
  EntityId sourceEntityId;

  static constexpr auto serializableFields() {
    return std::tuple{&EntityInteractResultPacket::action, &EntityInteractResultPacket::requestId, &EntityInteractResultPacket::sourceEntityId};
  }
};

struct HitRequestPacket : AutoPacket<HitRequestPacket, PacketType::HitRequest> {
  HitRequestPacket();
  explicit HitRequestPacket(RemoteHitRequest remoteHitRequest);

  RemoteHitRequest remoteHitRequest;

  static constexpr auto serializableFields() {
    return std::tuple{&HitRequestPacket::remoteHitRequest};
  }
};

struct DamageRequestPacket : AutoPacket<DamageRequestPacket, PacketType::DamageRequest> {
  DamageRequestPacket();
  explicit DamageRequestPacket(RemoteDamageRequest remoteDamageRequest);

  RemoteDamageRequest remoteDamageRequest;

  static constexpr auto serializableFields() {
    return std::tuple{&DamageRequestPacket::remoteDamageRequest};
  }
};

struct DamageNotificationPacket : AutoPacket<DamageNotificationPacket, PacketType::DamageNotification> {
  DamageNotificationPacket();
  explicit DamageNotificationPacket(RemoteDamageNotification remoteDamageNotification);

  RemoteDamageNotification remoteDamageNotification;

  static constexpr auto serializableFields() {
    return std::tuple{&DamageNotificationPacket::remoteDamageNotification};
  }
};

struct EntityMessagePacket : AutoPacket<EntityMessagePacket, PacketType::EntityMessage> {
  EntityMessagePacket();
  EntityMessagePacket(Variant<EntityId, String> entityId, String message, JsonArray args, Uuid uuid, ConnectionId fromConnection = ServerConnectionId);

  void readJson(Json const& json) override;
  Json writeJson() const override;

  Variant<EntityId, String> entityId;
  String message;
  JsonArray args;
  Uuid uuid;
  ConnectionId fromConnection;

  static constexpr auto serializableFields() {
    return std::tuple{&EntityMessagePacket::entityId, &EntityMessagePacket::message,
      &EntityMessagePacket::args, &EntityMessagePacket::uuid, &EntityMessagePacket::fromConnection};
  }
};

struct EntityMessageResponsePacket : AutoPacket<EntityMessageResponsePacket, PacketType::EntityMessageResponse> {
  EntityMessageResponsePacket();
  EntityMessageResponsePacket(Either<String, Json> response, Uuid uuid);

  Either<String, Json> response;
  Uuid uuid;

  static constexpr auto serializableFields() {
    return std::tuple{&EntityMessageResponsePacket::response, &EntityMessageResponsePacket::uuid};
  }
};

struct UpdateWorldPropertiesPacket : PacketBase<PacketType::UpdateWorldProperties> {
  UpdateWorldPropertiesPacket();
  explicit UpdateWorldPropertiesPacket(JsonObject const& updatedProperties);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  JsonObject updatedProperties;
};

struct StepUpdatePacket : PacketBase<PacketType::StepUpdate> {
  StepUpdatePacket();
  explicit StepUpdatePacket(double remoteTime);

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  double remoteTime;
};

struct SystemWorldStartPacket : AutoPacket<SystemWorldStartPacket, PacketType::SystemWorldStart> {
  SystemWorldStartPacket();
  SystemWorldStartPacket(Vec3I location, List<ByteArray> objectStores, List<ByteArray> shipStores, pair<Uuid, SystemLocation> clientShip);

  Vec3I location;
  List<ByteArray> objectStores;
  List<ByteArray> shipStores;
  pair<Uuid, SystemLocation> clientShip;

  static constexpr auto serializableFields() {
    return std::tuple{&SystemWorldStartPacket::location, &SystemWorldStartPacket::objectStores,
      &SystemWorldStartPacket::shipStores, &SystemWorldStartPacket::clientShip};
  }
};

struct SystemWorldUpdatePacket : AutoPacket<SystemWorldUpdatePacket, PacketType::SystemWorldUpdate> {
  SystemWorldUpdatePacket();
  SystemWorldUpdatePacket(HashMap<Uuid, ByteArray> objectUpdates, HashMap<Uuid, ByteArray> shipUpdates);

  HashMap<Uuid, ByteArray> objectUpdates;
  HashMap<Uuid, ByteArray> shipUpdates;

  static constexpr auto serializableFields() {
    return std::tuple{&SystemWorldUpdatePacket::objectUpdates, &SystemWorldUpdatePacket::shipUpdates};
  }
};

struct SystemObjectCreatePacket : AutoPacket<SystemObjectCreatePacket, PacketType::SystemObjectCreate> {
  SystemObjectCreatePacket();
  explicit SystemObjectCreatePacket(ByteArray objectStore);

  ByteArray objectStore;

  static constexpr auto serializableFields() {
    return std::tuple{&SystemObjectCreatePacket::objectStore};
  }
};

struct SystemObjectDestroyPacket : AutoPacket<SystemObjectDestroyPacket, PacketType::SystemObjectDestroy> {
  SystemObjectDestroyPacket();
  explicit SystemObjectDestroyPacket(Uuid objectUuid);

  Uuid objectUuid;

  static constexpr auto serializableFields() {
    return std::tuple{&SystemObjectDestroyPacket::objectUuid};
  }
};

struct SystemShipCreatePacket : AutoPacket<SystemShipCreatePacket, PacketType::SystemShipCreate> {
  SystemShipCreatePacket();
  explicit SystemShipCreatePacket(ByteArray shipStore);

  ByteArray shipStore;

  static constexpr auto serializableFields() {
    return std::tuple{&SystemShipCreatePacket::shipStore};
  }
};

struct SystemShipDestroyPacket : AutoPacket<SystemShipDestroyPacket, PacketType::SystemShipDestroy> {
  SystemShipDestroyPacket();
  explicit SystemShipDestroyPacket(Uuid shipUuid);

  Uuid shipUuid;

  static constexpr auto serializableFields() {
    return std::tuple{&SystemShipDestroyPacket::shipUuid};
  }
};

struct SystemObjectSpawnPacket : AutoPacket<SystemObjectSpawnPacket, PacketType::SystemObjectSpawn> {
  SystemObjectSpawnPacket();
  SystemObjectSpawnPacket(String typeName, Uuid uuid, Maybe<Vec2F> position, JsonObject parameters);

  String typeName;
  Uuid uuid;
  Maybe<Vec2F> position;
  JsonObject parameters;

  static constexpr auto serializableFields() {
    return std::tuple{&SystemObjectSpawnPacket::typeName, &SystemObjectSpawnPacket::uuid,
      &SystemObjectSpawnPacket::position, &SystemObjectSpawnPacket::parameters};
  }
};

struct UpdateWorldTemplatePacket : AutoPacket<UpdateWorldTemplatePacket, PacketType::UpdateWorldTemplate> {
  UpdateWorldTemplatePacket();
  explicit UpdateWorldTemplatePacket(Json templateData);

  Json templateData;

  static constexpr auto serializableFields() {
    return std::tuple{&UpdateWorldTemplatePacket::templateData};
  }
};
}
