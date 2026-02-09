#pragma once

#include "StarConfig.hpp"
#include "StarDataStreamDevices.hpp"
#include "StarException.hpp"
#include "StarTcp.hpp"
#include "StarThread.hpp"

import std;

namespace Star {

class UniverseServer;

class ServerRconClient : public Thread {
public:
  static constexpr std::uint32_t SERVERDATA_AUTH = 0x03;
  static constexpr std::uint32_t SERVERDATA_EXECCOMMAND = 0x02;
  static constexpr std::uint32_t SERVERDATA_RESPONSE_VALUE = 0x00;
  static constexpr std::uint32_t SERVERDATA_AUTH_RESPONSE = 0x02;
  static constexpr std::uint32_t SERVERDATA_AUTH_FAILURE = 0xffffffff;
  ServerRconClient(UniverseServer* universe, Ptr<TcpSocket> socket);
  ~ServerRconClient() override;

  void start();
  void stop();

protected:
  void run() override;

private:
  static constexpr std::size_t MaxPacketSize = 4096;
  using NoMoreRequests = ExceptionDerived<"NoMoreRequests">;

  void receive(std::size_t size);
  void send(std::uint32_t requestId, std::uint32_t cmd, String str = "");
  void sendAuthFailure();
  void sendCmdResponse(std::uint32_t requestId, String response);
  void closeSocket();
  void processRequest();
  auto handleCommand(String commandLine) -> String;

  UniverseServer* m_universe;
  Ptr<TcpSocket> m_socket;
  DataStreamBuffer m_packetBuffer;
  bool m_stop;
  bool m_authed;
  String m_rconPassword;
};
}// namespace Star
