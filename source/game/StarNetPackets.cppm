module;
import star.xx_hash;
import star.json;
import star.id_map;
import star.data_stream;
import star.list;
import star.bi_map;
import star.multi_array;
import star.game_types;
import star.vector;
import star.string;
import star.map;
import star.format;
import star.version;
import star.ordered_map;
import star.either;
import star.color;
import star.random;
import star.weighted_pool;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.casting;
import star.strong_typedef;
import star.net_compatibility;
import std;
import star.type_identity;
import star.set;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.data_stream_devices;
import star.btree_database;
import star.ordered_set;
import star.rpc_promise;

import star.collision_block;
import star.liquid_types;
import star.tile_damage;
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
import star.lua_root;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;

import star.byte_array;
namespace Star {

struct StarPacketExceptionTag {
  static constexpr char const* name() { return "StarPacketException"; }
};
using StarPacketException = StarError<StarPacketExceptionTag, IOException>;

extern VersionNumber const StarProtocolVersion;

// Packet types sent between the client and server over a NetSocket.  Does not
// correspond to actual packets, simply logical portions of NetSocket data.
enum class PacketType : std::uint8_t {
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

enum class NetCompressionMode : std::uint8_t {
  None,
  Zstd
};
extern EnumMap<NetCompressionMode> const NetCompressionModeNames;

enum class PacketCompressionMode : std::uint8_t {
  Disabled,
  Automatic,
  Enabled
};

struct Packet {
  virtual void const* queryType(TypeKey key) const noexcept { return queryTypeOf<Packet>(this, key); }
  virtual std::string_view dynamicTypeName() const noexcept { return Star::typeName<Packet>(); }

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

// Stages a packet for wire decoding; identity fields are initially unfilled.
std::shared_ptr<Packet> createPacket(PacketType type);
// Constructs a raw outgoing packet, generating identity fields before JSON load.
std::shared_ptr<Packet> createPacket(RandomService& random, PacketType type, std::optional<Json> const& args);

template <PacketType PacketT>
struct PacketBase : public Packet {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<PacketBase<PacketT>, Packet>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<PacketBase<PacketT>>(); }

  static PacketType const Type = PacketT;

  PacketType type() const override { return Type; }
};

struct ProtocolRequestPacket : PacketBase<PacketType::ProtocolRequest> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ProtocolRequestPacket, PacketBase<PacketType::ProtocolRequest>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ProtocolRequestPacket>(); }

  ProtocolRequestPacket();
  ProtocolRequestPacket(VersionNumber requestProtocolVersion);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  VersionNumber requestProtocolVersion;
};

struct ProtocolResponsePacket : PacketBase<PacketType::ProtocolResponse> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ProtocolResponsePacket, PacketBase<PacketType::ProtocolResponse>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ProtocolResponsePacket>(); }

  ProtocolResponsePacket();
  ProtocolResponsePacket(bool allowed, Json info = {});

  void read(DataStream& ds) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  bool allowed;
  Json info;
};

struct ServerDisconnectPacket : PacketBase<PacketType::ServerDisconnect> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ServerDisconnectPacket, PacketBase<PacketType::ServerDisconnect>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ServerDisconnectPacket>(); }

  ServerDisconnectPacket();
  ServerDisconnectPacket(String reason);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String reason;
};

struct ConnectSuccessPacket : PacketBase<PacketType::ConnectSuccess> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ConnectSuccessPacket, PacketBase<PacketType::ConnectSuccess>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ConnectSuccessPacket>(); }

  ConnectSuccessPacket();
  ConnectSuccessPacket(ConnectionId clientId, Uuid serverUuid, CelestialBaseInformation celestialInformation);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ConnectionId clientId;
  Uuid serverUuid;
  CelestialBaseInformation celestialInformation;
};

