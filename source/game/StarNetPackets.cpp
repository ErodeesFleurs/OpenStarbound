import star.json_extra;
import star.xx_hash;
import star.json;
import star.id_map;
import star.data_stream;
import star.list;
import star.bi_map;
import star.game_types;
import star.math_common;
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
import star.data_stream_extra;

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
import star.net_packets;

import star.byte_array;
namespace Star {

extern VersionNumber const StarProtocolVersion = 747;

extern std::expected<EnumMap<PacketType>, StarException> const PacketTypeNames = EnumMap<PacketType>::create({
  {PacketType::ProtocolRequest, "ProtocolRequest"},
  {PacketType::ProtocolResponse, "ProtocolResponse"},
  {PacketType::ServerDisconnect, "ServerDisconnect"},
  {PacketType::ConnectSuccess, "ConnectSuccess"},
  {PacketType::ConnectFailure, "ConnectFailure"},
  {PacketType::HandshakeChallenge, "HandshakeChallenge"},
  {PacketType::ChatReceive, "ChatReceive"},
  {PacketType::UniverseTimeUpdate, "UniverseTimeUpdate"},
  {PacketType::CelestialResponse, "CelestialResponse"},
  {PacketType::PlayerWarpResult, "PlayerWarpResult"},
  {PacketType::PlanetTypeUpdate, "PlanetTypeUpdate"},
  {PacketType::Pause, "Pause"},
  {PacketType::ServerInfo, "ServerInfo"},
  {PacketType::ClientConnect, "ClientConnect"},
  {PacketType::ClientDisconnectRequest, "ClientDisconnectRequest"},
  {PacketType::HandshakeResponse, "HandshakeResponse"},
  {PacketType::PlayerWarp, "PlayerWarp"},
  {PacketType::FlyShip, "FlyShip"},
  {PacketType::ChatSend, "ChatSend"},
  {PacketType::CelestialRequest, "CelestialRequest"},
  {PacketType::ClientContextUpdate, "ClientContextUpdate"},
  {PacketType::WorldStart, "WorldStart"},
  {PacketType::WorldStop, "WorldStop"},
  {PacketType::WorldLayoutUpdate, "WorldLayoutUpdate"},
  {PacketType::WorldParametersUpdate, "WorldParametersUpdate"},
  {PacketType::CentralStructureUpdate, "CentralStructureUpdate"},
  {PacketType::TileArrayUpdate, "TileArrayUpdate"},
  {PacketType::TileUpdate, "TileUpdate"},
  {PacketType::TileLiquidUpdate, "TileLiquidUpdate"},
  {PacketType::TileDamageUpdate, "TileDamageUpdate"},
  {PacketType::TileModificationFailure, "TileModificationFailure"},
  {PacketType::GiveItem, "GiveItem"},
  {PacketType::EnvironmentUpdate, "EnvironmentUpdate"},
  {PacketType::UpdateTileProtection, "UpdateTileProtection"},
  {PacketType::SetDungeonGravity, "SetDungeonGravity"},
  {PacketType::SetDungeonBreathable, "SetDungeonBreathable"},
  {PacketType::SetPlayerStart, "SetPlayerStart"},
  {PacketType::FindUniqueEntityResponse, "FindUniqueEntityResponse"},
  {PacketType::Pong, "Pong"},
  {PacketType::ModifyTileList, "ModifyTileList"},
  {PacketType::DamageTileGroup, "DamageTileGroup"},
  {PacketType::CollectLiquid, "CollectLiquid"},
  {PacketType::RequestDrop, "RequestDrop"},
  {PacketType::SpawnEntity, "SpawnEntity"},
  {PacketType::ConnectWire, "ConnectWire"},
  {PacketType::DisconnectAllWires, "DisconnectAllWires"},
  {PacketType::WorldClientStateUpdate, "WorldClientStateUpdate"},
  {PacketType::FindUniqueEntity, "FindUniqueEntity"},
  {PacketType::WorldStartAcknowledge, "WorldStartAcknowledge"},
  {PacketType::Ping, "Ping"},
  {PacketType::EntityCreate, "EntityCreate"},
  {PacketType::EntityUpdateSet, "EntityUpdate"},
  {PacketType::EntityDestroy, "EntityDestroy"},
  {PacketType::EntityInteract, "EntityInteract"},
  {PacketType::EntityInteractResult, "EntityInteractResult"},
  {PacketType::HitRequest, "HitRequest"},
  {PacketType::DamageRequest, "DamageRequest"},
  {PacketType::DamageNotification, "DamageNotification"},
  {PacketType::EntityMessage, "EntityMessage"},
  {PacketType::EntityMessageResponse, "EntityMessageResponse"},
  {PacketType::UpdateWorldProperties, "UpdateWorldProperties"},
  {PacketType::StepUpdate, "StepUpdate"},
  {PacketType::SystemWorldStart, "SystemWorldStart"},
  {PacketType::SystemWorldUpdate, "SystemWorldUpdate"},
  {PacketType::SystemObjectCreate, "SystemObjectCreate"},
  {PacketType::SystemObjectDestroy, "SystemObjectDestroy"},
  {PacketType::SystemShipCreate, "SystemShipCreate"},
  {PacketType::SystemShipDestroy, "SystemShipDestroy"},
  {PacketType::SystemObjectSpawn, "SystemObjectSpawn"},
  // OpenStarbound packets
  {PacketType::ReplaceTileList, "ReplaceTileList"},
  {PacketType::UpdateWorldTemplate, "UpdateWorldTemplate"},
  {PacketType::ClientCustomWorldRequest, "ClientCustomWorldRequest"},
  {PacketType::ClientCustomWorldResponse, "ClientCustomWorldResponse"},
  {PacketType::ClientCustomWorldCreate, "ClientCustomWorldCreate"},
  {PacketType::ClientSubWorldPackets, "ClientSubWorldPackets"},
  {PacketType::ClientSubWorldRequest, "ClientSubWorldRequest"},
  {PacketType::ClientSubWorldReject, "ClientSubWorldReject"},
  {PacketType::NotifyWorldLoad, "NotifyWorldLoad"},
  {PacketType::LogMapUpdate, "LogMapUpdate"},
  {PacketType::UniverseMessage, "UniverseMessage"},
  {PacketType::UniverseMessageResponse, "UniverseMessageResponse"}
});

extern std::expected<EnumMap<NetCompressionMode>, StarException> const NetCompressionModeNames = EnumMap<NetCompressionMode>::create({
  {NetCompressionMode::None, "None"},
  {NetCompressionMode::Zstd, "Zstd"}
});

Packet::~Packet() {}

std::expected<void, StarException> Packet::read(DataStream& ds, NetCompatibilityRules) { return read(ds); }
std::expected<void, StarException> Packet::read(DataStream&) { return {}; }
std::expected<void, StarException> Packet::write(DataStream& ds, NetCompatibilityRules) const { return write(ds); }
std::expected<void, StarException> Packet::write(DataStream&) const { return {}; }
std::expected<void, StarException> Packet::readJson(Json const&) { return {}; }
std::expected<Json, StarException> Packet::writeJson() const  { return JsonObject{}; }

PacketCompressionMode Packet::compressionMode() const { return m_compressionMode; }
void Packet::setCompressionMode(PacketCompressionMode compressionMode) { m_compressionMode = compressionMode; }

namespace {
// Empty packet construction stages a subsequent wire/JSON read, not an identity.
// Sending code supplies IDs or uses the explicit random-source factory below.
Uuid unfilledPacketUuid() {
  return Uuid::nil();
}
}

std::expected<std::shared_ptr<Packet>, StarException> createPacket(PacketType type) {
  switch (type) {
    case PacketType::ProtocolRequest: return std::make_shared<ProtocolRequestPacket>();
    case PacketType::ProtocolResponse: return std::make_shared<ProtocolResponsePacket>();
    case PacketType::ServerDisconnect: return std::make_shared<ServerDisconnectPacket>();
    case PacketType::ConnectSuccess: return std::make_shared<ConnectSuccessPacket>();
    case PacketType::ConnectFailure: return std::make_shared<ConnectFailurePacket>();
    case PacketType::HandshakeChallenge: return std::make_shared<HandshakeChallengePacket>();
    case PacketType::ChatReceive: return std::make_shared<ChatReceivePacket>();
    case PacketType::UniverseTimeUpdate: return std::make_shared<UniverseTimeUpdatePacket>();
    case PacketType::CelestialResponse: return std::make_shared<CelestialResponsePacket>();
    case PacketType::PlayerWarpResult: return std::make_shared<PlayerWarpResultPacket>();
    case PacketType::PlanetTypeUpdate: return std::make_shared<PlanetTypeUpdatePacket>();
    case PacketType::Pause: return std::make_shared<PausePacket>();
    case PacketType::ServerInfo: return std::make_shared<ServerInfoPacket>();
    case PacketType::ClientConnect: return std::make_shared<ClientConnectPacket>();
    case PacketType::ClientDisconnectRequest: return std::make_shared<ClientDisconnectRequestPacket>();
    case PacketType::HandshakeResponse: return std::make_shared<HandshakeResponsePacket>();
    case PacketType::PlayerWarp: return std::make_shared<PlayerWarpPacket>();
    case PacketType::FlyShip: return std::make_shared<FlyShipPacket>();
    case PacketType::ChatSend: return std::make_shared<ChatSendPacket>();
    case PacketType::CelestialRequest: return std::make_shared<CelestialRequestPacket>();
    case PacketType::ClientContextUpdate: return std::make_shared<ClientContextUpdatePacket>();
    case PacketType::WorldStart: return std::make_shared<WorldStartPacket>();
    case PacketType::WorldStop: return std::make_shared<WorldStopPacket>();
    case PacketType::WorldLayoutUpdate: return std::make_shared<WorldLayoutUpdatePacket>();
    case PacketType::WorldParametersUpdate: return std::make_shared<WorldParametersUpdatePacket>();
    case PacketType::CentralStructureUpdate: return std::make_shared<CentralStructureUpdatePacket>();
    case PacketType::TileArrayUpdate: return std::make_shared<TileArrayUpdatePacket>();
    case PacketType::TileUpdate: return std::make_shared<TileUpdatePacket>();
    case PacketType::TileLiquidUpdate: return std::make_shared<TileLiquidUpdatePacket>();
    case PacketType::TileDamageUpdate: return std::make_shared<TileDamageUpdatePacket>();
    case PacketType::TileModificationFailure: return std::make_shared<TileModificationFailurePacket>();
    case PacketType::GiveItem: return std::make_shared<GiveItemPacket>();
    case PacketType::EnvironmentUpdate: return std::make_shared<EnvironmentUpdatePacket>();
    case PacketType::UpdateTileProtection: return std::make_shared<UpdateTileProtectionPacket>();
    case PacketType::SetDungeonGravity: return std::make_shared<SetDungeonGravityPacket>();
    case PacketType::SetDungeonBreathable: return std::make_shared<SetDungeonBreathablePacket>();
    case PacketType::SetPlayerStart: return std::make_shared<SetPlayerStartPacket>();
    case PacketType::FindUniqueEntityResponse: return std::make_shared<FindUniqueEntityResponsePacket>();
    case PacketType::Pong: return std::make_shared<PongPacket>();
    case PacketType::ModifyTileList: return std::make_shared<ModifyTileListPacket>();
    case PacketType::DamageTileGroup: return std::make_shared<DamageTileGroupPacket>();
    case PacketType::CollectLiquid: return std::make_shared<CollectLiquidPacket>();
    case PacketType::RequestDrop: return std::make_shared<RequestDropPacket>();
    case PacketType::SpawnEntity: return std::make_shared<SpawnEntityPacket>();
    case PacketType::ConnectWire: return std::make_shared<ConnectWirePacket>();
    case PacketType::DisconnectAllWires: return std::make_shared<DisconnectAllWiresPacket>();
    case PacketType::WorldClientStateUpdate: return std::make_shared<WorldClientStateUpdatePacket>();
    case PacketType::FindUniqueEntity: return std::make_shared<FindUniqueEntityPacket>();
    case PacketType::WorldStartAcknowledge: return std::make_shared<WorldStartAcknowledgePacket>();
    case PacketType::Ping: return std::make_shared<PingPacket>();
    case PacketType::EntityCreate: return std::make_shared<EntityCreatePacket>();
    case PacketType::EntityUpdateSet: return std::make_shared<EntityUpdateSetPacket>();
    case PacketType::EntityDestroy: return std::make_shared<EntityDestroyPacket>();
    case PacketType::EntityInteract: return std::make_shared<EntityInteractPacket>();
    case PacketType::EntityInteractResult: return std::make_shared<EntityInteractResultPacket>();
    case PacketType::HitRequest: return std::make_shared<HitRequestPacket>();
    case PacketType::DamageRequest: return std::make_shared<DamageRequestPacket>();
    case PacketType::DamageNotification: return std::make_shared<DamageNotificationPacket>();
    case PacketType::EntityMessage: return std::make_shared<EntityMessagePacket>();
    case PacketType::EntityMessageResponse: return std::make_shared<EntityMessageResponsePacket>();
    case PacketType::UpdateWorldProperties: return std::make_shared<UpdateWorldPropertiesPacket>();
    case PacketType::StepUpdate: return std::make_shared<StepUpdatePacket>();
    case PacketType::SystemWorldStart: return std::make_shared<SystemWorldStartPacket>();
    case PacketType::SystemWorldUpdate: return std::make_shared<SystemWorldUpdatePacket>();
    case PacketType::SystemObjectCreate: return std::make_shared<SystemObjectCreatePacket>();
    case PacketType::SystemObjectDestroy: return std::make_shared<SystemObjectDestroyPacket>();
    case PacketType::SystemShipCreate: return std::make_shared<SystemShipCreatePacket>();
    case PacketType::SystemShipDestroy: return std::make_shared<SystemShipDestroyPacket>();
    case PacketType::SystemObjectSpawn: return std::make_shared<SystemObjectSpawnPacket>();
    // OpenStarbound
    case PacketType::ReplaceTileList: return std::make_shared<ReplaceTileListPacket>();
    case PacketType::UpdateWorldTemplate: return std::make_shared<UpdateWorldTemplatePacket>();
    case PacketType::ClientCustomWorldRequest: return std::make_shared<ClientCustomWorldRequest>();
    case PacketType::ClientCustomWorldResponse: return std::make_shared<ClientCustomWorldResponse>();
    case PacketType::ClientCustomWorldCreate: return std::make_shared<ClientCustomWorldCreate>();
    case PacketType::ClientSubWorldPackets: return std::make_shared<ClientSubWorldPackets>();
    case PacketType::ClientSubWorldRequest: return std::make_shared<ClientSubWorldRequest>();
    case PacketType::ClientSubWorldReject: return std::make_shared<ClientSubWorldReject>();
    case PacketType::NotifyWorldLoad: return std::make_shared<NotifyWorldLoad>();
    case PacketType::LogMapUpdate: return std::make_shared<LogMapUpdate>();
    case PacketType::UniverseMessage: return std::make_shared<UniverseMessage>();
    case PacketType::UniverseMessageResponse: return std::make_shared<UniverseMessageResponse>();
    default:
      return std::unexpected(StarPacketException::format("Unrecognized packet type {}", (unsigned int)type));
  }
}

std::expected<std::shared_ptr<Packet>, StarException> createPacket(RandomService& random, PacketType type, std::optional<Json> const& args) {
  auto created = createPacket(type);
  if (!created)
    return std::unexpected(std::move(created.error()));
  auto packet = std::move(*created);
  // The raw Lua packet API also permits omitted JSON fields. Preserve its
  // generated identities explicitly; decoding through createPacket(type) never
  // consumes the domain's random stream.
  switch (type) {
    case PacketType::ConnectSuccess:
      static_cast<ConnectSuccessPacket&>(*packet).serverUuid = Uuid(random);
      break;
    case PacketType::ClientConnect:
      static_cast<ClientConnectPacket&>(*packet).playerUuid = Uuid(random);
      break;
    case PacketType::EntityInteract:
      static_cast<EntityInteractPacket&>(*packet).requestId = Uuid(random);
      break;
    case PacketType::EntityInteractResult:
      static_cast<EntityInteractResultPacket&>(*packet).requestId = Uuid(random);
      break;
    case PacketType::EntityMessage:
      static_cast<EntityMessagePacket&>(*packet).uuid = Uuid(random);
      break;
    case PacketType::EntityMessageResponse:
      static_cast<EntityMessageResponsePacket&>(*packet).uuid = Uuid(random);
      break;
    case PacketType::SystemWorldStart:
      static_cast<SystemWorldStartPacket&>(*packet).clientShip.first = Uuid(random);
      break;
    case PacketType::SystemObjectDestroy:
      static_cast<SystemObjectDestroyPacket&>(*packet).objectUuid = Uuid(random);
      break;
    case PacketType::SystemShipDestroy:
      static_cast<SystemShipDestroyPacket&>(*packet).shipUuid = Uuid(random);
      break;
    case PacketType::SystemObjectSpawn:
      static_cast<SystemObjectSpawnPacket&>(*packet).uuid = Uuid(random);
      break;
    case PacketType::UniverseMessage:
      static_cast<UniverseMessage&>(*packet).uuid = Uuid(random);
      break;
    case PacketType::UniverseMessageResponse:
      static_cast<UniverseMessageResponse&>(*packet).uuid = Uuid(random);
      break;
    default:
      break;
  }

  if (args && !args->isNull()) {
    auto read = packet->readJson(*args);
    if (!read)
      return std::unexpected(std::move(read.error()));
  }

  return packet;
}

ProtocolRequestPacket::ProtocolRequestPacket()
  : requestProtocolVersion(0) {}

ProtocolRequestPacket::ProtocolRequestPacket(VersionNumber requestProtocolVersion)
  : requestProtocolVersion(requestProtocolVersion) {}

std::expected<void, StarException> ProtocolRequestPacket::read(DataStream& ds) {
  {
    auto result = ds.read(requestProtocolVersion);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ProtocolRequestPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(requestProtocolVersion);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ProtocolResponsePacket::ProtocolResponsePacket() : ProtocolResponsePacket(false, {}) {}

ProtocolResponsePacket::ProtocolResponsePacket(bool allowed, Json info)
  : allowed(allowed), info(info) {}

std::expected<void, StarException> ProtocolResponsePacket::read(DataStream& ds) {
  {
    auto result = ds.read(allowed);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (compressionMode() == PacketCompressionMode::Enabled) {
    // gross hack for backwards compatibility with older OpenSB servers
    // can be removed later
    auto externalBuffer = as<DataStreamExternalBuffer>(&ds);
    bool readInfo = !externalBuffer;
    if (externalBuffer) {
      auto ended = externalBuffer->atEnd();
      if (!ended)
        return std::unexpected(std::move(ended.error()));
      readInfo = !*ended;
    }
    if (readInfo) {
      auto result = ds.read(info);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  }
  return {};
}

std::expected<void, StarException> ProtocolResponsePacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  {
    auto result = ds.write(allowed);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (!netRules.isLegacy())
    {
      auto result = ds.write(info);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  return {};
}

ConnectSuccessPacket::ConnectSuccessPacket() : serverUuid(unfilledPacketUuid()) {}

ConnectSuccessPacket::ConnectSuccessPacket(
    ConnectionId clientId, Uuid serverUuid, CelestialBaseInformation celestialInformation)
  : clientId(clientId), serverUuid(std::move(serverUuid)), celestialInformation(std::move(celestialInformation)) {}

std::expected<void, StarException> ConnectSuccessPacket::read(DataStream& ds) {
  {
    auto result = ds.vuread(clientId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(serverUuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(celestialInformation);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ConnectSuccessPacket::write(DataStream& ds) const {
  {
    auto result = ds.vuwrite(clientId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(serverUuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(celestialInformation);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ConnectFailurePacket::ConnectFailurePacket() {}

ConnectFailurePacket::ConnectFailurePacket(String reason) : reason(std::move(reason)) {}

std::expected<void, StarException> ConnectFailurePacket::read(DataStream& ds) {
  {
    auto result = ds.read(reason);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ConnectFailurePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(reason);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

HandshakeChallengePacket::HandshakeChallengePacket() {}

HandshakeChallengePacket::HandshakeChallengePacket(ByteArray const& passwordSalt) : passwordSalt(passwordSalt) {}

std::expected<void, StarException> HandshakeChallengePacket::read(DataStream& ds) {
  {
    auto result = ds.read(passwordSalt);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> HandshakeChallengePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(passwordSalt);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ChatReceivePacket::ChatReceivePacket() {}

ChatReceivePacket::ChatReceivePacket(ChatReceivedMessage receivedMessage) : receivedMessage(receivedMessage) {}

std::expected<void, StarException> ChatReceivePacket::read(DataStream& ds) {
  {
    auto result = ds.read(receivedMessage);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ChatReceivePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(receivedMessage);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ChatReceivePacket::readJson(Json const& json) {
  auto data = json.get("receivedMessage");
  if (!data)
    return std::unexpected(std::move(data.error()));
  auto decoded = ChatReceivedMessage::create(*data);
  if (!decoded)
    return std::unexpected(std::move(decoded.error()));
  receivedMessage = std::move(*decoded);
  return {};
}

std::expected<Json, StarException> ChatReceivePacket::writeJson() const {
  auto encoded = receivedMessage.toJson();
  if (!encoded)
    return std::unexpected(std::move(encoded.error()));
  return JsonObject{
    {"receivedMessage", std::move(*encoded)}
  };
}

UniverseTimeUpdatePacket::UniverseTimeUpdatePacket() {
  universeTime = 0;
}

UniverseTimeUpdatePacket::UniverseTimeUpdatePacket(double universeTime) : universeTime(universeTime) {}

std::expected<void, StarException> UniverseTimeUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.vfread(universeTime, 0.05);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> UniverseTimeUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.vfwrite(universeTime, 0.05);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

CelestialResponsePacket::CelestialResponsePacket() {}

CelestialResponsePacket::CelestialResponsePacket(List<CelestialResponse> responses) : responses(std::move(responses)) {}

std::expected<void, StarException> CelestialResponsePacket::read(DataStream& ds) {
  {
    auto result = ds.read(responses);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> CelestialResponsePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(responses);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

PlayerWarpResultPacket::PlayerWarpResultPacket() : warpActionInvalid(false) {}

PlayerWarpResultPacket::PlayerWarpResultPacket(bool success, WarpAction warpAction, bool warpActionInvalid)
  : success(success), warpAction(std::move(warpAction)), warpActionInvalid(warpActionInvalid) {}

std::expected<void, StarException> PlayerWarpResultPacket::read(DataStream& ds) {
  {
    auto result = ds.read(success);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(warpAction);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(warpActionInvalid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> PlayerWarpResultPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(success);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(warpAction);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(warpActionInvalid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

PlanetTypeUpdatePacket::PlanetTypeUpdatePacket() {}

PlanetTypeUpdatePacket::PlanetTypeUpdatePacket(CelestialCoordinate coordinate)
  : coordinate(coordinate) {}

std::expected<void, StarException> PlanetTypeUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(coordinate);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> PlanetTypeUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(coordinate);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

PausePacket::PausePacket() {}

PausePacket::PausePacket(bool pause, float timescale) : pause(pause), timescale(timescale) {}

std::expected<void, StarException> PausePacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  {
    auto result = ds.read(pause);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (!netRules.isLegacy())
    {
      auto result = ds.read(timescale);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  else
    timescale = 1.0f;
  return {};
}

std::expected<void, StarException> PausePacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  {
    auto result = ds.write(pause);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (!netRules.isLegacy())
    {
      auto result = ds.write(timescale);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  return {};
}

std::expected<void, StarException> PausePacket::readJson(Json const& json) {
  {
    auto result = json.getBool("pause");
    if (!result)
      return std::unexpected(std::move(result.error()));
    pause = std::move(*result);
  }
  {
    auto result = json.getFloat("timescale", 1.0f);
    if (!result)
      return std::unexpected(std::move(result.error()));
    timescale = std::move(*result);
  }
  return {};
}

std::expected<Json, StarException> PausePacket::writeJson() const {
  return JsonObject{
    {"pause", pause},
    {"timescale", timescale}
  };
}

ServerInfoPacket::ServerInfoPacket() {}

ServerInfoPacket::ServerInfoPacket(std::uint16_t players, std::uint16_t maxPlayers) :
  players(players),
  maxPlayers(maxPlayers) {}

std::expected<void, StarException> ServerInfoPacket::read(DataStream& ds)
{
  {
    auto result = ds.read(players);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(maxPlayers);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ServerInfoPacket::write(DataStream& ds) const
{
  {
    auto result = ds.write(players);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(maxPlayers);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ServerInfoPacket::readJson(Json const& json) {
  {
    auto result = json.getUInt("players");
    if (!result)
      return std::unexpected(std::move(result.error()));
    players = std::move(*result);
  }
  {
    auto result = json.getUInt("maxPlayers");
    if (!result)
      return std::unexpected(std::move(result.error()));
    maxPlayers = std::move(*result);
  }
  return {};
}

std::expected<Json, StarException> ServerInfoPacket::writeJson() const {
  return JsonObject{
    {"players", players},
    {"maxPlayers", maxPlayers}
  };
}

ClientConnectPacket::ClientConnectPacket() : playerUuid(unfilledPacketUuid()) {}

ClientConnectPacket::ClientConnectPacket(ByteArray assetsDigest, bool allowAssetsMismatch, Uuid playerUuid,
    String playerName, String shipSpecies, WorldChunks shipChunks, ShipUpgrades shipUpgrades,
    bool introComplete, String account, Json info)
  : assetsDigest(std::move(assetsDigest)), allowAssetsMismatch(allowAssetsMismatch), playerUuid(std::move(playerUuid)),
    playerName(std::move(playerName)), shipSpecies(std::move(shipSpecies)), shipChunks(std::move(shipChunks)),
    shipUpgrades(std::move(shipUpgrades)), introComplete(std::move(introComplete)), account(std::move(account)), info(std::move(info)) {}


std::expected<void, StarException> ClientConnectPacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  {
    auto result = ds.read(assetsDigest);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(allowAssetsMismatch);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(playerUuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(playerName);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(shipSpecies);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(shipChunks);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(shipUpgrades);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(introComplete);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(account);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (!netRules.isLegacy())
    {
      auto result = ds.read(info);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  return {};
}

std::expected<void, StarException> ClientConnectPacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  {
    auto result = ds.write(assetsDigest);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(allowAssetsMismatch);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(playerUuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(playerName);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(shipSpecies);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(shipChunks);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(shipUpgrades);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(introComplete);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(account);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (!netRules.isLegacy())
    {
      auto result = ds.write(info);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  return {};
}

ClientDisconnectRequestPacket::ClientDisconnectRequestPacket() {}

std::expected<void, StarException> ClientDisconnectRequestPacket::read(DataStream& ds) {
  // Packets cannot be empty due to the way packet serialization is handled.
  {
    auto result = ds.read<std::uint8_t>();
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ClientDisconnectRequestPacket::write(DataStream& ds) const {
  // Packets cannot be empty due to the way packet serialization is handled.
  {
    auto result = ds.write<std::uint8_t>(0);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

HandshakeResponsePacket::HandshakeResponsePacket() {}

HandshakeResponsePacket::HandshakeResponsePacket(ByteArray const& passHash) : passHash(passHash) {}

std::expected<void, StarException> HandshakeResponsePacket::read(DataStream& ds) {
  {
    auto result = ds.read(passHash);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> HandshakeResponsePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(passHash);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

PlayerWarpPacket::PlayerWarpPacket() {}

PlayerWarpPacket::PlayerWarpPacket(WarpAction action, bool deploy) : action(std::move(action)), deploy(std::move(deploy)) {}

std::expected<void, StarException> PlayerWarpPacket::read(DataStream& ds) {
  {
    auto result = ds.read(action);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(deploy);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> PlayerWarpPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(action);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(deploy);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

FlyShipPacket::FlyShipPacket() {}

FlyShipPacket::FlyShipPacket(Vec3I system, SystemLocation location, Json settings) : system(std::move(system)), location(std::move(location)), settings(std::move(settings)) {}

std::expected<void, StarException> FlyShipPacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  {
    auto result = ds.read(system);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(location);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (netRules.version() >= 3)
    {
      auto result = ds.read(settings);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  return {};
}

std::expected<void, StarException> FlyShipPacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  {
    auto result = ds.write(system);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(location);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (netRules.version() >= 3)
    {
      auto result = ds.write(settings);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  return {};
}

ChatSendPacket::ChatSendPacket() : sendMode(ChatSendMode::Broadcast) {}

ChatSendPacket::ChatSendPacket(String text, ChatSendMode sendMode) : text(std::move(text)), sendMode(sendMode) {}

ChatSendPacket::ChatSendPacket(String text, ChatSendMode sendMode, JsonObject data) : text(std::move(text)), sendMode(sendMode), data(std::move(data)) {}

std::expected<void, StarException> ChatSendPacket::read(DataStream& ds) {
  {
    auto result = ds.read(text);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(sendMode);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (ds.streamCompatibilityVersion() >= 5)
    {
      auto result = ds.read(data);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  return {};
}

std::expected<void, StarException> ChatSendPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(text);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(sendMode);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (ds.streamCompatibilityVersion() >= 5)
    {
      auto result = ds.write(data);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  return {};
}

CelestialRequestPacket::CelestialRequestPacket() {}

CelestialRequestPacket::CelestialRequestPacket(List<CelestialRequest> requests) : requests(std::move(requests)) {}

std::expected<void, StarException> CelestialRequestPacket::read(DataStream& ds) {
  {
    auto result = ds.read(requests);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> CelestialRequestPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(requests);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ClientContextUpdatePacket::ClientContextUpdatePacket() {}

ClientContextUpdatePacket::ClientContextUpdatePacket(ByteArray updateData) : updateData(std::move(updateData)) {}

std::expected<void, StarException> ClientContextUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(updateData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ClientContextUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(updateData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

WorldStartPacket::WorldStartPacket() : clientId(), localInterpolationMode() {}

std::expected<void, StarException> WorldStartPacket::read(DataStream& ds) {
  {
    auto result = ds.read(templateData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(skyData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(weatherData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(playerStart);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(playerRespawn);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(respawnInWorld);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(worldProperties);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(dungeonIdGravity);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(dungeonIdBreathable);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(protectedDungeonIds);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(clientId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(localInterpolationMode);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> WorldStartPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(templateData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(skyData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(weatherData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(playerStart);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(playerRespawn);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(respawnInWorld);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(worldProperties);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(dungeonIdGravity);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(dungeonIdBreathable);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(protectedDungeonIds);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(clientId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(localInterpolationMode);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

WorldStopPacket::WorldStopPacket() {}

WorldStopPacket::WorldStopPacket(String const& reason) : reason(reason) {}

std::expected<void, StarException> WorldStopPacket::read(DataStream& ds) {
  {
    auto result = ds.read(reason);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> WorldStopPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(reason);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

WorldLayoutUpdatePacket::WorldLayoutUpdatePacket() {}

WorldLayoutUpdatePacket::WorldLayoutUpdatePacket(Json const& layoutData) : layoutData(layoutData) {}

std::expected<void, StarException> WorldLayoutUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(layoutData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> WorldLayoutUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(layoutData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

WorldParametersUpdatePacket::WorldParametersUpdatePacket() {}

WorldParametersUpdatePacket::WorldParametersUpdatePacket(ByteArray const& parametersData) : parametersData(parametersData) {}

std::expected<void, StarException> WorldParametersUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(parametersData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> WorldParametersUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(parametersData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

CentralStructureUpdatePacket::CentralStructureUpdatePacket() {}

CentralStructureUpdatePacket::CentralStructureUpdatePacket(Json structureData) : structureData(std::move(structureData)) {}

std::expected<void, StarException> CentralStructureUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(structureData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> CentralStructureUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(structureData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

TileArrayUpdatePacket::TileArrayUpdatePacket() {}

std::expected<void, StarException> TileArrayUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.viread(min[0]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.viread(min[1]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }

  std::size_t width, height;
  {
    auto result = ds.vuread(width);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.vuread(height);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  array.resize(width, height);
  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x)
      {
        auto result = ds.read(array(x, y));
        if (!result)
          return std::unexpected(std::move(result.error()));
      }
  }
  return {};
}

std::expected<void, StarException> TileArrayUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.viwrite(min[0]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.viwrite(min[1]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.vuwrite(array.size(0));
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.vuwrite(array.size(1));
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  for (std::size_t y = 0; y < array.size(1); ++y) {
    for (std::size_t x = 0; x < array.size(0); ++x)
      {
        auto result = ds.write(array(x, y));
        if (!result)
          return std::unexpected(std::move(result.error()));
      }
  }
  return {};
}

std::expected<void, StarException> TileUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.viread(position[0]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.viread(position[1]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(tile);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> TileUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.viwrite(position[0]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.viwrite(position[1]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(tile);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

TileLiquidUpdatePacket::TileLiquidUpdatePacket() {}

TileLiquidUpdatePacket::TileLiquidUpdatePacket(Vec2I const& position, LiquidNetUpdate liquidUpdate)
  : position(position), liquidUpdate(liquidUpdate) {}

std::expected<void, StarException> TileLiquidUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.viread(position[0]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.viread(position[1]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(liquidUpdate.liquid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(liquidUpdate.level);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> TileLiquidUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.viwrite(position[0]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.viwrite(position[1]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(liquidUpdate.liquid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(liquidUpdate.level);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

TileDamageUpdatePacket::TileDamageUpdatePacket() : layer(TileLayer::Foreground) {}

TileDamageUpdatePacket::TileDamageUpdatePacket(
    Vec2I const& position, TileLayer layer, TileDamageStatus const& tileDamage)
  : position(position), layer(layer), tileDamage(tileDamage) {}

std::expected<void, StarException> TileDamageUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(position);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(layer);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(tileDamage);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> TileDamageUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(position);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(layer);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(tileDamage);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

TileModificationFailurePacket::TileModificationFailurePacket() {}

TileModificationFailurePacket::TileModificationFailurePacket(TileModificationList modifications)
  : modifications(modifications) {}

std::expected<void, StarException> TileModificationFailurePacket::read(DataStream& ds) {
  {
    auto result = ds.readContainer(modifications);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> TileModificationFailurePacket::write(DataStream& ds) const {
  {
    auto result = ds.writeContainer(modifications);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

GiveItemPacket::GiveItemPacket() {}

GiveItemPacket::GiveItemPacket(ItemDescriptor const& item) : item(item) {}

std::expected<void, StarException> GiveItemPacket::read(DataStream& ds) {
  {
    auto result = ds.read(item);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> GiveItemPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(item);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> GiveItemPacket::readJson(Json const& json) {
  auto data = json.get("item");
  if (!data)
    return std::unexpected(std::move(data.error()));
  auto decoded = ItemDescriptor::create(*data);
  if (!decoded)
    return std::unexpected(std::move(decoded.error()));
  item = std::move(*decoded);
  return {};
}

std::expected<Json, StarException> GiveItemPacket::writeJson() const {
  return JsonObject{
    {"item", item.toJson()}
  };
}

EnvironmentUpdatePacket::EnvironmentUpdatePacket() {}

EnvironmentUpdatePacket::EnvironmentUpdatePacket(ByteArray skyDelta, ByteArray weatherDelta)
  : skyDelta(std::move(skyDelta)), weatherDelta(std::move(weatherDelta)) {}

std::expected<void, StarException> EnvironmentUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(skyDelta);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(weatherDelta);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> EnvironmentUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(skyDelta);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(weatherDelta);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ModifyTileListPacket::ModifyTileListPacket() : allowEntityOverlap() {}

ModifyTileListPacket::ModifyTileListPacket(TileModificationList modifications, bool allowEntityOverlap)
  : modifications(modifications), allowEntityOverlap(allowEntityOverlap) {}

std::expected<void, StarException> ModifyTileListPacket::read(DataStream& ds) {
  {
    auto result = ds.readContainer(modifications);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(allowEntityOverlap);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ModifyTileListPacket::write(DataStream& ds) const {
  {
    auto result = ds.writeContainer(modifications);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(allowEntityOverlap);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ReplaceTileListPacket::ReplaceTileListPacket() : applyDamage() {}

ReplaceTileListPacket::ReplaceTileListPacket(TileModificationList modifications, TileDamage tileDamage, bool applyDamage)
  : modifications(modifications), tileDamage(std::move(tileDamage)), applyDamage(applyDamage) {}

std::expected<void, StarException> ReplaceTileListPacket::read(DataStream& ds) {
  {
    auto result = ds.readContainer(modifications);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(tileDamage);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (ds.streamCompatibilityVersion() >= 7)
    {
      auto result = ds.read(applyDamage);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  else
    applyDamage = false;
  return {};
}

std::expected<void, StarException> ReplaceTileListPacket::write(DataStream& ds) const {
  {
    auto result = ds.writeContainer(modifications);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(tileDamage);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  if (ds.streamCompatibilityVersion() >= 7)
    {
      auto result = ds.write(applyDamage);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  return {};
}

DamageTileGroupPacket::DamageTileGroupPacket() : layer(TileLayer::Foreground) {}

DamageTileGroupPacket::DamageTileGroupPacket(
    List<Vec2I> tilePositions, TileLayer layer, Vec2F sourcePosition, TileDamage tileDamage, std::optional<EntityId> sourceEntity)
  : tilePositions(std::move(tilePositions)), layer(layer), sourcePosition(sourcePosition), tileDamage(std::move(tileDamage)), sourceEntity(std::move(sourceEntity)) {}

std::expected<void, StarException> DamageTileGroupPacket::read(DataStream& ds) {
  {
    auto result = ds.readContainer(tilePositions);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(layer);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(sourcePosition);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(tileDamage);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(sourceEntity);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> DamageTileGroupPacket::write(DataStream& ds) const {
  {
    auto result = ds.writeContainer(tilePositions);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(layer);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(sourcePosition);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(tileDamage);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(sourceEntity);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

CollectLiquidPacket::CollectLiquidPacket() {}

CollectLiquidPacket::CollectLiquidPacket(List<Vec2I> tilePositions, LiquidId liquidId)
  : tilePositions(std::move(tilePositions)), liquidId(liquidId) {}

std::expected<void, StarException> CollectLiquidPacket::read(DataStream& ds) {
  {
    auto result = ds.readContainer(tilePositions);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(liquidId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> CollectLiquidPacket::write(DataStream& ds) const {
  {
    auto result = ds.writeContainer(tilePositions);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(liquidId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

RequestDropPacket::RequestDropPacket() {
  dropEntityId = NullEntityId;
}

RequestDropPacket::RequestDropPacket(EntityId dropEntityId) : dropEntityId(dropEntityId) {}

std::expected<void, StarException> RequestDropPacket::read(DataStream& ds) {
  {
    auto result = ds.viread(dropEntityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> RequestDropPacket::write(DataStream& ds) const {
  {
    auto result = ds.viwrite(dropEntityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

SpawnEntityPacket::SpawnEntityPacket() {}

SpawnEntityPacket::SpawnEntityPacket(EntityType entityType, ByteArray storeData, ByteArray firstNetState)
  : entityType(entityType), storeData(std::move(storeData)), firstNetState(std::move(firstNetState)) {}

std::expected<void, StarException> SpawnEntityPacket::read(DataStream& ds) {
  {
    auto result = ds.read(entityType);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(storeData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(firstNetState);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SpawnEntityPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(entityType);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(storeData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(firstNetState);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

EntityInteractPacket::EntityInteractPacket() : requestId(unfilledPacketUuid()) {}

EntityInteractPacket::EntityInteractPacket(InteractRequest interactRequest, Uuid requestId)
  : interactRequest(interactRequest), requestId(requestId) {}

std::expected<void, StarException> EntityInteractPacket::read(DataStream& ds) {
  {
    auto result = ds.read(interactRequest);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(requestId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> EntityInteractPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(interactRequest);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(requestId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

EntityInteractResultPacket::EntityInteractResultPacket() : requestId(unfilledPacketUuid()) {}

EntityInteractResultPacket::EntityInteractResultPacket(InteractAction action, Uuid requestId, EntityId sourceEntityId)
  : action(action), requestId(requestId), sourceEntityId(sourceEntityId) {}

std::expected<void, StarException> EntityInteractResultPacket::read(DataStream& ds) {
  {
    auto result = ds.read(action);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(requestId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(sourceEntityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> EntityInteractResultPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(action);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(requestId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(sourceEntityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

EntityCreatePacket::EntityCreatePacket() {
  entityId = NullEntityId;
}

ServerDisconnectPacket::ServerDisconnectPacket() {}

ServerDisconnectPacket::ServerDisconnectPacket(String reason) : reason(std::move(reason)) {}

std::expected<void, StarException> ServerDisconnectPacket::read(DataStream& ds) {
  {
    auto result = ds.read(reason);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ServerDisconnectPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(reason);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ConnectWirePacket::ConnectWirePacket() {}

ConnectWirePacket::ConnectWirePacket(WireConnection outputConnection, WireConnection inputConnection)
  : outputConnection(outputConnection), inputConnection(inputConnection) {}

std::expected<void, StarException> ConnectWirePacket::read(DataStream& ds) {
  {
    auto result = ds.read(outputConnection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(inputConnection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ConnectWirePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(outputConnection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(inputConnection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

DisconnectAllWiresPacket::DisconnectAllWiresPacket() {}

DisconnectAllWiresPacket::DisconnectAllWiresPacket(Vec2I entityPosition, WireNode wireNode)
  : entityPosition(entityPosition), wireNode(wireNode) {}

std::expected<void, StarException> DisconnectAllWiresPacket::read(DataStream& ds) {
  {
    auto result = ds.viread(entityPosition[0]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.viread(entityPosition[1]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(wireNode);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> DisconnectAllWiresPacket::write(DataStream& ds) const {
  {
    auto result = ds.viwrite(entityPosition[0]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.viwrite(entityPosition[1]);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(wireNode);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

WorldClientStateUpdatePacket::WorldClientStateUpdatePacket() {}

WorldClientStateUpdatePacket::WorldClientStateUpdatePacket(ByteArray const& worldClientStateDelta)
  : worldClientStateDelta(worldClientStateDelta) {}

std::expected<void, StarException> WorldClientStateUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(worldClientStateDelta);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> WorldClientStateUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(worldClientStateDelta);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

FindUniqueEntityPacket::FindUniqueEntityPacket() {}

FindUniqueEntityPacket::FindUniqueEntityPacket(String uniqueEntityId)
  : uniqueEntityId(std::move(uniqueEntityId)) {}

std::expected<void, StarException> FindUniqueEntityPacket::read(DataStream& ds) {
  {
    auto result = ds.read(uniqueEntityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> FindUniqueEntityPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(uniqueEntityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

WorldStartAcknowledgePacket::WorldStartAcknowledgePacket() {}

std::expected<void, StarException> WorldStartAcknowledgePacket::read(DataStream& ds) {
  // Packets can't be empty, read the trash data
  {
    auto result = ds.read<bool>();
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> WorldStartAcknowledgePacket::write(DataStream& ds) const {
  // Packets can't be empty, write some trash data
  {
    auto result = ds.write<bool>(false);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

PingPacket::PingPacket() {}
PingPacket::PingPacket(std::int64_t time) : time(time) {}

std::expected<void, StarException> PingPacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  if (netRules.isLegacy()) {
    // Packets can't be empty, read the trash data
    {
      auto result = ds.read<bool>();
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
    time = 0;
  } else {
    {
      auto result = ds.readVlqI(time);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  }
  return {};
}

std::expected<void, StarException> PingPacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  if (netRules.isLegacy()) {
    // Packets can't be empty, write some trash data
    {
      auto result = ds.write<bool>(false);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  } else {
    {
      auto result = ds.writeVlqI(time);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  }
  return {};
}

EntityCreatePacket::EntityCreatePacket(EntityType entityType, ByteArray storeData, ByteArray firstNetState, EntityId entityId)
  : entityType(entityType), storeData(std::move(storeData)), firstNetState(std::move(firstNetState)), entityId(entityId) {}

std::expected<void, StarException> EntityCreatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(entityType);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(storeData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(firstNetState);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.viread(entityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> EntityCreatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(entityType);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(storeData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(firstNetState);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.viwrite(entityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

EntityUpdateSetPacket::EntityUpdateSetPacket() : EntityUpdateSetPacket(ServerConnectionId) {}

EntityUpdateSetPacket::EntityUpdateSetPacket(ConnectionId forConnection) : forConnection(forConnection) {}

std::expected<void, StarException> EntityUpdateSetPacket::read(DataStream& ds) {
  {
    auto result = ds.vuread(forConnection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  auto decoded = ds.readMapContainer(deltas,
      [](DataStream& ds, EntityId& entityId, ByteArray& delta) -> std::expected<void, StarException> {
        auto id = ds.viread(entityId);
        if (!id)
          return std::unexpected(std::move(id.error()));
        return ds.read(delta);
      });
  if (!decoded)
    return std::unexpected(std::move(decoded.error()));
  return {};
}

std::expected<void, StarException> EntityUpdateSetPacket::write(DataStream& ds) const {
  {
    auto result = ds.vuwrite(forConnection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  auto encoded = ds.writeMapContainer(deltas,
      [](DataStream& ds, EntityId const& entityId, ByteArray const& delta) -> std::expected<void, StarException> {
        auto id = ds.viwrite(entityId);
        if (!id)
          return std::unexpected(std::move(id.error()));
        return ds.write(delta);
      });
  if (!encoded)
    return std::unexpected(std::move(encoded.error()));
  return {};
}

EntityDestroyPacket::EntityDestroyPacket() {
  entityId = NullEntityId;
  death = false;
}

EntityDestroyPacket::EntityDestroyPacket(EntityId entityId, ByteArray finalNetState, bool death)
  : entityId(entityId), finalNetState(std::move(finalNetState)), death(death) {}

std::expected<void, StarException> EntityDestroyPacket::read(DataStream& ds) {
  {
    auto result = ds.viread(entityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(finalNetState);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(death);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> EntityDestroyPacket::write(DataStream& ds) const {
  {
    auto result = ds.viwrite(entityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(finalNetState);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(death);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

HitRequestPacket::HitRequestPacket() {}

HitRequestPacket::HitRequestPacket(RemoteHitRequest remoteHitRequest) : remoteHitRequest(std::move(remoteHitRequest)) {}

std::expected<void, StarException> HitRequestPacket::read(DataStream& ds) {
  {
    auto result = ds.read(remoteHitRequest);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> HitRequestPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(remoteHitRequest);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

DamageRequestPacket::DamageRequestPacket() {}

DamageRequestPacket::DamageRequestPacket(RemoteDamageRequest remoteDamageRequest)
  : remoteDamageRequest(std::move(remoteDamageRequest)) {}

std::expected<void, StarException> DamageRequestPacket::read(DataStream& ds) {
  {
    auto result = ds.read(remoteDamageRequest);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> DamageRequestPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(remoteDamageRequest);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

DamageNotificationPacket::DamageNotificationPacket() {}

DamageNotificationPacket::DamageNotificationPacket(RemoteDamageNotification remoteDamageNotification)
  : remoteDamageNotification(std::move(remoteDamageNotification)) {}

std::expected<void, StarException> DamageNotificationPacket::read(DataStream& ds) {
  {
    auto result = ds.read(remoteDamageNotification);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> DamageNotificationPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(remoteDamageNotification);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

EntityMessagePacket::EntityMessagePacket() : uuid(unfilledPacketUuid()) {}

EntityMessagePacket::EntityMessagePacket(std::variant<EntityId, String> entityId, String message, JsonArray args, Uuid uuid, ConnectionId fromConnection)
  : entityId(entityId), message(std::move(message)), args(std::move(args)), uuid(uuid), fromConnection(fromConnection) {}

std::expected<void, StarException> EntityMessagePacket::read(DataStream& ds) {
  {
    auto result = ds.read(entityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(message);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(args);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(uuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(fromConnection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> EntityMessagePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(entityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(message);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(args);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(uuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(fromConnection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> EntityMessagePacket::readJson(Json const& json) {
  auto jEntityId = json.get("entityId");
  if (!jEntityId)
    return std::unexpected(std::move(jEntityId.error()));
  if (jEntityId->canConvert(Json::Type::Int)) {
    auto id = jEntityId->toInt();
    if (!id)
      return std::unexpected(std::move(id.error()));
    entityId = (EntityId)*id;
  } else {
    auto id = jEntityId->toString();
    if (!id)
      return std::unexpected(std::move(id.error()));
    entityId = std::move(*id);
  }
  auto decodedMessage = json.getString("message");
  if (!decodedMessage)
    return std::unexpected(std::move(decodedMessage.error()));
  message = std::move(*decodedMessage);
  auto decodedArgs = json.getArray("args");
  if (!decodedArgs)
    return std::unexpected(std::move(decodedArgs.error()));
  args = std::move(*decodedArgs);
  auto uuidString = json.getString("uuid");
  if (!uuidString)
    return std::unexpected(std::move(uuidString.error()));
  auto decodedUuid = Uuid::create(*uuidString);
  if (!decodedUuid)
    return std::unexpected(std::move(decodedUuid.error()));
  uuid = std::move(*decodedUuid);
  auto from = json.getUInt("fromConnection");
  if (!from)
    return std::unexpected(std::move(from.error()));
  fromConnection = *from;
  return {};
}

std::expected<Json, StarException> EntityMessagePacket::writeJson() const {
  return JsonObject{
    {"entityId", std::holds_alternative<EntityId>(entityId) ? Json(std::get<EntityId>(entityId)) : Json(std::get<String>(entityId))},
    {"message", message},
    {"args", args},
    {"uuid", uuid.hex()},
    {"fromConnection", fromConnection}
  };
}

EntityMessageResponsePacket::EntityMessageResponsePacket() : uuid(unfilledPacketUuid()) {}

EntityMessageResponsePacket::EntityMessageResponsePacket(Either<String, Json> response, Uuid uuid)
  : response(std::move(response)), uuid(uuid) {}

std::expected<void, StarException> EntityMessageResponsePacket::read(DataStream& ds) {
  {
    auto result = ds.read(response);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(uuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> EntityMessageResponsePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(response);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(uuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

UpdateWorldPropertiesPacket::UpdateWorldPropertiesPacket() {}

UpdateWorldPropertiesPacket::UpdateWorldPropertiesPacket(JsonObject const& updatedProperties)
  : updatedProperties(updatedProperties) {}

std::expected<void, StarException> UpdateWorldPropertiesPacket::read(DataStream& ds) {
  {
    auto result = ds.readMapContainer(updatedProperties);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> UpdateWorldPropertiesPacket::write(DataStream& ds) const {
  {
    auto result = ds.writeMapContainer(updatedProperties);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}


std::expected<void, StarException> UpdateWorldPropertiesPacket::readJson(Json const& json) {
  {
    auto result = json.getObject("updatedProperties");
    if (!result)
      return std::unexpected(std::move(result.error()));
    updatedProperties = std::move(*result);
  }
  return {};
}

std::expected<Json, StarException> UpdateWorldPropertiesPacket::writeJson() const {
  return JsonObject{
    {"updatedProperties", updatedProperties},
  };
}

UpdateTileProtectionPacket::UpdateTileProtectionPacket() {}

UpdateTileProtectionPacket::UpdateTileProtectionPacket(DungeonId dungeonId, bool isProtected)
  : dungeonId(dungeonId), isProtected(isProtected) {}

std::expected<void, StarException> UpdateTileProtectionPacket::read(DataStream& ds) {
  {
    auto result = ds.read(dungeonId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(isProtected);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> UpdateTileProtectionPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(dungeonId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(isProtected);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> UpdateTileProtectionPacket::readJson(Json const& json) {
  {
    auto result = json.getUInt("dungeonId");
    if (!result)
      return std::unexpected(std::move(result.error()));
    dungeonId = std::move(*result);
  }
  {
    auto result = json.getBool("isProtected");
    if (!result)
      return std::unexpected(std::move(result.error()));
    isProtected = std::move(*result);
  }
  return {};
}

std::expected<Json, StarException> UpdateTileProtectionPacket::writeJson() const {
  return JsonObject{
    {"dungeonId", dungeonId},
    {"isProtected", isProtected}
  };
}

SetDungeonGravityPacket::SetDungeonGravityPacket() {}

SetDungeonGravityPacket::SetDungeonGravityPacket(DungeonId dungeonId, std::optional<float> gravity)
  : dungeonId(std::move(dungeonId)), gravity(std::move(gravity)) {}

std::expected<void, StarException> SetDungeonGravityPacket::read(DataStream& ds) {
  {
    auto result = ds.read(dungeonId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(gravity);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SetDungeonGravityPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(dungeonId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(gravity);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SetDungeonGravityPacket::readJson(Json const& json) {
  {
    auto result = json.getUInt("dungeonId");
    if (!result)
      return std::unexpected(std::move(result.error()));
    dungeonId = std::move(*result);
  }
  {
    auto result = json.optFloat("gravity");
    if (!result)
      return std::unexpected(std::move(result.error()));
    gravity = std::move(*result);
  }
  return {};
}

std::expected<Json, StarException> SetDungeonGravityPacket::writeJson() const {
  return JsonObject{
    {"dungeonId", dungeonId},
    {"gravity", jsonFromOptional<float>(gravity)}
  };
}

SetDungeonBreathablePacket::SetDungeonBreathablePacket() {}

SetDungeonBreathablePacket::SetDungeonBreathablePacket(DungeonId dungeonId, std::optional<bool> breathable)
  : dungeonId(std::move(dungeonId)), breathable(std::move(breathable)) {}

std::expected<void, StarException> SetDungeonBreathablePacket::read(DataStream& ds) {
  {
    auto result = ds.read(dungeonId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(breathable);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SetDungeonBreathablePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(dungeonId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(breathable);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SetDungeonBreathablePacket::readJson(Json const& json) {
  {
    auto result = json.getUInt("dungeonId");
    if (!result)
      return std::unexpected(std::move(result.error()));
    dungeonId = std::move(*result);
  }
  {
    auto result = json.optBool("breathable");
    if (!result)
      return std::unexpected(std::move(result.error()));
    breathable = std::move(*result);
  }
  return {};
}

std::expected<Json, StarException> SetDungeonBreathablePacket::writeJson() const {
  return JsonObject{
    {"dungeonId", dungeonId},
    {"breathable", jsonFromOptional<bool>(breathable)}
  };
}

SetPlayerStartPacket::SetPlayerStartPacket() {}

SetPlayerStartPacket::SetPlayerStartPacket(Vec2F playerStart, bool respawnInWorld) : playerStart(playerStart), respawnInWorld(respawnInWorld) {}

std::expected<void, StarException> SetPlayerStartPacket::read(DataStream& ds) {
  {
    auto result = ds.read(playerStart);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(respawnInWorld);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SetPlayerStartPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(playerStart);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(respawnInWorld);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SetPlayerStartPacket::readJson(Json const& json) {
  auto startData = json.get("playerStart");
  if (!startData)
    return std::unexpected(std::move(startData.error()));
  auto start = jsonToVec2F(*startData);
  if (!start)
    return std::unexpected(std::move(start.error()));
  playerStart = *start;
  {
    auto result = json.getBool("respawnInWorld");
    if (!result)
      return std::unexpected(std::move(result.error()));
    respawnInWorld = std::move(*result);
  }
  return {};
}

std::expected<Json, StarException> SetPlayerStartPacket::writeJson() const {
  return JsonObject{
    {"playerStart", jsonFromVec2F(playerStart)},
    {"respawnInWorld", respawnInWorld}
  };
}

FindUniqueEntityResponsePacket::FindUniqueEntityResponsePacket() {}

FindUniqueEntityResponsePacket::FindUniqueEntityResponsePacket(String uniqueEntityId, std::optional<Vec2F> entityPosition)
  : uniqueEntityId(std::move(uniqueEntityId)), entityPosition(std::move(entityPosition)) {}

std::expected<void, StarException> FindUniqueEntityResponsePacket::read(DataStream& ds) {
  {
    auto result = ds.read(uniqueEntityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(entityPosition);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> FindUniqueEntityResponsePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(uniqueEntityId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(entityPosition);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

PongPacket::PongPacket() {}
PongPacket::PongPacket(std::int64_t time) : time(time) {}


std::expected<void, StarException> PongPacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  if (netRules.isLegacy()) {
    // Packets can't be empty, read the trash data
    {
      auto result = ds.read<bool>();
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
    time = 0;
  } else {
    {
      auto result = ds.readVlqI(time);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  }

  return {};
}

std::expected<void, StarException> PongPacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  if (netRules.isLegacy()) {
    // Packets can't be empty, write some trash data
    {
      auto result = ds.write<bool>(false);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  } else {
    {
      auto result = ds.writeVlqI(time);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  }
  return {};
}

StepUpdatePacket::StepUpdatePacket() : remoteTime(0.0) {}

StepUpdatePacket::StepUpdatePacket(double remoteTime) : remoteTime(remoteTime) {}


std::expected<void, StarException> StepUpdatePacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  if (netRules.isLegacy()) {
    auto steps = ds.readVlqU();
    if (!steps)
      return std::unexpected(std::move(steps.error()));
    remoteTime = double(*steps) / 60.0;
  } else {
    {
      auto result = ds.read(remoteTime);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  }
  return {};
}

std::expected<void, StarException> StepUpdatePacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  if (netRules.isLegacy()) {
    {
      auto result = ds.writeVlqU((std::uint64_t)std::round(remoteTime * 60.0));
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  } else {
    {
      auto result = ds.write(remoteTime);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
  }
  return {};
}

SystemWorldStartPacket::SystemWorldStartPacket() : clientShip(unfilledPacketUuid(), SystemLocation()) {}

SystemWorldStartPacket::SystemWorldStartPacket(Vec3I location, List<ByteArray> objectStores, List<ByteArray> shipStores, std::pair<Uuid, SystemLocation> clientShip)
  : location(std::move(location)), objectStores(std::move(objectStores)), shipStores(std::move(shipStores)), clientShip(std::move(clientShip)) {}

std::expected<void, StarException> SystemWorldStartPacket::read(DataStream& ds) {
  {
    auto result = ds.read(location);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(objectStores);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(shipStores);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(clientShip);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SystemWorldStartPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(location);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(objectStores);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(shipStores);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(clientShip);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

SystemWorldUpdatePacket::SystemWorldUpdatePacket() {}

SystemWorldUpdatePacket::SystemWorldUpdatePacket(HashMap<Uuid, ByteArray> objectUpdates, HashMap<Uuid, ByteArray> shipUpdates)
  : objectUpdates(std::move(objectUpdates)), shipUpdates(std::move(shipUpdates)) {}

std::expected<void, StarException> SystemWorldUpdatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(objectUpdates);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(shipUpdates);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SystemWorldUpdatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(objectUpdates);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(shipUpdates);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

SystemObjectCreatePacket::SystemObjectCreatePacket() {}

SystemObjectCreatePacket::SystemObjectCreatePacket(ByteArray objectStore) : objectStore(std::move(objectStore)) {}

std::expected<void, StarException> SystemObjectCreatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(objectStore);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SystemObjectCreatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(objectStore);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

SystemObjectDestroyPacket::SystemObjectDestroyPacket() : objectUuid(unfilledPacketUuid()) {}

SystemObjectDestroyPacket::SystemObjectDestroyPacket(Uuid objectUuid) : objectUuid(std::move(objectUuid)) {}

std::expected<void, StarException> SystemObjectDestroyPacket::read(DataStream& ds) {
  {
    auto result = ds.read(objectUuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SystemObjectDestroyPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(objectUuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

SystemShipCreatePacket::SystemShipCreatePacket() {}

SystemShipCreatePacket::SystemShipCreatePacket(ByteArray shipStore) : shipStore(std::move(shipStore)) {}

std::expected<void, StarException> SystemShipCreatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(shipStore);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SystemShipCreatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(shipStore);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

SystemShipDestroyPacket::SystemShipDestroyPacket() : shipUuid(unfilledPacketUuid()) {}

SystemShipDestroyPacket::SystemShipDestroyPacket(Uuid shipUuid) : shipUuid(std::move(shipUuid)) {}

std::expected<void, StarException> SystemShipDestroyPacket::read(DataStream& ds) {
  {
    auto result = ds.read(shipUuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SystemShipDestroyPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(shipUuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

SystemObjectSpawnPacket::SystemObjectSpawnPacket() : uuid(unfilledPacketUuid()) {}

SystemObjectSpawnPacket::SystemObjectSpawnPacket(String typeName, Uuid uuid, std::optional<Vec2F> position, JsonObject parameters)
  : typeName(std::move(typeName)), uuid(std::move(uuid)), position(std::move(position)), parameters(std::move(parameters)) {}

std::expected<void, StarException> SystemObjectSpawnPacket::read(DataStream& ds) {
  {
    auto result = ds.read(typeName);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(uuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(position);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(parameters);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> SystemObjectSpawnPacket::write(DataStream& ds) const {
  {
    auto result = ds.write(typeName);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(uuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(position);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(parameters);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

UpdateWorldTemplatePacket::UpdateWorldTemplatePacket() {}

UpdateWorldTemplatePacket::UpdateWorldTemplatePacket(Json templateData) : templateData(std::move(templateData)) {}

std::expected<void, StarException> UpdateWorldTemplatePacket::read(DataStream& ds) {
  {
    auto result = ds.read(templateData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> UpdateWorldTemplatePacket::write(DataStream& ds) const {
  {
    auto result = ds.write(templateData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}



ClientCustomWorldRequest::ClientCustomWorldRequest() {}

ClientCustomWorldRequest::ClientCustomWorldRequest(String name) : name(std::move(name)) {}

std::expected<void, StarException> ClientCustomWorldRequest::read(DataStream& ds) {
  {
    auto result = ds.read(name);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ClientCustomWorldRequest::write(DataStream& ds) const {
  {
    auto result = ds.write(name);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ClientCustomWorldResponse::ClientCustomWorldResponse() {}

ClientCustomWorldResponse::ClientCustomWorldResponse(String name, WorldChunks chunks) : name(std::move(name)), chunks(std::move(chunks)) {}

std::expected<void, StarException> ClientCustomWorldResponse::read(DataStream& ds) {
  {
    auto result = ds.read(name);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(chunks);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ClientCustomWorldResponse::write(DataStream& ds) const {
  {
    auto result = ds.write(name);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(chunks);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ClientCustomWorldCreate::ClientCustomWorldCreate() {}

ClientCustomWorldCreate::ClientCustomWorldCreate(String name, Json templateData) : name(std::move(name)), templateData(std::move(templateData)) {}

std::expected<void, StarException> ClientCustomWorldCreate::read(DataStream& ds) {
  {
    auto result = ds.read(name);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(templateData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ClientCustomWorldCreate::write(DataStream& ds) const {
  {
    auto result = ds.write(name);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(templateData);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ClientSubWorldPackets::ClientSubWorldPackets() {}

ClientSubWorldPackets::ClientSubWorldPackets(ClientSubWorldId const& subWorldId, List<std::shared_ptr<Packet>> packets) : subWorldId(subWorldId), packets(std::move(packets)) {}

std::expected<void, StarException> ClientSubWorldPackets::read(DataStream& ds) {
  {
    auto result = ds.read(subWorldId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  // note: this approach may have an issue with packet size
  auto count = ds.read<std::uint32_t>();
  if (!count)
    return std::unexpected(std::move(count.error()));
  for (std::uint32_t i = 0; i < *count; i++) {
    auto packetType = ds.read<PacketType>();
    if (!packetType)
      return std::unexpected(std::move(packetType.error()));
    auto created = createPacket(*packetType);
    if (!created)
      return std::unexpected(std::move(created.error()));
    auto packet = std::move(*created);
    auto read = packet->read(ds);
    if (!read)
      return std::unexpected(std::move(read.error()));
    packets.append(std::move(packet));
  }
  return {};
}

std::expected<void, StarException> ClientSubWorldPackets::write(DataStream& ds) const {
  {
    auto result = ds.write(subWorldId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write<std::uint32_t>(packets.count());
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  for (std::shared_ptr<Packet> const& packet : packets) {
    auto packetType = packet->type();
    {
      auto result = ds.write(packetType);
      if (!result)
        return std::unexpected(std::move(result.error()));
    }
    auto written = packet->write(ds);
    if (!written)
      return std::unexpected(std::move(written.error()));
  }
  return {};
}

ClientSubWorldRequest::ClientSubWorldRequest() {}

ClientSubWorldRequest::ClientSubWorldRequest(ClientSubWorldId const& subWorldId, WorldId worldId) : subWorldId(subWorldId), worldId(std::move(worldId)) {}

std::expected<void, StarException> ClientSubWorldRequest::read(DataStream& ds) {
  {
    auto result = ds.read(subWorldId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(worldId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ClientSubWorldRequest::write(DataStream& ds) const {
  {
    auto result = ds.write(subWorldId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(worldId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

ClientSubWorldReject::ClientSubWorldReject() {}

ClientSubWorldReject::ClientSubWorldReject(ClientSubWorldId const& subWorldId) : subWorldId(subWorldId) {}

std::expected<void, StarException> ClientSubWorldReject::read(DataStream& ds) {
  {
    auto result = ds.read(subWorldId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> ClientSubWorldReject::write(DataStream& ds) const {
  {
    auto result = ds.write(subWorldId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

NotifyWorldLoad::NotifyWorldLoad() {}

NotifyWorldLoad::NotifyWorldLoad(WorldId worldId) : worldId(std::move(worldId)) {}

std::expected<void, StarException> NotifyWorldLoad::read(DataStream& ds) {
  {
    auto result = ds.read(worldId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> NotifyWorldLoad::write(DataStream& ds) const {
  {
    auto result = ds.write(worldId);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

LogMapUpdate::LogMapUpdate() {}

LogMapUpdate::LogMapUpdate(Map<String,String> map) : map(std::move(map)) {}

std::expected<void, StarException> LogMapUpdate::read(DataStream& ds) {
  {
    auto result = ds.read(map);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> LogMapUpdate::write(DataStream& ds) const {
  {
    auto result = ds.write(map);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

UniverseMessage::UniverseMessage() : uuid(unfilledPacketUuid()) {}

UniverseMessage::UniverseMessage(ConnectionId connection, String message, JsonArray args, Uuid uuid, ConnectionId fromConnection)
  : connection(connection), message(std::move(message)), args(std::move(args)), uuid(uuid), fromConnection(fromConnection) {}

std::expected<void, StarException> UniverseMessage::read(DataStream& ds) {
  {
    auto result = ds.read(connection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(message);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(args);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(uuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(fromConnection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> UniverseMessage::write(DataStream& ds) const {
  {
    auto result = ds.write(connection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(message);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(args);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(uuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(fromConnection);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> UniverseMessage::readJson(Json const& json) {
  {
    auto result = json.getUInt("connection");
    if (!result)
      return std::unexpected(std::move(result.error()));
    connection = std::move(*result);
  }
  {
    auto result = json.getString("message");
    if (!result)
      return std::unexpected(std::move(result.error()));
    message = std::move(*result);
  }
  {
    auto result = json.getArray("args");
    if (!result)
      return std::unexpected(std::move(result.error()));
    args = std::move(*result);
  }
  auto uuidString = json.getString("uuid");
  if (!uuidString)
    return std::unexpected(std::move(uuidString.error()));
  auto decodedUuid = Uuid::create(*uuidString);
  if (!decodedUuid)
    return std::unexpected(std::move(decodedUuid.error()));
  uuid = std::move(*decodedUuid);
  {
    auto result = json.getUInt("fromConnection");
    if (!result)
      return std::unexpected(std::move(result.error()));
    fromConnection = std::move(*result);
  }
  return {};
}

std::expected<Json, StarException> UniverseMessage::writeJson() const {
  return JsonObject{
    {"connection", connection},
    {"message", message},
    {"args", args},
    {"uuid", uuid.hex()},
    {"fromConnection", fromConnection}
  };
}

UniverseMessageResponse::UniverseMessageResponse() : uuid(unfilledPacketUuid()) {}

UniverseMessageResponse::UniverseMessageResponse(Either<String, Json> response, Uuid uuid)
  : response(std::move(response)), uuid(uuid) {}

std::expected<void, StarException> UniverseMessageResponse::read(DataStream& ds) {
  {
    auto result = ds.read(response);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.read(uuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

std::expected<void, StarException> UniverseMessageResponse::write(DataStream& ds) const {
  {
    auto result = ds.write(response);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  {
    auto result = ds.write(uuid);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return {};
}

}
