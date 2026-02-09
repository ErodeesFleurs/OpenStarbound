#pragma once

#include "StarConfig.hpp"
#include "StarRpcThreadPromise.hpp"
#include "StarThread.hpp"
#include "StarWorldServer.hpp"

import std;

namespace Star {

// Runs a WorldServer in a separate thread and guards exceptions that occur in
// it.  All methods are designed to not throw exceptions, but will instead log
// the error and trigger the WorldServerThread error state.
class WorldServerThread : public Thread {
public:
  struct Message {
    String message;
    JsonArray args;
    RpcThreadPromiseKeeper<Json> promise;
  };

  using WorldServerAction = std::function<void(WorldServerThread*, WorldServer*)>;

  WorldServerThread(Ptr<WorldServer> server, WorldId worldId);
  ~WorldServerThread() override;

  auto worldId() const -> WorldId;

  void start();
  // Signals the WorldServerThread to stop and then joins it
  void stop();
  void setPause(std::shared_ptr<const std::atomic<bool>> pause);

  // An exception occurred from the actual WorldServer itself and the
  // WorldServerThread has stopped running.
  auto serverErrorOccurred() -> bool;
  auto shouldExpire() -> bool;

  auto spawnTargetValid(SpawnTarget const& spawnTarget) -> bool;

  auto addClient(ConnectionId clientId, SpawnTarget const& spawnTarget, bool isLocal, bool isAdmin = false, NetCompatibilityRules netRules = {}) -> bool;
  // Returns final outgoing packets
  auto removeClient(ConnectionId clientId) -> List<Ptr<Packet>>;

  auto clients() const -> List<ConnectionId>;
  auto hasClient(ConnectionId clientId) const -> bool;
  auto noClients() const -> bool;

  // Clients that have caused an error with incoming packets are removed from
  // the world and no further packets are handled from them.  They are still
  // added to this WorldServerThread, and must be removed and the final
  // outgoing packets should be sent to them.
  auto erroredClients() const -> List<ConnectionId>;

  void pushIncomingPackets(ConnectionId clientId, List<Ptr<Packet>> packets);
  auto pullOutgoingPackets(ConnectionId clientId) -> List<Ptr<Packet>>;

  auto playerRevivePosition(ConnectionId clientId) const -> std::optional<Vec2F>;

  // Worlds use this to notify the universe server that their celestial type should change
  auto pullNewPlanetType() -> std::optional<std::pair<String, String>>;

  // Executes the given action on the world in a thread safe context.  This
  // does *not* catch exceptions thrown by the action or set the server error
  // flag.
  void executeAction(WorldServerAction action);

  // If a callback is set here, then this is called after every world update,
  // also in a thread safe context.
  void setUpdateAction(WorldServerAction updateAction);

  //
  void passMessages(List<Message>&& messages);

  void unloadAll(bool force = false);

  // Syncs all active sectors to disk and reads the full content of the world
  // into memory, useful for the ship.
  auto readChunks() -> WorldChunks;

protected:
  void run() override;

private:
  void update(WorldServerFidelity fidelity);
  void sync();

  mutable RecursiveMutex m_mutex;

  HashSet<ConnectionId> m_clients;

  Ptr<WorldServer> m_worldServer;
  WorldId m_worldId;
  WorldServerAction m_updateAction;

  mutable RecursiveMutex m_queueMutex;
  Map<ConnectionId, List<Ptr<Packet>>> m_incomingPacketQueue;
  Map<ConnectionId, List<Ptr<Packet>>> m_outgoingPacketQueue;

  mutable RecursiveMutex m_messageMutex;
  List<Message> m_messages;

  std::atomic<bool> m_stop;
  std::shared_ptr<const std::atomic<bool>> m_pause;
  mutable std::atomic<bool> m_errorOccurred;
  mutable std::atomic<bool> m_shouldExpire;
};

}// namespace Star