struct ConnectFailurePacket : PacketBase<PacketType::ConnectFailure> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ConnectFailurePacket, PacketBase<PacketType::ConnectFailure>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ConnectFailurePacket>(); }

  ConnectFailurePacket();
  ConnectFailurePacket(String reason);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String reason;
};

struct HandshakeChallengePacket : PacketBase<PacketType::HandshakeChallenge> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<HandshakeChallengePacket, PacketBase<PacketType::HandshakeChallenge>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<HandshakeChallengePacket>(); }

  HandshakeChallengePacket();
  HandshakeChallengePacket(ByteArray const& passwordSalt);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray passwordSalt;
};

struct ChatReceivePacket : PacketBase<PacketType::ChatReceive> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ChatReceivePacket, PacketBase<PacketType::ChatReceive>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ChatReceivePacket>(); }

  ChatReceivePacket();
  ChatReceivePacket(ChatReceivedMessage receivedMessage);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  ChatReceivedMessage receivedMessage;
};

struct UniverseTimeUpdatePacket : PacketBase<PacketType::UniverseTimeUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<UniverseTimeUpdatePacket, PacketBase<PacketType::UniverseTimeUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<UniverseTimeUpdatePacket>(); }

  UniverseTimeUpdatePacket();
  UniverseTimeUpdatePacket(double universeTime);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  double universeTime;
  float timescale;
};

struct CelestialResponsePacket : PacketBase<PacketType::CelestialResponse> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<CelestialResponsePacket, PacketBase<PacketType::CelestialResponse>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<CelestialResponsePacket>(); }

  CelestialResponsePacket();
  CelestialResponsePacket(List<CelestialResponse> responses);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  List<CelestialResponse> responses;
};

struct PlayerWarpResultPacket : PacketBase<PacketType::PlayerWarpResult> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<PlayerWarpResultPacket, PacketBase<PacketType::PlayerWarpResult>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<PlayerWarpResultPacket>(); }

  PlayerWarpResultPacket();
  PlayerWarpResultPacket(bool success, WarpAction warpAction, bool warpActionInvalid);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  bool success;
  WarpAction warpAction;
  bool warpActionInvalid;
};

struct PlanetTypeUpdatePacket : PacketBase<PacketType::PlanetTypeUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<PlanetTypeUpdatePacket, PacketBase<PacketType::PlanetTypeUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<PlanetTypeUpdatePacket>(); }

  PlanetTypeUpdatePacket();
  PlanetTypeUpdatePacket(CelestialCoordinate coordinate);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  CelestialCoordinate coordinate;
};

struct PausePacket : PacketBase<PacketType::Pause> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<PausePacket, PacketBase<PacketType::Pause>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<PausePacket>(); }

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
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ServerInfoPacket, PacketBase<PacketType::ServerInfo>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ServerInfoPacket>(); }

  ServerInfoPacket();
  ServerInfoPacket(std::uint16_t players, std::uint16_t maxPlayers);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  std::uint16_t players;
  std::uint16_t maxPlayers;
};

struct ClientConnectPacket : PacketBase<PacketType::ClientConnect> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ClientConnectPacket, PacketBase<PacketType::ClientConnect>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ClientConnectPacket>(); }

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
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ClientDisconnectRequestPacket, PacketBase<PacketType::ClientDisconnectRequest>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ClientDisconnectRequestPacket>(); }

  ClientDisconnectRequestPacket();

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;
};

struct HandshakeResponsePacket : PacketBase<PacketType::HandshakeResponse> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<HandshakeResponsePacket, PacketBase<PacketType::HandshakeResponse>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<HandshakeResponsePacket>(); }

  HandshakeResponsePacket();
  HandshakeResponsePacket(ByteArray const& passHash);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray passHash;
};

struct PlayerWarpPacket : PacketBase<PacketType::PlayerWarp> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<PlayerWarpPacket, PacketBase<PacketType::PlayerWarp>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<PlayerWarpPacket>(); }

  PlayerWarpPacket();
  PlayerWarpPacket(WarpAction action, bool deploy);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  WarpAction action;
  bool deploy;
};

