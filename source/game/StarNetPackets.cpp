#include "StarNetPackets.hpp"

#include "StarCasting.hpp"
#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"

import std;

namespace Star {

VersionNumber const StarProtocolVersion = 747;

EnumMap<PacketType> const PacketTypeNames{
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
  {PacketType::UpdateWorldTemplate, "UpdateWorldTemplate"}};

EnumMap<NetCompressionMode> const NetCompressionModeNames{
  {NetCompressionMode::None, "None"},
  {NetCompressionMode::Zstd, "Zstd"}};

Packet::~Packet() = default;

void Packet::read(DataStream& ds, [[maybe_unused]] NetCompatibilityRules netRules) { read(ds); }
void Packet::read([[maybe_unused]] DataStream& ds) {}
void Packet::write(DataStream& ds, [[maybe_unused]] NetCompatibilityRules netRules) const { write(ds); }
void Packet::write([[maybe_unused]] DataStream& ds) const {}
void Packet::readJson([[maybe_unused]] Json const& json) {}
auto Packet::writeJson() const -> Json { return JsonObject{}; }

auto Packet::compressionMode() const -> PacketCompressionMode { return m_compressionMode; }
void Packet::setCompressionMode(PacketCompressionMode compressionMode) { m_compressionMode = compressionMode; }

auto createPacket(PacketType type) -> Ptr<Packet> {
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
  default:
    throw StarPacketException(strf("Unrecognized packet type {}", (unsigned int)type));
  }
}

auto createPacket(PacketType type, std::optional<Json> const& args) -> Ptr<Packet> {
  auto packet = createPacket(type);

  if (args && !args->isNull())
    packet->readJson(*args);

  return packet;
}

ProtocolRequestPacket::ProtocolRequestPacket()
    : requestProtocolVersion(0) {}

ProtocolRequestPacket::ProtocolRequestPacket(VersionNumber requestProtocolVersion)
    : requestProtocolVersion(requestProtocolVersion) {}

void ProtocolRequestPacket::read(DataStream& ds) {
  ds.read(requestProtocolVersion);
}

void ProtocolRequestPacket::write(DataStream& ds) const {
  ds.write(requestProtocolVersion);
}

ProtocolResponsePacket::ProtocolResponsePacket(bool allowed, Json info)
    : allowed(allowed), info(std::move(info)) {}

void ProtocolResponsePacket::read(DataStream& ds) {
  ds.read(allowed);
  if (compressionMode() == PacketCompressionMode::Enabled) {
    // gross hack for backwards compatibility with older OpenSB servers
    // can be removed later
    auto externalBuffer = as<DataStreamExternalBuffer>(&ds);
    if (!externalBuffer || !externalBuffer->atEnd())
      ds.read(info);
  }
}

void ProtocolResponsePacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  ds.write(allowed);
  if (!netRules.isLegacy())
    ds.write(info);
}

ConnectSuccessPacket::ConnectSuccessPacket() = default;

ConnectSuccessPacket::ConnectSuccessPacket(
  ConnectionId clientId, Uuid serverUuid, CelestialBaseInformation celestialInformation)
    : clientId(clientId), serverUuid(std::move(serverUuid)), celestialInformation(std::move(celestialInformation)) {}

void ConnectSuccessPacket::read(DataStream& ds) {
  ds.vuread(clientId);
  ds.read(serverUuid);
  ds.read(celestialInformation);
}

void ConnectSuccessPacket::write(DataStream& ds) const {
  ds.vuwrite(clientId);
  ds.write(serverUuid);
  ds.write(celestialInformation);
}

ConnectFailurePacket::ConnectFailurePacket() = default;

ConnectFailurePacket::ConnectFailurePacket(String reason) : reason(std::move(reason)) {}

void ConnectFailurePacket::read(DataStream& ds) {
  ds.read(reason);
}

void ConnectFailurePacket::write(DataStream& ds) const {
  ds.write(reason);
}

HandshakeChallengePacket::HandshakeChallengePacket() = default;

HandshakeChallengePacket::HandshakeChallengePacket(ByteArray const& passwordSalt) : passwordSalt(std::move(passwordSalt)) {}

void HandshakeChallengePacket::read(DataStream& ds) {
  ds.read(passwordSalt);
}

void HandshakeChallengePacket::write(DataStream& ds) const {
  ds.write(passwordSalt);
}

ChatReceivePacket::ChatReceivePacket() = default;

ChatReceivePacket::ChatReceivePacket(ChatReceivedMessage receivedMessage) : receivedMessage(std::move(receivedMessage)) {}

void ChatReceivePacket::read(DataStream& ds) {
  ds.read(receivedMessage);
}

void ChatReceivePacket::write(DataStream& ds) const {
  ds.write(receivedMessage);
}

void ChatReceivePacket::readJson(Json const& json) {
  receivedMessage = ChatReceivedMessage(json.get("receivedMessage"));
}

auto ChatReceivePacket::writeJson() const -> Json {
  return JsonObject{
    {"receivedMessage", receivedMessage.toJson()}};
}

UniverseTimeUpdatePacket::UniverseTimeUpdatePacket() {
  universeTime = 0;
}

UniverseTimeUpdatePacket::UniverseTimeUpdatePacket(double universeTime) : universeTime(universeTime) {}

void UniverseTimeUpdatePacket::read(DataStream& ds) {
  ds.vfread(universeTime, 0.05);
}

void UniverseTimeUpdatePacket::write(DataStream& ds) const {
  ds.vfwrite(universeTime, 0.05);
}

CelestialResponsePacket::CelestialResponsePacket() = default;

