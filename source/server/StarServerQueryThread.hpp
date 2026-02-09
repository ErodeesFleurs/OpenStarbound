#pragma once

#include "StarDataStreamDevices.hpp"
#include "StarHostAddress.hpp"
#include "StarThread.hpp"
#include "StarUdp.hpp"

import std;

namespace Star {

class UniverseServer;

class ServerQueryThread : public Thread {
public:
  ServerQueryThread(UniverseServer* universe, HostAddressWithPort const& bindAddress);
  ~ServerQueryThread() override;

  void start();
  void stop();

protected:
  void run() override;

private:
  static constexpr std::uint8_t A2A_PING_REQUEST = 0x69;
  static constexpr std::uint8_t A2A_PING_REPLY = 0x6a;
  static constexpr std::uint8_t A2S_CHALLENGE_REQUEST = 0x57;
  static constexpr std::uint8_t A2S_CHALLENGE_RESPONSE = 0x41;
  static constexpr std::uint8_t A2S_INFO_REQUEST = 0x54;
  static constexpr std::uint8_t A2S_INFO_REPLY = 0x49;
  static constexpr std::uint8_t A2S_PLAYER_REQUEST = 0x55;
  static constexpr std::uint8_t A2S_PLAYER_REPLY = 0x44;
  static constexpr std::uint8_t A2S_RULES_REQUEST = 0x56;
  static constexpr std::uint8_t A2S_RULES_REPLY = 0x45;
  static constexpr std::uint8_t A2S_VERSION = 0x07;
  static constexpr std::uint8_t A2S_STR_TERM = 0x00;
  static constexpr std::uint8_t A2S_EDF_GID = 0x01;
  static constexpr std::uint8_t A2S_EDF_SID = 0x10;
  static constexpr std::uint8_t A2S_EDF_TAGS = 0x20;
  static constexpr std::uint8_t A2S_EDF_STV = 0x40;
  static constexpr std::uint8_t A2S_EDF_PORT = 0x80;
  static constexpr std::uint8_t A2S_ENV_WINDOWS = 'w';
  static constexpr std::uint8_t A2S_ENV_LINUX = 'l';
  static constexpr std::uint8_t A2S_ENV_MAC = 'm';
  static constexpr std::uint8_t A2S_TYPE_DEDICATED = 'd';
  static constexpr std::uint8_t A2S_TYPE_LISTEN = 'l';
  static constexpr std::uint8_t A2S_TYPE_TV = 'p';
  static constexpr std::uint8_t A2S_VAC_OFF = 0x00;
  static constexpr std::uint8_t A2S_VAC_ON = 0x01;
  static constexpr const char* A2S_INFO_REQUEST_STRING = "Source Engine Query";
  static constexpr std::uint16_t A2S_APPID = (std::uint16_t)0xfffe;
  static constexpr std::uint16_t A2S_PACKET_SIZE = (std::uint16_t)0x4e0;
  static constexpr std::uint32_t A2S_HEAD_INT = 0xffffffff;
  static constexpr const char* GAME_DIR = "starbound";
  static constexpr const char* GAME_DESC = "Starbound";
  static constexpr const char* GAME_TYPE = "SMP";
  static constexpr std::int32_t challengeCheckInterval = 30000;
  static constexpr std::int32_t responseCacheTime = 5000;

  void sendTo(HostAddressWithPort const& address, DataStreamBuffer* ds);
  auto processPacket(HostAddressWithPort const& address, char const* data, std::size_t length) -> bool;
  void buildPlayerResponse();
  void buildRuleResponse();
  auto validChallenge(HostAddressWithPort const& address, char const* data, std::size_t length) -> bool;
  void sendChallenge(HostAddressWithPort const& address);
  void pruneChallenges();
  auto challengeRequest(HostAddressWithPort const& address, char const* data, std::size_t length) -> bool;

  // Server API
  auto serverPlayerCount() -> std::uint8_t;
  auto serverPassworded() -> bool;
  auto serverPlugins() -> const char*;
  auto serverWorldNames() -> String;

  UniverseServer* m_universe;
  UdpServer m_queryServer;
  bool m_stop;
  DataStreamBuffer m_playersResponse;
  DataStreamBuffer m_rulesResponse;
  DataStreamBuffer m_generalResponse;

  class RequestChallenge {
  public:
    RequestChallenge();
    auto before(std::uint64_t time) -> bool;
    auto getChallenge() -> std::int32_t;

  private:
    std::uint64_t m_time;
    std::int32_t m_challenge;
  };

  std::uint16_t m_serverPort;
  std::uint8_t m_maxPlayers;
  String m_serverName;
  HashMap<HostAddress, std::shared_ptr<RequestChallenge>> m_validChallenges;
  std::int64_t m_lastChallengeCheck;
  std::int64_t m_lastPlayersResponse;
  std::int64_t m_lastRulesResponse;
  std::int64_t m_lastActiveTime;
};

}// namespace Star