struct FlyShipPacket : PacketBase<PacketType::FlyShip> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<FlyShipPacket, PacketBase<PacketType::FlyShip>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<FlyShipPacket>(); }

  FlyShipPacket();
  FlyShipPacket(Vec3I system, SystemLocation location, Json settings = {});

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  Vec3I system;
  SystemLocation location;
  Json settings;
};

struct ChatSendPacket : PacketBase<PacketType::ChatSend> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ChatSendPacket, PacketBase<PacketType::ChatSend>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ChatSendPacket>(); }

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
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<CelestialRequestPacket, PacketBase<PacketType::CelestialRequest>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<CelestialRequestPacket>(); }

  CelestialRequestPacket();
  CelestialRequestPacket(List<CelestialRequest> requests);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  List<CelestialRequest> requests;
};

struct ClientContextUpdatePacket : PacketBase<PacketType::ClientContextUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ClientContextUpdatePacket, PacketBase<PacketType::ClientContextUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ClientContextUpdatePacket>(); }

  ClientContextUpdatePacket();
  ClientContextUpdatePacket(ByteArray updateData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray updateData;
};

// Sent when a client should initialize themselves on a new world
struct WorldStartPacket : PacketBase<PacketType::WorldStart> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<WorldStartPacket, PacketBase<PacketType::WorldStart>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<WorldStartPacket>(); }

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
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<WorldStopPacket, PacketBase<PacketType::WorldStop>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<WorldStopPacket>(); }

  WorldStopPacket();
  WorldStopPacket(String const& reason);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String reason;
};

// Sent when the region data for the client's current world changes
struct WorldLayoutUpdatePacket : PacketBase<PacketType::WorldLayoutUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<WorldLayoutUpdatePacket, PacketBase<PacketType::WorldLayoutUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<WorldLayoutUpdatePacket>(); }

  WorldLayoutUpdatePacket();
  WorldLayoutUpdatePacket(Json const& layoutData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Json layoutData;
};

// Sent when the environment status effect list for the client's current world changes
struct WorldParametersUpdatePacket : PacketBase<PacketType::WorldParametersUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<WorldParametersUpdatePacket, PacketBase<PacketType::WorldParametersUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<WorldParametersUpdatePacket>(); }

  WorldParametersUpdatePacket();
  WorldParametersUpdatePacket(ByteArray const& parametersData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray parametersData;
};

struct CentralStructureUpdatePacket : PacketBase<PacketType::CentralStructureUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<CentralStructureUpdatePacket, PacketBase<PacketType::CentralStructureUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<CentralStructureUpdatePacket>(); }

  CentralStructureUpdatePacket();
  CentralStructureUpdatePacket(Json structureData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Json structureData;
};

struct TileArrayUpdatePacket : PacketBase<PacketType::TileArrayUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<TileArrayUpdatePacket, PacketBase<PacketType::TileArrayUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<TileArrayUpdatePacket>(); }

  typedef MultiArray<NetTile, 2> TileArray;

  TileArrayUpdatePacket();

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I min;
  TileArray array;
};

struct TileUpdatePacket : PacketBase<PacketType::TileUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<TileUpdatePacket, PacketBase<PacketType::TileUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<TileUpdatePacket>(); }

  TileUpdatePacket() {}
  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I position;
  NetTile tile;
};

struct TileLiquidUpdatePacket : PacketBase<PacketType::TileLiquidUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<TileLiquidUpdatePacket, PacketBase<PacketType::TileLiquidUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<TileLiquidUpdatePacket>(); }

  TileLiquidUpdatePacket();
  TileLiquidUpdatePacket(Vec2I const& position, LiquidNetUpdate liquidUpdate);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I position;
  LiquidNetUpdate liquidUpdate;
};