CelestialResponsePacket::CelestialResponsePacket(List<CelestialResponse> responses) : responses(std::move(responses)) {}

void CelestialResponsePacket::read(DataStream& ds) {
  ds.read(responses);
}

void CelestialResponsePacket::write(DataStream& ds) const {
  ds.write(responses);
}

PlayerWarpResultPacket::PlayerWarpResultPacket() : warpActionInvalid(false) {}

PlayerWarpResultPacket::PlayerWarpResultPacket(bool success, WarpAction warpAction, bool warpActionInvalid)
    : success(success), warpAction(std::move(warpAction)), warpActionInvalid(warpActionInvalid) {}

void PlayerWarpResultPacket::read(DataStream& ds) {
  ds.read(success);
  ds.read(warpAction);
  ds.read(warpActionInvalid);
}

void PlayerWarpResultPacket::write(DataStream& ds) const {
  ds.write(success);
  ds.write(warpAction);
  ds.write(warpActionInvalid);
}

PlanetTypeUpdatePacket::PlanetTypeUpdatePacket() = default;

PlanetTypeUpdatePacket::PlanetTypeUpdatePacket(CelestialCoordinate coordinate)
    : coordinate(coordinate) {}

void PlanetTypeUpdatePacket::read(DataStream& ds) {
  ds.read(coordinate);
}

void PlanetTypeUpdatePacket::write(DataStream& ds) const {
  ds.write(coordinate);
}

PausePacket::PausePacket() = default;

PausePacket::PausePacket(bool pause, float timescale) : pause(pause), timescale(timescale) {}

void PausePacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  ds.read(pause);
  if (!netRules.isLegacy())
    ds.read(timescale);
  else
    timescale = 1.0f;
}

void PausePacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  ds.write(pause);
  if (!netRules.isLegacy())
    ds.write(timescale);
}

void PausePacket::readJson(Json const& json) {
  pause = json.getBool("pause");
  timescale = json.getFloat("timescale", 1.0f);
}

auto PausePacket::writeJson() const -> Json {
  return JsonObject{
    {"pause", pause},
    {"timescale", timescale}};
}

ServerInfoPacket::ServerInfoPacket() = default;

ServerInfoPacket::ServerInfoPacket(std::uint16_t players, std::uint16_t maxPlayers) : players(players),
                                                                                      maxPlayers(maxPlayers) {}

void ServerInfoPacket::read(DataStream& ds) {
  ds.read(players);
  ds.read(maxPlayers);
}

void ServerInfoPacket::write(DataStream& ds) const {
  ds.write(players);
  ds.write(maxPlayers);
}

void ServerInfoPacket::readJson(Json const& json) {
  players = json.getUInt("players");
  maxPlayers = json.getUInt("maxPlayers");
}

auto ServerInfoPacket::writeJson() const -> Json {
  return JsonObject{
    {"players", players},
    {"maxPlayers", maxPlayers}};
}

ClientConnectPacket::ClientConnectPacket() = default;

ClientConnectPacket::ClientConnectPacket(ByteArray assetsDigest, bool allowAssetsMismatch, Uuid playerUuid,
                                         String playerName, String shipSpecies, WorldChunks shipChunks, ShipUpgrades shipUpgrades,
                                         bool introComplete, String account, Json info)
    : assetsDigest(std::move(assetsDigest)), allowAssetsMismatch(allowAssetsMismatch), playerUuid(std::move(playerUuid)),
      playerName(std::move(playerName)), shipSpecies(std::move(shipSpecies)), shipChunks(std::move(shipChunks)),
      shipUpgrades(std::move(shipUpgrades)), introComplete(std::move(introComplete)), account(std::move(account)), info(std::move(info)) {}

void ClientConnectPacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  ds.read(assetsDigest);
  ds.read(allowAssetsMismatch);
  ds.read(playerUuid);
  ds.read(playerName);
  ds.read(shipSpecies);
  ds.read(shipChunks);
  ds.read(shipUpgrades);
  ds.read(introComplete);
  ds.read(account);
  if (!netRules.isLegacy())
    ds.read(info);
}

void ClientConnectPacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  ds.write(assetsDigest);
  ds.write(allowAssetsMismatch);
  ds.write(playerUuid);
  ds.write(playerName);
  ds.write(shipSpecies);
  ds.write(shipChunks);
  ds.write(shipUpgrades);
  ds.write(introComplete);
  ds.write(account);
  if (!netRules.isLegacy())
    ds.write(info);
}

ClientDisconnectRequestPacket::ClientDisconnectRequestPacket() = default;

void ClientDisconnectRequestPacket::read(DataStream& ds) {
  // Packets cannot be empty due to the way packet serialization is handled.
  ds.read<std::uint8_t>();
}

void ClientDisconnectRequestPacket::write(DataStream& ds) const {
  // Packets cannot be empty due to the way packet serialization is handled.
  ds.write<std::uint8_t>(0);
}

HandshakeResponsePacket::HandshakeResponsePacket() = default;

HandshakeResponsePacket::HandshakeResponsePacket(ByteArray const& passHash) : passHash(std::move(passHash)) {}

void HandshakeResponsePacket::read(DataStream& ds) {
  ds.read(passHash);
}

void HandshakeResponsePacket::write(DataStream& ds) const {
  ds.write(passHash);
}

PlayerWarpPacket::PlayerWarpPacket() = default;

PlayerWarpPacket::PlayerWarpPacket(WarpAction action, bool deploy) : action(std::move(action)), deploy(std::move(deploy)) {}

void PlayerWarpPacket::read(DataStream& ds) {
  ds.read(action);
  ds.read(deploy);
}

