#pragma once

#include "StarThread.hpp"
#include "StarTcp.hpp"
#include "StarMap.hpp"
#include "StarServerRconClient.hpp"
#include "StarConfiguration.hpp"

namespace Star {

class UniverseServer;
class ServerRconThread;

class ServerRconThread : public Thread {
public:
  ServerRconThread(UniverseServer* universe, HostAddressWithPort const& address, ConfigurationPtr configuration);
  ~ServerRconThread();

  void start();
  void stop();

protected:
  virtual void run();

private:
  void clearClients(bool all = false);

  UniverseServer* m_universe;
  TcpServer m_rconServer;
  String m_rconPassword;
  int m_rconTimeout;
  bool m_stop;
  HashMap<HostAddress, ServerRconClientPtr> m_clients;
};

}