struct TileDamageUpdatePacket : PacketBase<PacketType::TileDamageUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<TileDamageUpdatePacket, PacketBase<PacketType::TileDamageUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<TileDamageUpdatePacket>(); }

  TileDamageUpdatePacket();
  TileDamageUpdatePacket(Vec2I const& position, TileLayer layer, TileDamageStatus const& tileDamage);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I position;
  TileLayer layer;

  TileDamageStatus tileDamage;
};

struct TileModificationFailurePacket : PacketBase<PacketType::TileModificationFailure> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<TileModificationFailurePacket, PacketBase<PacketType::TileModificationFailure>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<TileModificationFailurePacket>(); }

  TileModificationFailurePacket();
  TileModificationFailurePacket(TileModificationList modifications);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  TileModificationList modifications;
};

struct GiveItemPacket : PacketBase<PacketType::GiveItem> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<GiveItemPacket, PacketBase<PacketType::GiveItem>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<GiveItemPacket>(); }

  GiveItemPacket();
  GiveItemPacket(ItemDescriptor const& item);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  ItemDescriptor item;
};

struct EnvironmentUpdatePacket : PacketBase<PacketType::EnvironmentUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<EnvironmentUpdatePacket, PacketBase<PacketType::EnvironmentUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<EnvironmentUpdatePacket>(); }

  EnvironmentUpdatePacket();
  EnvironmentUpdatePacket(ByteArray skyDelta, ByteArray weatherDelta);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray skyDelta;
  ByteArray weatherDelta;
};

struct UpdateTileProtectionPacket : PacketBase<PacketType::UpdateTileProtection> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<UpdateTileProtectionPacket, PacketBase<PacketType::UpdateTileProtection>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<UpdateTileProtectionPacket>(); }

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
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SetDungeonGravityPacket, PacketBase<PacketType::SetDungeonGravity>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SetDungeonGravityPacket>(); }

  SetDungeonGravityPacket();
  SetDungeonGravityPacket(DungeonId dungeonId, std::optional<float> gravity);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  DungeonId dungeonId;
  std::optional<float> gravity;
};

struct SetDungeonBreathablePacket : PacketBase<PacketType::SetDungeonBreathable> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SetDungeonBreathablePacket, PacketBase<PacketType::SetDungeonBreathable>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SetDungeonBreathablePacket>(); }

  SetDungeonBreathablePacket();
  SetDungeonBreathablePacket(DungeonId dungeonId, std::optional<bool> breathable);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  DungeonId dungeonId;
  std::optional<bool> breathable;
};

struct SetPlayerStartPacket : PacketBase<PacketType::SetPlayerStart> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SetPlayerStartPacket, PacketBase<PacketType::SetPlayerStart>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SetPlayerStartPacket>(); }

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
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<FindUniqueEntityResponsePacket, PacketBase<PacketType::FindUniqueEntityResponse>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<FindUniqueEntityResponsePacket>(); }

  FindUniqueEntityResponsePacket();
  FindUniqueEntityResponsePacket(String uniqueEntityId, std::optional<Vec2F> entityPosition);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String uniqueEntityId;
  std::optional<Vec2F> entityPosition;
};

struct PongPacket : PacketBase<PacketType::Pong> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<PongPacket, PacketBase<PacketType::Pong>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<PongPacket>(); }

  PongPacket();
  PongPacket(std::int64_t time);

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  std::int64_t time = 0;
};

struct ModifyTileListPacket : PacketBase<PacketType::ModifyTileList> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ModifyTileListPacket, PacketBase<PacketType::ModifyTileList>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ModifyTileListPacket>(); }

  ModifyTileListPacket();
  ModifyTileListPacket(TileModificationList modifications, bool allowEntityOverlap);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  TileModificationList modifications;
  bool allowEntityOverlap;
};