void PlayerWarpPacket::write(DataStream& ds) const {
  ds.write(action);
  ds.write(deploy);
}

FlyShipPacket::FlyShipPacket() = default;

FlyShipPacket::FlyShipPacket(Vec3I system, SystemLocation location, Json settings) : system(std::move(system)), location(std::move(location)), settings(std::move(settings)) {}

void FlyShipPacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  ds.read(system);
  ds.read(location);
  if (netRules.version() >= 3)
    ds.read(settings);
}

void FlyShipPacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  ds.write(system);
  ds.write(location);
  if (netRules.version() >= 3)
    ds.write(settings);
}

ChatSendPacket::ChatSendPacket() : sendMode(ChatSendMode::Broadcast) {}

ChatSendPacket::ChatSendPacket(String text, ChatSendMode sendMode) : text(std::move(text)), sendMode(sendMode) {}

ChatSendPacket::ChatSendPacket(String text, ChatSendMode sendMode, JsonObject data) : text(std::move(text)), sendMode(sendMode), data(std::move(data)) {}

void ChatSendPacket::read(DataStream& ds) {
  ds.read(text);
  ds.read(sendMode);
  if (ds.streamCompatibilityVersion() >= 5)
    ds.read(data);
}

void ChatSendPacket::write(DataStream& ds) const {
  ds.write(text);
  ds.write(sendMode);
  if (ds.streamCompatibilityVersion() >= 5)
    ds.write(data);
}

CelestialRequestPacket::CelestialRequestPacket() = default;

CelestialRequestPacket::CelestialRequestPacket(List<CelestialRequest> requests) : requests(std::move(requests)) {}

void CelestialRequestPacket::read(DataStream& ds) {
  ds.read(requests);
}

void CelestialRequestPacket::write(DataStream& ds) const {
  ds.write(requests);
}

ClientContextUpdatePacket::ClientContextUpdatePacket() = default;

ClientContextUpdatePacket::ClientContextUpdatePacket(ByteArray updateData) : updateData(std::move(updateData)) {}

void ClientContextUpdatePacket::read(DataStream& ds) {
  ds.read(updateData);
}

void ClientContextUpdatePacket::write(DataStream& ds) const {
  ds.write(updateData);
}

WorldStartPacket::WorldStartPacket() : clientId(), localInterpolationMode() {}

void WorldStartPacket::read(DataStream& ds) {
  ds.read(templateData);
  ds.read(skyData);
  ds.read(weatherData);
  ds.read(playerStart);
  ds.read(playerRespawn);
  ds.read(respawnInWorld);
  ds.read(worldProperties);
  ds.read(dungeonIdGravity);
  ds.read(dungeonIdBreathable);
  ds.read(protectedDungeonIds);
  ds.read(clientId);
  ds.read(localInterpolationMode);
}

void WorldStartPacket::write(DataStream& ds) const {
  ds.write(templateData);
  ds.write(skyData);
  ds.write(weatherData);
  ds.write(playerStart);
  ds.write(playerRespawn);
  ds.write(respawnInWorld);
  ds.write(worldProperties);
  ds.write(dungeonIdGravity);
  ds.write(dungeonIdBreathable);
  ds.write(protectedDungeonIds);
  ds.write(clientId);
  ds.write(localInterpolationMode);
}

WorldStopPacket::WorldStopPacket() = default;

WorldStopPacket::WorldStopPacket(String const& reason) : reason(std::move(reason)) {}

void WorldStopPacket::read(DataStream& ds) {
  ds.read(reason);
}

void WorldStopPacket::write(DataStream& ds) const {
  ds.write(reason);
}

WorldLayoutUpdatePacket::WorldLayoutUpdatePacket() = default;

WorldLayoutUpdatePacket::WorldLayoutUpdatePacket(Json const& layoutData) : layoutData(std::move(layoutData)) {}

void WorldLayoutUpdatePacket::read(DataStream& ds) {
  ds.read(layoutData);
}

void WorldLayoutUpdatePacket::write(DataStream& ds) const {
  ds.write(layoutData);
}

WorldParametersUpdatePacket::WorldParametersUpdatePacket() = default;

WorldParametersUpdatePacket::WorldParametersUpdatePacket(ByteArray const& parametersData) : parametersData(std::move(parametersData)) {}

void WorldParametersUpdatePacket::read(DataStream& ds) {
  ds.read(parametersData);
}

void WorldParametersUpdatePacket::write(DataStream& ds) const {
  ds.write(parametersData);
}

CentralStructureUpdatePacket::CentralStructureUpdatePacket() = default;

CentralStructureUpdatePacket::CentralStructureUpdatePacket(Json structureData) : structureData(std::move(structureData)) {}

void CentralStructureUpdatePacket::read(DataStream& ds) {
  ds.read(structureData);
}

void CentralStructureUpdatePacket::write(DataStream& ds) const {
  ds.write(structureData);
}

TileArrayUpdatePacket::TileArrayUpdatePacket() = default;

void TileArrayUpdatePacket::read(DataStream& ds) {
  ds.viread(min[0]);
  ds.viread(min[1]);

  size_t width, height;
  ds.vuread(width);
  ds.vuread(height);
  array.resize(width, height);
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x)
      ds.read(array(x, y));
  }
}

void TileArrayUpdatePacket::write(DataStream& ds) const {
  ds.viwrite(min[0]);
  ds.viwrite(min[1]);
  ds.vuwrite(array.size(0));
  ds.vuwrite(array.size(1));
  for (size_t y = 0; y < array.size(1); ++y) {
    for (size_t x = 0; x < array.size(0); ++x)
      ds.write(array(x, y));
  }
}

