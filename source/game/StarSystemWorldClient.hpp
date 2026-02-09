#pragma once

#include "StarCelestialCoordinate.hpp"
#include "StarConfig.hpp"
#include "StarNetPackets.hpp"
#include "StarSystemWorld.hpp"

import std;

namespace Star {

class PlayerUniverseMap;

class SystemWorldClient : public SystemWorld {
public:
  SystemWorldClient(ConstPtr<Clock> universeClock, Ptr<CelestialDatabase> celestialDatabase, Ptr<PlayerUniverseMap> clientContext);

  [[nodiscard]] auto currentSystem() const -> CelestialCoordinate;

  [[nodiscard]] auto shipPosition() const -> std::optional<Vec2F>;
  [[nodiscard]] auto shipLocation() const -> SystemLocation;
  [[nodiscard]] auto shipDestination() const -> SystemLocation;
  [[nodiscard]] auto flying() const -> bool;

  void update(float dt);

  [[nodiscard]] auto objects() const -> List<Ptr<SystemObject>> override;
  [[nodiscard]] auto objectKeys() const -> List<Uuid> override;
  [[nodiscard]] auto getObject(Uuid const& uuid) const -> Ptr<SystemObject> override;

  [[nodiscard]] auto ships() const -> List<Ptr<SystemClientShip>>;
  [[nodiscard]] auto getShip(Uuid const& uuid) const -> Ptr<SystemClientShip>;

  auto spawnObject(String typeName, std::optional<Vec2F> position = {}, std::optional<Uuid> const& uuid = {}, JsonObject parameters = {}) -> Uuid;

  // returns whether the packet was handled
  auto handleIncomingPacket(Ptr<Packet> packet) -> bool;
  auto pullOutgoingPackets() -> List<Ptr<Packet>>;

private:
  auto netLoadObject(ByteArray netStore) -> Ptr<SystemObject>;
  auto netLoadShip(ByteArray netStore) -> Ptr<SystemClientShip>;

  // m_ship can be a null pointer, indicating that the system is not initialized
  Ptr<SystemClientShip> m_ship;
  HashMap<Uuid, Ptr<SystemObject>> m_objects;
  HashMap<Uuid, Ptr<SystemClientShip>> m_clientShips;

  Ptr<PlayerUniverseMap> m_universeMap;

  List<Ptr<Packet>> m_outgoingPackets;
};

}// namespace Star