struct ReplaceTileListPacket : PacketBase<PacketType::ReplaceTileList> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ReplaceTileListPacket, PacketBase<PacketType::ReplaceTileList>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ReplaceTileListPacket>(); }

  ReplaceTileListPacket();
  ReplaceTileListPacket(TileModificationList modifications, TileDamage tileDamage, bool applyDamage);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  TileModificationList modifications;
  TileDamage tileDamage;
  bool applyDamage;
};

struct DamageTileGroupPacket : PacketBase<PacketType::DamageTileGroup> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<DamageTileGroupPacket, PacketBase<PacketType::DamageTileGroup>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<DamageTileGroupPacket>(); }

  DamageTileGroupPacket();
  DamageTileGroupPacket(List<Vec2I> tilePositions, TileLayer layer, Vec2F sourcePosition, TileDamage tileDamage, std::optional<EntityId> sourceEntity);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  List<Vec2I> tilePositions;
  TileLayer layer;
  Vec2F sourcePosition;
  TileDamage tileDamage;
  std::optional<EntityId> sourceEntity;
};

struct CollectLiquidPacket : PacketBase<PacketType::CollectLiquid> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<CollectLiquidPacket, PacketBase<PacketType::CollectLiquid>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<CollectLiquidPacket>(); }

  CollectLiquidPacket();
  CollectLiquidPacket(List<Vec2I> tilePositions, LiquidId liquidId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  List<Vec2I> tilePositions;
  LiquidId liquidId;
};

struct RequestDropPacket : PacketBase<PacketType::RequestDrop> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<RequestDropPacket, PacketBase<PacketType::RequestDrop>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<RequestDropPacket>(); }

  RequestDropPacket();
  RequestDropPacket(EntityId dropEntityId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  EntityId dropEntityId;
};

struct SpawnEntityPacket : PacketBase<PacketType::SpawnEntity> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SpawnEntityPacket, PacketBase<PacketType::SpawnEntity>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SpawnEntityPacket>(); }

  SpawnEntityPacket();
  SpawnEntityPacket(EntityType entityType, ByteArray storeData, ByteArray firstNetState);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  EntityType entityType;
  ByteArray storeData;
  ByteArray firstNetState;
};

struct ConnectWirePacket : PacketBase<PacketType::ConnectWire> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ConnectWirePacket, PacketBase<PacketType::ConnectWire>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ConnectWirePacket>(); }

  ConnectWirePacket();
  ConnectWirePacket(WireConnection outputConnection, WireConnection inputConnection);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  WireConnection outputConnection;
  WireConnection inputConnection;
};

struct DisconnectAllWiresPacket : PacketBase<PacketType::DisconnectAllWires> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<DisconnectAllWiresPacket, PacketBase<PacketType::DisconnectAllWires>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<DisconnectAllWiresPacket>(); }

  DisconnectAllWiresPacket();
  DisconnectAllWiresPacket(Vec2I entityPosition, WireNode wireNode);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec2I entityPosition;
  WireNode wireNode;
};

struct WorldClientStateUpdatePacket : PacketBase<PacketType::WorldClientStateUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<WorldClientStateUpdatePacket, PacketBase<PacketType::WorldClientStateUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<WorldClientStateUpdatePacket>(); }

  WorldClientStateUpdatePacket();
  WorldClientStateUpdatePacket(ByteArray const& worldClientStateDelta);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray worldClientStateDelta;
};

struct FindUniqueEntityPacket : PacketBase<PacketType::FindUniqueEntity> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<FindUniqueEntityPacket, PacketBase<PacketType::FindUniqueEntity>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<FindUniqueEntityPacket>(); }

  FindUniqueEntityPacket();
  FindUniqueEntityPacket(String uniqueEntityId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String uniqueEntityId;
};

struct WorldStartAcknowledgePacket : PacketBase<PacketType::WorldStartAcknowledge> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<WorldStartAcknowledgePacket, PacketBase<PacketType::WorldStartAcknowledge>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<WorldStartAcknowledgePacket>(); }

  WorldStartAcknowledgePacket();

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;
};