void TileUpdatePacket::read(DataStream& ds) {
  ds.viread(position[0]);
  ds.viread(position[1]);
  ds.read(tile);
}

void TileUpdatePacket::write(DataStream& ds) const {
  ds.viwrite(position[0]);
  ds.viwrite(position[1]);
  ds.write(tile);
}

TileLiquidUpdatePacket::TileLiquidUpdatePacket() = default;

TileLiquidUpdatePacket::TileLiquidUpdatePacket(Vec2I const& position, LiquidNetUpdate liquidUpdate)
    : position(position), liquidUpdate(liquidUpdate) {}

void TileLiquidUpdatePacket::read(DataStream& ds) {
  ds.viread(position[0]);
  ds.viread(position[1]);
  ds.read(liquidUpdate.liquid);
  ds.read(liquidUpdate.level);
}

void TileLiquidUpdatePacket::write(DataStream& ds) const {
  ds.viwrite(position[0]);
  ds.viwrite(position[1]);
  ds.write(liquidUpdate.liquid);
  ds.write(liquidUpdate.level);
}

TileDamageUpdatePacket::TileDamageUpdatePacket() : layer(TileLayer::Foreground) {}

TileDamageUpdatePacket::TileDamageUpdatePacket(
  Vec2I const& position, TileLayer layer, TileDamageStatus const& tileDamage)
    : position(position), layer(layer), tileDamage(tileDamage) {}

void TileDamageUpdatePacket::read(DataStream& ds) {
  ds.read(position);
  ds.read(layer);
  ds.read(tileDamage);
}

void TileDamageUpdatePacket::write(DataStream& ds) const {
  ds.write(position);
  ds.write(layer);
  ds.write(tileDamage);
}

TileModificationFailurePacket::TileModificationFailurePacket() = default;

TileModificationFailurePacket::TileModificationFailurePacket(TileModificationList modifications)
    : modifications(std::move(modifications)) {}

void TileModificationFailurePacket::read(DataStream& ds) {
  ds.readContainer(modifications);
}

void TileModificationFailurePacket::write(DataStream& ds) const {
  ds.writeContainer(modifications);
}

GiveItemPacket::GiveItemPacket() = default;

GiveItemPacket::GiveItemPacket(ItemDescriptor const& item) : item(std::move(item)) {}

void GiveItemPacket::read(DataStream& ds) {
  ds.read(item);
}

void GiveItemPacket::write(DataStream& ds) const {
  ds.write(item);
}

void GiveItemPacket::readJson(Json const& json) {
  item = ItemDescriptor(json.get("item"));
}

auto GiveItemPacket::writeJson() const -> Json {
  return JsonObject{
    {"item", item.toJson()}};
}

EnvironmentUpdatePacket::EnvironmentUpdatePacket() = default;

EnvironmentUpdatePacket::EnvironmentUpdatePacket(ByteArray skyDelta, ByteArray weatherDelta)
    : skyDelta(std::move(skyDelta)), weatherDelta(std::move(weatherDelta)) {}

void EnvironmentUpdatePacket::read(DataStream& ds) {
  ds.read(skyDelta);
  ds.read(weatherDelta);
}

void EnvironmentUpdatePacket::write(DataStream& ds) const {
  ds.write(skyDelta);
  ds.write(weatherDelta);
}

ModifyTileListPacket::ModifyTileListPacket() : allowEntityOverlap() {}

ModifyTileListPacket::ModifyTileListPacket(TileModificationList modifications, bool allowEntityOverlap)
    : modifications(std::move(modifications)), allowEntityOverlap(allowEntityOverlap) {}

void ModifyTileListPacket::read(DataStream& ds) {
  ds.readContainer(modifications);
  ds.read(allowEntityOverlap);
}

void ModifyTileListPacket::write(DataStream& ds) const {
  ds.writeContainer(modifications);
  ds.write(allowEntityOverlap);
}

ReplaceTileListPacket::ReplaceTileListPacket() : applyDamage() {}

ReplaceTileListPacket::ReplaceTileListPacket(TileModificationList modifications, TileDamage tileDamage, bool applyDamage)
    : modifications(std::move(modifications)), tileDamage(std::move(tileDamage)), applyDamage(applyDamage) {}

void ReplaceTileListPacket::read(DataStream& ds) {
  ds.readContainer(modifications);
  ds.read(tileDamage);
  if (ds.streamCompatibilityVersion() >= 7)
    ds.read(applyDamage);
  else
    applyDamage = false;
}

void ReplaceTileListPacket::write(DataStream& ds) const {
  ds.writeContainer(modifications);
  ds.write(tileDamage);
  if (ds.streamCompatibilityVersion() >= 7)
    ds.write(applyDamage);
}

DamageTileGroupPacket::DamageTileGroupPacket() : layer(TileLayer::Foreground) {}

DamageTileGroupPacket::DamageTileGroupPacket(
  List<Vec2I> tilePositions, TileLayer layer, Vec2F sourcePosition, TileDamage tileDamage, std::optional<EntityId> sourceEntity)
    : tilePositions(std::move(tilePositions)), layer(layer), sourcePosition(sourcePosition), tileDamage(std::move(tileDamage)), sourceEntity(std::move(sourceEntity)) {}

void DamageTileGroupPacket::read(DataStream& ds) {
  ds.readContainer(tilePositions);
  ds.read(layer);
  ds.read(sourcePosition);
  ds.read(tileDamage);
  ds.read(sourceEntity);
}

