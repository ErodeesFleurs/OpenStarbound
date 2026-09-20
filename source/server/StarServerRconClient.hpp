#pragma once

#include "StarThread.hpp"
#include "StarTcp.hpp"
#include "StarMap.hpp"
#include "StarGameTypes.hpp"
#include "StarDataStreamDevices.hpp"

namespace Star {

class UniverseServer;

class ServerRconClient : public Thread {
public:
  static const uint32_t SERVERDATA_AUTH = 0x03;
  static const uint32_t SERVERDATA_EXECCOMMAND = 0x02;
  static const uint32_t SERVERDATA_RESPONSE_VALUE = 0x00;
  static const uint32_t SERVERDATA_AUTH_RESPONSE = 0x02;
  static const uint32_t SERVERDATA_AUTH_FAILURE = 0xffffffff;
  ServerRconClient(UniverseServer* universe, TcpSocketPtr socket);
  ~ServerRconClient();

  void start();
  void stop();

protected:
  virtual void run();

private:
  static constexpr size_t MaxPacketSize = 4096;
  // Requests are small (a command line); anything larger is refused instead of
  // being allocated, since the length is client supplied.
  static constexpr size_t MaxReceivePacketSize = MaxPacketSize * 4;
  struct NoMoreRequestsTag {
    static constexpr char const* name() { return "NoMoreRequests"; }
  };
  using NoMoreRequests = StarError<NoMoreRequestsTag, StarException>;
  struct OversizedPacketTag {
    static constexpr char const* name() { return "OversizedPacket"; }
  };
  using OversizedPacket = StarError<OversizedPacketTag, StarException>;

  void receive(size_t size);
  void send(uint32_t requestId, uint32_t cmd, String str = "");
  void sendAuthFailure();
  void sendCmdResponse(uint32_t requestId, String response);
  void closeSocket();
  void processRequest();
  ServerCommandResult handleCommand(String commandLine);

  UniverseServer* m_universe;
  TcpSocketPtr m_socket;
  DataStreamBuffer m_packetBuffer;
  bool m_stop;
  bool m_authed;
  String m_rconPassword;
  
  HashMap<uint32_t,RpcPromise<String>> m_commandPromises;
};
typedef shared_ptr<ServerRconClient> ServerRconClientPtr;
}