struct PingPacket : PacketBase<PacketType::Ping> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<PingPacket, PacketBase<PacketType::Ping>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<PingPacket>(); }

  PingPacket();
  PingPacket(std::int64_t time);

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  std::int64_t time = 0;
};

struct EntityCreatePacket : PacketBase<PacketType::EntityCreate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<EntityCreatePacket, PacketBase<PacketType::EntityCreate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<EntityCreatePacket>(); }

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
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<EntityUpdateSetPacket, PacketBase<PacketType::EntityUpdateSet>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<EntityUpdateSetPacket>(); }

  EntityUpdateSetPacket();
  EntityUpdateSetPacket(ConnectionId forConnection);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ConnectionId forConnection;
  HashMap<EntityId, ByteArray> deltas;
};

struct EntityDestroyPacket : PacketBase<PacketType::EntityDestroy> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<EntityDestroyPacket, PacketBase<PacketType::EntityDestroy>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<EntityDestroyPacket>(); }

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
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<EntityInteractPacket, PacketBase<PacketType::EntityInteract>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<EntityInteractPacket>(); }

  EntityInteractPacket();
  EntityInteractPacket(InteractRequest interactRequest, Uuid requestId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  InteractRequest interactRequest;
  Uuid requestId;
};

struct EntityInteractResultPacket : PacketBase<PacketType::EntityInteractResult> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<EntityInteractResultPacket, PacketBase<PacketType::EntityInteractResult>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<EntityInteractResultPacket>(); }

  EntityInteractResultPacket();
  EntityInteractResultPacket(InteractAction action, Uuid requestId, EntityId sourceEntityId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  InteractAction action;
  Uuid requestId;
  EntityId sourceEntityId;
};

struct HitRequestPacket : PacketBase<PacketType::HitRequest> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<HitRequestPacket, PacketBase<PacketType::HitRequest>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<HitRequestPacket>(); }

  HitRequestPacket();
  HitRequestPacket(RemoteHitRequest remoteHitRequest);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  RemoteHitRequest remoteHitRequest;
};

struct DamageRequestPacket : PacketBase<PacketType::DamageRequest> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<DamageRequestPacket, PacketBase<PacketType::DamageRequest>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<DamageRequestPacket>(); }

  DamageRequestPacket();
  DamageRequestPacket(RemoteDamageRequest remoteDamageRequest);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  RemoteDamageRequest remoteDamageRequest;
};

struct DamageNotificationPacket : PacketBase<PacketType::DamageNotification> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<DamageNotificationPacket, PacketBase<PacketType::DamageNotification>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<DamageNotificationPacket>(); }

  DamageNotificationPacket();
  DamageNotificationPacket(RemoteDamageNotification remoteDamageNotification);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  RemoteDamageNotification remoteDamageNotification;
};

struct EntityMessagePacket : PacketBase<PacketType::EntityMessage> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<EntityMessagePacket, PacketBase<PacketType::EntityMessage>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<EntityMessagePacket>(); }

  EntityMessagePacket();
  EntityMessagePacket(std::variant<EntityId, String> entityId, String message, JsonArray args, Uuid uuid, ConnectionId fromConnection = ServerConnectionId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  std::variant<EntityId, String> entityId;
  String message;
  JsonArray args;
  Uuid uuid;
  ConnectionId fromConnection;
};

struct EntityMessageResponsePacket : PacketBase<PacketType::EntityMessageResponse> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<EntityMessageResponsePacket, PacketBase<PacketType::EntityMessageResponse>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<EntityMessageResponsePacket>(); }

  EntityMessageResponsePacket();
  EntityMessageResponsePacket(Either<String, Json> response, Uuid uuid);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Either<String, Json> response;
  Uuid uuid;
};