void DamageTileGroupPacket::write(DataStream& ds) const {
  ds.writeContainer(tilePositions);
  ds.write(layer);
  ds.write(sourcePosition);
  ds.write(tileDamage);
  ds.write(sourceEntity);
}

CollectLiquidPacket::CollectLiquidPacket() = default;

CollectLiquidPacket::CollectLiquidPacket(List<Vec2I> tilePositions, LiquidId liquidId)
    : tilePositions(std::move(tilePositions)), liquidId(liquidId) {}

void CollectLiquidPacket::read(DataStream& ds) {
  ds.readContainer(tilePositions);
  ds.read(liquidId);
}

void CollectLiquidPacket::write(DataStream& ds) const {
  ds.writeContainer(tilePositions);
  ds.write(liquidId);
}

RequestDropPacket::RequestDropPacket() {
  dropEntityId = NullEntityId;
}

RequestDropPacket::RequestDropPacket(EntityId dropEntityId) : dropEntityId(dropEntityId) {}

void RequestDropPacket::read(DataStream& ds) {
  ds.viread(dropEntityId);
}

void RequestDropPacket::write(DataStream& ds) const {
  ds.viwrite(dropEntityId);
}

SpawnEntityPacket::SpawnEntityPacket() = default;

SpawnEntityPacket::SpawnEntityPacket(EntityType entityType, ByteArray storeData, ByteArray firstNetState)
    : entityType(entityType), storeData(std::move(storeData)), firstNetState(std::move(firstNetState)) {}

void SpawnEntityPacket::read(DataStream& ds) {
  ds.read(entityType);
  ds.read(storeData);
  ds.read(firstNetState);
}

void SpawnEntityPacket::write(DataStream& ds) const {
  ds.write(entityType);
  ds.write(storeData);
  ds.write(firstNetState);
}

EntityInteractPacket::EntityInteractPacket() = default;

EntityInteractPacket::EntityInteractPacket(InteractRequest interactRequest, Uuid requestId)
    : interactRequest(interactRequest), requestId(requestId) {}

void EntityInteractPacket::read(DataStream& ds) {
  ds.read(interactRequest);
  ds.read(requestId);
}

void EntityInteractPacket::write(DataStream& ds) const {
  ds.write(interactRequest);
  ds.write(requestId);
}

EntityInteractResultPacket::EntityInteractResultPacket() = default;

EntityInteractResultPacket::EntityInteractResultPacket(InteractAction action, Uuid requestId, EntityId sourceEntityId)
    : action(std::move(action)), requestId(requestId), sourceEntityId(sourceEntityId) {}

void EntityInteractResultPacket::read(DataStream& ds) {
  ds.read(action);
  ds.read(requestId);
  ds.read(sourceEntityId);
}

void EntityInteractResultPacket::write(DataStream& ds) const {
  ds.write(action);
  ds.write(requestId);
  ds.write(sourceEntityId);
}

EntityCreatePacket::EntityCreatePacket() {
  entityId = NullEntityId;
}

ServerDisconnectPacket::ServerDisconnectPacket() = default;

ServerDisconnectPacket::ServerDisconnectPacket(String reason) : reason(std::move(reason)) {}

void ServerDisconnectPacket::read(DataStream& ds) {
  ds.read(reason);
}

void ServerDisconnectPacket::write(DataStream& ds) const {
  ds.write(reason);
}

ConnectWirePacket::ConnectWirePacket() = default;

ConnectWirePacket::ConnectWirePacket(WireConnection outputConnection, WireConnection inputConnection)
    : outputConnection(outputConnection), inputConnection(inputConnection) {}

void ConnectWirePacket::read(DataStream& ds) {
  ds.read(outputConnection);
  ds.read(inputConnection);
}

void ConnectWirePacket::write(DataStream& ds) const {
  ds.write(outputConnection);
  ds.write(inputConnection);
}

DisconnectAllWiresPacket::DisconnectAllWiresPacket() = default;

DisconnectAllWiresPacket::DisconnectAllWiresPacket(Vec2I entityPosition, WireNode wireNode)
    : entityPosition(entityPosition), wireNode(wireNode) {}

void DisconnectAllWiresPacket::read(DataStream& ds) {
  ds.viread(entityPosition[0]);
  ds.viread(entityPosition[1]);
  ds.read(wireNode);
}

void DisconnectAllWiresPacket::write(DataStream& ds) const {
  ds.viwrite(entityPosition[0]);
  ds.viwrite(entityPosition[1]);
  ds.write(wireNode);
}

WorldClientStateUpdatePacket::WorldClientStateUpdatePacket() = default;

WorldClientStateUpdatePacket::WorldClientStateUpdatePacket(ByteArray const& worldClientStateDelta)
    : worldClientStateDelta(std::move(worldClientStateDelta)) {}

void WorldClientStateUpdatePacket::read(DataStream& ds) {
  ds.read(worldClientStateDelta);
}

void WorldClientStateUpdatePacket::write(DataStream& ds) const {
  ds.write(worldClientStateDelta);
}

FindUniqueEntityPacket::FindUniqueEntityPacket() = default;

FindUniqueEntityPacket::FindUniqueEntityPacket(String uniqueEntityId)
    : uniqueEntityId(std::move(uniqueEntityId)) {}

void FindUniqueEntityPacket::read(DataStream& ds) {
  ds.read(uniqueEntityId);
}

void FindUniqueEntityPacket::write(DataStream& ds) const {
  ds.write(uniqueEntityId);
}

WorldStartAcknowledgePacket::WorldStartAcknowledgePacket() = default;

void WorldStartAcknowledgePacket::read(DataStream& ds) {
  // Packets can't be empty, read the trash data
  ds.read<bool>();
}

