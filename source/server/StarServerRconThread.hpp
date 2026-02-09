#pragma once

#include "StarConfig.hpp"
#include "StarMap.hpp"
#include "StarServerRconClient.hpp"
#include "StarTcp.hpp"
#include "StarThread.hpp"

namespace Star {

class ServerRconThread : public Thread {
public:
  ServerRconThread(UniverseServer* universe, HostAddressWithPort const& address);
  ~ServerRconThread() override;

  void start();
  void stop();

protected:
  void run() override;

private:
  void clearClients(bool all = false);

  UniverseServer* m_universe;
  TcpServer m_rconServer;
  bool m_stop;
  HashMap<HostAddress, Ptr<ServerRconClient>> m_clients;
};

}// namespace Star