struct UpdateWorldPropertiesPacket : PacketBase<PacketType::UpdateWorldProperties> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<UpdateWorldPropertiesPacket, PacketBase<PacketType::UpdateWorldProperties>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<UpdateWorldPropertiesPacket>(); }

  UpdateWorldPropertiesPacket();
  UpdateWorldPropertiesPacket(JsonObject const& updatedProperties);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  void readJson(Json const& json) override;
  Json writeJson() const override;

  JsonObject updatedProperties;
};

struct StepUpdatePacket : PacketBase<PacketType::StepUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<StepUpdatePacket, PacketBase<PacketType::StepUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<StepUpdatePacket>(); }

  StepUpdatePacket();
  StepUpdatePacket(double remoteTime);

  void read(DataStream& ds, NetCompatibilityRules netRules) override;
  void write(DataStream& ds, NetCompatibilityRules netRules) const override;

  double remoteTime;
};

struct SystemWorldStartPacket : PacketBase<PacketType::SystemWorldStart> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SystemWorldStartPacket, PacketBase<PacketType::SystemWorldStart>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SystemWorldStartPacket>(); }

  SystemWorldStartPacket();
  SystemWorldStartPacket(Vec3I location, List<ByteArray> objectStores, List<ByteArray> shipStores, std::pair<Uuid, SystemLocation> clientShip);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Vec3I location;
  List<ByteArray> objectStores;
  List<ByteArray> shipStores;
  std::pair<Uuid, SystemLocation> clientShip;
};

struct SystemWorldUpdatePacket : PacketBase<PacketType::SystemWorldUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SystemWorldUpdatePacket, PacketBase<PacketType::SystemWorldUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SystemWorldUpdatePacket>(); }

  SystemWorldUpdatePacket();
  SystemWorldUpdatePacket(HashMap<Uuid, ByteArray> objectUpdates, HashMap<Uuid, ByteArray> shipUpdates);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  HashMap<Uuid, ByteArray> objectUpdates;
  HashMap<Uuid, ByteArray> shipUpdates;
};

struct SystemObjectCreatePacket : PacketBase<PacketType::SystemObjectCreate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SystemObjectCreatePacket, PacketBase<PacketType::SystemObjectCreate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SystemObjectCreatePacket>(); }

  SystemObjectCreatePacket();
  SystemObjectCreatePacket(ByteArray objectStore);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray objectStore;
};

struct SystemObjectDestroyPacket : PacketBase<PacketType::SystemObjectDestroy> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SystemObjectDestroyPacket, PacketBase<PacketType::SystemObjectDestroy>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SystemObjectDestroyPacket>(); }

  SystemObjectDestroyPacket();
  SystemObjectDestroyPacket(Uuid objectUuid);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Uuid objectUuid;
};

struct SystemShipCreatePacket : PacketBase<PacketType::SystemShipCreate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SystemShipCreatePacket, PacketBase<PacketType::SystemShipCreate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SystemShipCreatePacket>(); }

  SystemShipCreatePacket();
  SystemShipCreatePacket(ByteArray shipStore);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ByteArray shipStore;
};

struct SystemShipDestroyPacket : PacketBase<PacketType::SystemShipDestroy> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SystemShipDestroyPacket, PacketBase<PacketType::SystemShipDestroy>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SystemShipDestroyPacket>(); }

  SystemShipDestroyPacket();
  SystemShipDestroyPacket(Uuid shipUuid);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Uuid shipUuid;
};

struct SystemObjectSpawnPacket : PacketBase<PacketType::SystemObjectSpawn> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<SystemObjectSpawnPacket, PacketBase<PacketType::SystemObjectSpawn>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<SystemObjectSpawnPacket>(); }

  SystemObjectSpawnPacket();
  SystemObjectSpawnPacket(String typeName, Uuid uuid, std::optional<Vec2F> position, JsonObject parameters);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String typeName;
  Uuid uuid;
  std::optional<Vec2F> position;
  JsonObject parameters;
};