void WorldStartAcknowledgePacket::write(DataStream& ds) const {
  // Packets can't be empty, write some trash data
  ds.write<bool>(false);
}

PingPacket::PingPacket() = default;
PingPacket::PingPacket(std::int64_t time) : time(time) {}

void PingPacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  if (netRules.isLegacy()) {
    // Packets can't be empty, read the trash data
    ds.read<bool>();
    time = 0;
  } else {
    ds.readVlqI(time);
  }
}

void PingPacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  if (netRules.isLegacy()) {
    // Packets can't be empty, write some trash data
    ds.write<bool>(false);
  } else {
    ds.writeVlqI(time);
  }
}

EntityCreatePacket::EntityCreatePacket(EntityType entityType, ByteArray storeData, ByteArray firstNetState, EntityId entityId)
    : entityType(entityType), storeData(std::move(storeData)), firstNetState(std::move(firstNetState)), entityId(entityId) {}

void EntityCreatePacket::read(DataStream& ds) {
  ds.read(entityType);
  ds.read(storeData);
  ds.read(firstNetState);
  ds.viread(entityId);
}

void EntityCreatePacket::write(DataStream& ds) const {
  ds.write(entityType);
  ds.write(storeData);
  ds.write(firstNetState);
  ds.viwrite(entityId);
}

EntityUpdateSetPacket::EntityUpdateSetPacket(ConnectionId forConnection) : forConnection(forConnection) {}

void EntityUpdateSetPacket::read(DataStream& ds) {
  ds.vuread(forConnection);
  ds.readMapContainer(deltas,
                      [](DataStream& ds, EntityId& entityId, ByteArray& delta) -> void {
                        ds.viread(entityId);
                        ds.read(delta);
                      });
}

void EntityUpdateSetPacket::write(DataStream& ds) const {
  ds.vuwrite(forConnection);
  ds.writeMapContainer(deltas, [](DataStream& ds, EntityId const& entityId, ByteArray const& delta) -> void {
    ds.viwrite(entityId);
    ds.write(delta);
  });
}

EntityDestroyPacket::EntityDestroyPacket() {
  entityId = NullEntityId;
  death = false;
}

EntityDestroyPacket::EntityDestroyPacket(EntityId entityId, ByteArray finalNetState, bool death)
    : entityId(entityId), finalNetState(std::move(finalNetState)), death(death) {}

void EntityDestroyPacket::read(DataStream& ds) {
  ds.viread(entityId);
  ds.read(finalNetState);
  ds.read(death);
}

void EntityDestroyPacket::write(DataStream& ds) const {
  ds.viwrite(entityId);
  ds.write(finalNetState);
  ds.write(death);
}

HitRequestPacket::HitRequestPacket() = default;

HitRequestPacket::HitRequestPacket(RemoteHitRequest remoteHitRequest) : remoteHitRequest(std::move(remoteHitRequest)) {}

void HitRequestPacket::read(DataStream& ds) {
  ds.read(remoteHitRequest);
}

void HitRequestPacket::write(DataStream& ds) const {
  ds.write(remoteHitRequest);
}

DamageRequestPacket::DamageRequestPacket() = default;

DamageRequestPacket::DamageRequestPacket(RemoteDamageRequest remoteDamageRequest)
    : remoteDamageRequest(std::move(remoteDamageRequest)) {}

void DamageRequestPacket::read(DataStream& ds) {
  ds.read(remoteDamageRequest);
}

void DamageRequestPacket::write(DataStream& ds) const {
  ds.write(remoteDamageRequest);
}

DamageNotificationPacket::DamageNotificationPacket() = default;

DamageNotificationPacket::DamageNotificationPacket(RemoteDamageNotification remoteDamageNotification)
    : remoteDamageNotification(std::move(remoteDamageNotification)) {}

void DamageNotificationPacket::read(DataStream& ds) {
  ds.read(remoteDamageNotification);
}

void DamageNotificationPacket::write(DataStream& ds) const {
  ds.write(remoteDamageNotification);
}

EntityMessagePacket::EntityMessagePacket() = default;

EntityMessagePacket::EntityMessagePacket(Variant<EntityId, String> entityId, String message, JsonArray args, Uuid uuid, ConnectionId fromConnection)
    : entityId(std::move(entityId)), message(std::move(message)), args(std::move(args)), uuid(uuid), fromConnection(fromConnection) {}

void EntityMessagePacket::read(DataStream& ds) {
  ds.read(entityId);
  ds.read(message);
  ds.read(args);
  ds.read(uuid);
  ds.read(fromConnection);
}

void EntityMessagePacket::write(DataStream& ds) const {
  ds.write(entityId);
  ds.write(message);
  ds.write(args);
  ds.write(uuid);
  ds.write(fromConnection);
}

void EntityMessagePacket::readJson(Json const& json) {
  auto jEntityId = json.get("entityId");
  if (jEntityId.canConvert(Json::Type::Int))
    entityId = (EntityId)jEntityId.toInt();
  else
    entityId = jEntityId.toString();
  message = json.getString("message");
  args = json.getArray("args");
  uuid = Uuid(json.getString("uuid"));
  fromConnection = json.getUInt("fromConnection");
}

auto EntityMessagePacket::writeJson() const -> Json {
  return JsonObject{
    {"entityId", entityId.is<EntityId>() ? Json(entityId.get<EntityId>()) : Json(entityId.get<String>())},
    {"message", message},
    {"args", args},
    {"uuid", uuid.hex()},
    {"fromConnection", fromConnection}};
}

