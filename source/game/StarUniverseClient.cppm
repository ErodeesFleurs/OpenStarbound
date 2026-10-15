module;
#include "StarUniverseClient.inc"

export module star.universe_client;

export namespace Star {
  using ::Star::WorldTemplate;
  using ::Star::ClientContext;
  using ::Star::Sky;
  using ::Star::Packet;
  using ::Star::WorldClient;
  using ::Star::SystemWorldClient;
  using ::Star::Player;
  using ::Star::PlayerStorage;
  using ::Star::Statistics;
  using ::Star::Clock;
  using ::Star::CelestialLog;
  using ::Star::CelestialSlaveDatabase;
  using ::Star::CelestialDatabase;
  using ::Star::JsonRpcInterface;
  using ::Star::TeamClient;
  using ::Star::QuestManager;
  using ::Star::UniverseClient;
  using ::Star::LuaRoot;
}