struct UpdateWorldTemplatePacket : PacketBase<PacketType::UpdateWorldTemplate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<UpdateWorldTemplatePacket, PacketBase<PacketType::UpdateWorldTemplate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<UpdateWorldTemplatePacket>(); }

  UpdateWorldTemplatePacket();
  UpdateWorldTemplatePacket(Json templateData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Json templateData;
};

struct ClientCustomWorldRequest : PacketBase<PacketType::ClientCustomWorldRequest> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ClientCustomWorldRequest, PacketBase<PacketType::ClientCustomWorldRequest>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ClientCustomWorldRequest>(); }

  ClientCustomWorldRequest();
  ClientCustomWorldRequest(String name);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String name;
};

struct ClientCustomWorldResponse : PacketBase<PacketType::ClientCustomWorldResponse> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ClientCustomWorldResponse, PacketBase<PacketType::ClientCustomWorldResponse>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ClientCustomWorldResponse>(); }

  ClientCustomWorldResponse();
  ClientCustomWorldResponse(String name, WorldChunks chunks);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String name;
  WorldChunks chunks;
};

struct ClientCustomWorldCreate : PacketBase<PacketType::ClientCustomWorldCreate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ClientCustomWorldCreate, PacketBase<PacketType::ClientCustomWorldCreate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ClientCustomWorldCreate>(); }

  ClientCustomWorldCreate();
  ClientCustomWorldCreate(String name, Json templateData);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  String name;
  Json templateData;
};

// wrapper packet for client world threads
struct ClientSubWorldPackets : PacketBase<PacketType::ClientSubWorldPackets> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ClientSubWorldPackets, PacketBase<PacketType::ClientSubWorldPackets>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ClientSubWorldPackets>(); }

  ClientSubWorldPackets();
  ClientSubWorldPackets(ClientSubWorldId const& subWorldId, List<std::shared_ptr<Packet>> packets);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ClientSubWorldId subWorldId;
  List<std::shared_ptr<Packet>> packets;
};

struct ClientSubWorldRequest : PacketBase<PacketType::ClientSubWorldRequest> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ClientSubWorldRequest, PacketBase<PacketType::ClientSubWorldRequest>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ClientSubWorldRequest>(); }

  ClientSubWorldRequest();
  ClientSubWorldRequest(ClientSubWorldId const& subWorldId, WorldId worldId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ClientSubWorldId subWorldId;
  WorldId worldId;
};

struct ClientSubWorldReject : PacketBase<PacketType::ClientSubWorldReject> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<ClientSubWorldReject, PacketBase<PacketType::ClientSubWorldReject>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<ClientSubWorldReject>(); }

  ClientSubWorldReject();
  ClientSubWorldReject(ClientSubWorldId const& subWorldId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  ClientSubWorldId subWorldId;
};

struct NotifyWorldLoad : PacketBase<PacketType::NotifyWorldLoad> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<NotifyWorldLoad, PacketBase<PacketType::NotifyWorldLoad>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<NotifyWorldLoad>(); }

  NotifyWorldLoad();
  NotifyWorldLoad(WorldId worldId);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  WorldId worldId;
};

struct LogMapUpdate : PacketBase<PacketType::LogMapUpdate> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<LogMapUpdate, PacketBase<PacketType::LogMapUpdate>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<LogMapUpdate>(); }

  LogMapUpdate();
  LogMapUpdate(Map<String,String> map);

  void read(DataStream& ds) override;
  void write(DataStream& ds) const override;

  Map<String,String> map;
};

struct UniverseMessage : PacketBase<PacketType::UniverseMessage> {
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<UniverseMessage, PacketBase<PacketType::UniverseMessage>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<UniverseMessage>(); }

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
  void const* queryType(TypeKey key) const noexcept override { return queryTypeOf<UniverseMessageResponse, PacketBase<PacketType::UniverseMessageResponse>>(this, key); }
  std::string_view dynamicTypeName() const noexcept override { return Star::typeName<UniverseMessageResponse>(); }

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