EntityMessageResponsePacket::EntityMessageResponsePacket() = default;

EntityMessageResponsePacket::EntityMessageResponsePacket(Either<String, Json> response, Uuid uuid)
    : response(std::move(response)), uuid(uuid) {}

void EntityMessageResponsePacket::read(DataStream& ds) {
  ds.read(response);
  ds.read(uuid);
}

void EntityMessageResponsePacket::write(DataStream& ds) const {
  ds.write(response);
  ds.write(uuid);
}

UpdateWorldPropertiesPacket::UpdateWorldPropertiesPacket() = default;

UpdateWorldPropertiesPacket::UpdateWorldPropertiesPacket(JsonObject const& updatedProperties)
    : updatedProperties(std::move(updatedProperties)) {}

void UpdateWorldPropertiesPacket::read(DataStream& ds) {
  ds.readMapContainer(updatedProperties);
}

void UpdateWorldPropertiesPacket::write(DataStream& ds) const {
  ds.writeMapContainer(updatedProperties);
}

void UpdateWorldPropertiesPacket::readJson(Json const& json) {
  updatedProperties = json.getObject("updatedProperties");
}

auto UpdateWorldPropertiesPacket::writeJson() const -> Json {
  return JsonObject{
    {"updatedProperties", updatedProperties},
  };
}

UpdateTileProtectionPacket::UpdateTileProtectionPacket() = default;

UpdateTileProtectionPacket::UpdateTileProtectionPacket(DungeonId dungeonId, bool isProtected)
    : dungeonId(dungeonId), isProtected(isProtected) {}

void UpdateTileProtectionPacket::read(DataStream& ds) {
  ds.read(dungeonId);
  ds.read(isProtected);
}

void UpdateTileProtectionPacket::write(DataStream& ds) const {
  ds.write(dungeonId);
  ds.write(isProtected);
}

void UpdateTileProtectionPacket::readJson(Json const& json) {
  dungeonId = json.getUInt("dungeonId");
  isProtected = json.getBool("isProtected");
}

auto UpdateTileProtectionPacket::writeJson() const -> Json {
  return JsonObject{
    {"dungeonId", dungeonId},
    {"isProtected", isProtected}};
}

SetDungeonGravityPacket::SetDungeonGravityPacket() = default;

SetDungeonGravityPacket::SetDungeonGravityPacket(DungeonId dungeonId, std::optional<float> gravity)
    : dungeonId(std::move(dungeonId)), gravity(std::move(gravity)) {}

void SetDungeonGravityPacket::read(DataStream& ds) {
  ds.read(dungeonId);
  ds.read(gravity);
}

void SetDungeonGravityPacket::write(DataStream& ds) const {
  ds.write(dungeonId);
  ds.write(gravity);
}

void SetDungeonGravityPacket::readJson(Json const& json) {
  dungeonId = json.getUInt("dungeonId");
  gravity = json.optFloat("gravity");
}

auto SetDungeonGravityPacket::writeJson() const -> Json {
  return JsonObject{
    {"dungeonId", dungeonId},
    {"gravity", jsonFromMaybe<float>(gravity)}};
}

SetDungeonBreathablePacket::SetDungeonBreathablePacket() = default;

SetDungeonBreathablePacket::SetDungeonBreathablePacket(DungeonId dungeonId, std::optional<bool> breathable)
    : dungeonId(std::move(dungeonId)), breathable(std::move(breathable)) {}

void SetDungeonBreathablePacket::read(DataStream& ds) {
  ds.read(dungeonId);
  ds.read(breathable);
}

void SetDungeonBreathablePacket::write(DataStream& ds) const {
  ds.write(dungeonId);
  ds.write(breathable);
}

void SetDungeonBreathablePacket::readJson(Json const& json) {
  dungeonId = json.getUInt("dungeonId");
  breathable = json.optBool("breathable");
}

auto SetDungeonBreathablePacket::writeJson() const -> Json {
  return JsonObject{
    {"dungeonId", dungeonId},
    {"breathable", jsonFromMaybe<bool>(breathable)}};
}

SetPlayerStartPacket::SetPlayerStartPacket() = default;

SetPlayerStartPacket::SetPlayerStartPacket(Vec2F playerStart, bool respawnInWorld) : playerStart(playerStart), respawnInWorld(respawnInWorld) {}

void SetPlayerStartPacket::read(DataStream& ds) {
  ds.read(playerStart);
  ds.read(respawnInWorld);
}

void SetPlayerStartPacket::write(DataStream& ds) const {
  ds.write(playerStart);
  ds.write(respawnInWorld);
}

void SetPlayerStartPacket::readJson(Json const& json) {
  playerStart = jsonToVec2F(json.get("playerStart"));
  respawnInWorld = json.getBool("respawnInWorld");
}

auto SetPlayerStartPacket::writeJson() const -> Json {
  return JsonObject{
    {"playerStart", jsonFromVec2F(playerStart)},
    {"respawnInWorld", respawnInWorld}};
}

FindUniqueEntityResponsePacket::FindUniqueEntityResponsePacket() = default;

FindUniqueEntityResponsePacket::FindUniqueEntityResponsePacket(String uniqueEntityId, std::optional<Vec2F> entityPosition)
    : uniqueEntityId(std::move(uniqueEntityId)), entityPosition(std::move(entityPosition)) {}

void FindUniqueEntityResponsePacket::read(DataStream& ds) {
  ds.read(uniqueEntityId);
  ds.read(entityPosition);
}

void FindUniqueEntityResponsePacket::write(DataStream& ds) const {
  ds.write(uniqueEntityId);
  ds.write(entityPosition);
}

