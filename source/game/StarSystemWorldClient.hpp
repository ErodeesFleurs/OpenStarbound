#pragma once

#include "StarCelestialParameters.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarPlayerUniverseMap.hpp"
#include "StarSystemWorld.hpp"
#include "StarNetPackets.hpp"

namespace Star {

class SystemWorldClient : public SystemWorld {
public:
  SystemWorldClient(AssetsConstPtr assets, ClockConstPtr universeClock, CelestialDatabasePtr celestialDatabase, PatternedNameGeneratorConstPtr nameGenerator, PlayerUniverseMapPtr clientContext);

  [[nodiscard]] CelestialCoordinate currentSystem() const;

  [[nodiscard]] Maybe<Vec2F> shipPosition() const;
  [[nodiscard]] SystemLocation shipLocation() const;
  [[nodiscard]] SystemLocation shipDestination() const;
  [[nodiscard]] bool flying() const;

  void update(float dt);

  [[nodiscard]] List<SystemObjectPtr> objects() const override;
  [[nodiscard]] List<Uuid> objectKeys() const override;
  [[nodiscard]] SystemObjectPtr getObject(Uuid const& uuid) const override;

  [[nodiscard]] List<SystemClientShipPtr> ships() const;
  [[nodiscard]] SystemClientShipPtr getShip(Uuid const& uuid) const;

  [[nodiscard]] Uuid spawnObject(String typeName, Maybe<Vec2F> position = {}, Maybe<Uuid> const& uuid = {}, JsonObject parameters = {});

  // returns whether the packet was handled
  [[nodiscard]] bool handleIncomingPacket(PacketPtr packet);
  [[nodiscard]] List<PacketPtr> pullOutgoingPackets();
private:
  [[nodiscard]] SystemObjectPtr netLoadObject(ByteArray netStore);
  [[nodiscard]] SystemClientShipPtr netLoadShip(ByteArray netStore);

  // m_ship can be a null pointer, indicating that the system is not initialized
  SystemClientShipPtr m_ship;
  HashMap<Uuid, SystemObjectPtr> m_objects;
  HashMap<Uuid, SystemClientShipPtr> m_clientShips;

  PlayerUniverseMapPtr m_universeMap;

  List<PacketPtr> m_outgoingPackets;
};


}