PongPacket::PongPacket() = default;
PongPacket::PongPacket(std::int64_t time) : time(time) {}

void PongPacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  if (netRules.isLegacy()) {
    // Packets can't be empty, read the trash data
    ds.read<bool>();
    time = 0;
  } else {
    ds.readVlqI(time);
  }
}

void PongPacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  if (netRules.isLegacy()) {
    // Packets can't be empty, write some trash data
    ds.write<bool>(false);
  } else {
    ds.writeVlqI(time);
  }
}

StepUpdatePacket::StepUpdatePacket() : remoteTime(0.0) {}

StepUpdatePacket::StepUpdatePacket(double remoteTime) : remoteTime(remoteTime) {}

void StepUpdatePacket::read(DataStream& ds, NetCompatibilityRules netRules) {
  if (netRules.isLegacy()) {
    auto steps = ds.readVlqU();
    remoteTime = double(steps) / 60.0;
  } else {
    ds.read(remoteTime);
  }
}

void StepUpdatePacket::write(DataStream& ds, NetCompatibilityRules netRules) const {
  if (netRules.isLegacy()) {
    ds.writeVlqU((std::uint64_t)std::round(remoteTime * 60.0));
  } else {
    ds.write(remoteTime);
  }
}

SystemWorldStartPacket::SystemWorldStartPacket() = default;

SystemWorldStartPacket::SystemWorldStartPacket(Vec3I location, List<ByteArray> objectStores, List<ByteArray> shipStores, std::pair<Uuid, SystemLocation> clientShip)
    : location(std::move(location)), objectStores(std::move(objectStores)), shipStores(std::move(shipStores)), clientShip(std::move(clientShip)) {}

void SystemWorldStartPacket::read(DataStream& ds) {
  ds.read(location);
  ds.read(objectStores);
  ds.read(shipStores);
  ds.read(clientShip);
}

void SystemWorldStartPacket::write(DataStream& ds) const {
  ds.write(location);
  ds.write(objectStores);
  ds.write(shipStores);
  ds.write(clientShip);
}

SystemWorldUpdatePacket::SystemWorldUpdatePacket() = default;

SystemWorldUpdatePacket::SystemWorldUpdatePacket(HashMap<Uuid, ByteArray> objectUpdates, HashMap<Uuid, ByteArray> shipUpdates)
    : objectUpdates(std::move(objectUpdates)), shipUpdates(std::move(shipUpdates)) {}

void SystemWorldUpdatePacket::read(DataStream& ds) {
  ds.read(objectUpdates);
  ds.read(shipUpdates);
}

void SystemWorldUpdatePacket::write(DataStream& ds) const {
  ds.write(objectUpdates);
  ds.write(shipUpdates);
}

SystemObjectCreatePacket::SystemObjectCreatePacket() = default;

SystemObjectCreatePacket::SystemObjectCreatePacket(ByteArray objectStore) : objectStore(std::move(objectStore)) {}

void SystemObjectCreatePacket::read(DataStream& ds) {
  ds.read(objectStore);
}

void SystemObjectCreatePacket::write(DataStream& ds) const {
  ds.write(objectStore);
}

SystemObjectDestroyPacket::SystemObjectDestroyPacket() = default;

SystemObjectDestroyPacket::SystemObjectDestroyPacket(Uuid objectUuid) : objectUuid(std::move(objectUuid)) {}

void SystemObjectDestroyPacket::read(DataStream& ds) {
  ds.read(objectUuid);
}

void SystemObjectDestroyPacket::write(DataStream& ds) const {
  ds.write(objectUuid);
}

SystemShipCreatePacket::SystemShipCreatePacket() = default;

SystemShipCreatePacket::SystemShipCreatePacket(ByteArray shipStore) : shipStore(std::move(shipStore)) {}

void SystemShipCreatePacket::read(DataStream& ds) {
  ds.read(shipStore);
}

void SystemShipCreatePacket::write(DataStream& ds) const {
  ds.write(shipStore);
}

SystemShipDestroyPacket::SystemShipDestroyPacket() = default;

SystemShipDestroyPacket::SystemShipDestroyPacket(Uuid shipUuid) : shipUuid(std::move(shipUuid)) {}

void SystemShipDestroyPacket::read(DataStream& ds) {
  ds.read(shipUuid);
}

void SystemShipDestroyPacket::write(DataStream& ds) const {
  ds.write(shipUuid);
}

SystemObjectSpawnPacket::SystemObjectSpawnPacket() = default;

SystemObjectSpawnPacket::SystemObjectSpawnPacket(String typeName, Uuid uuid, std::optional<Vec2F> position, JsonObject parameters)
    : typeName(std::move(typeName)), uuid(std::move(uuid)), position(std::move(position)), parameters(std::move(parameters)) {}

void SystemObjectSpawnPacket::read(DataStream& ds) {
  ds.read(typeName);
  ds.read(uuid);
  ds.read(position);
  ds.read(parameters);
}

void SystemObjectSpawnPacket::write(DataStream& ds) const {
  ds.write(typeName);
  ds.write(uuid);
  ds.write(position);
  ds.write(parameters);
}

UpdateWorldTemplatePacket::UpdateWorldTemplatePacket() = default;

UpdateWorldTemplatePacket::UpdateWorldTemplatePacket(Json templateData) : templateData(std::move(templateData)) {}

void UpdateWorldTemplatePacket::read(DataStream& ds) {
  ds.read(templateData);
}

void UpdateWorldTemplatePacket::write(DataStream& ds) const {
  ds.write(templateData);
}

}// namespace Star
