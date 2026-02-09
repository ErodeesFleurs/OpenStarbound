#pragma once

#include "StarConfig.hpp"
#include "StarGameTypes.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaRoot.hpp"
#include "StarShellParser.hpp"

import std;

namespace Star {

class UniverseServer;

class CommandProcessor {
public:
  CommandProcessor(UniverseServer* universe, Ptr<LuaRoot> luaRoot);

  auto adminCommand(String const& command, String const& argumentString) -> String;
  auto userCommand(ConnectionId clientId, String const& command, String const& argumentString) -> String;

private:
  static auto playerCidFromCommand(String const& player, UniverseServer* universe) -> std::optional<ConnectionId>;

  auto help(ConnectionId connectionId, String const& argumentString) -> String;
  auto admin(ConnectionId connectionId, String const& argumentString) -> String;
  auto pvp(ConnectionId connectionId, String const& argumentString) -> String;
  auto whoami(ConnectionId connectionId, String const& argumentString) -> String;

  auto warp(ConnectionId connectionId, String const& argumentString) -> String;
  auto warpRandom(ConnectionId connectionId, String const& argumentString) -> String;
  auto timewarp(ConnectionId connectionId, String const& argumentString) -> String;
  auto timescale(ConnectionId connectionId, String const& argumentString) -> String;
  auto tickrate(ConnectionId connectionId, String const& argumentString) -> String;
  auto setTileProtection(ConnectionId connectionId, String const& argumentString) -> String;
  auto setDungeonId(ConnectionId connectionId, String const& argumentString) -> String;
  auto setPlayerStart(ConnectionId connectionId, String const& argumentString) -> String;
  auto spawnItem(ConnectionId connectionId, String const& argumentString) -> String;
  auto spawnTreasure(ConnectionId connectionId, String const& argumentString) -> String;
  auto spawnMonster(ConnectionId connectionId, String const& argumentString) -> String;
  auto spawnNpc(ConnectionId connectionId, String const& argumentString) -> String;
  auto spawnVehicle(ConnectionId connectionId, String const& argumentString) -> String;
  auto spawnStagehand(ConnectionId connectionId, String const& argumentString) -> String;
  auto clearStagehand(ConnectionId connectionId, String const& argumentString) -> String;
  auto spawnLiquid(ConnectionId connectionId, String const& argumentString) -> String;
  auto kick(ConnectionId connectionId, String const& argumentString) -> String;
  auto ban(ConnectionId connectionId, String const& argumentString) -> String;
  auto unbanIp(ConnectionId connectionId, String const& argumentString) -> String;
  auto unbanUuid(ConnectionId connectionId, String const& argumentString) -> String;
  auto list(ConnectionId connectionId, String const& argumentString) -> String;
  auto clientCoordinate(ConnectionId connectionId, String const& argumentString) -> String;
  auto serverReload(ConnectionId connectionId, String const& argumentString) -> String;
  auto eval(ConnectionId connectionId, String const& lua) -> String;
  auto entityEval(ConnectionId connectionId, String const& lua) -> String;
  auto enableSpawning(ConnectionId connectionId, String const& argumentString) -> String;
  auto disableSpawning(ConnectionId connectionId, String const& argumentString) -> String;
  auto placeDungeon(ConnectionId connectionId, String const& argumentString) -> String;
  auto setUniverseFlag(ConnectionId connectionId, String const& argumentString) -> String;
  auto resetUniverseFlags(ConnectionId connectionId, String const& argumentString) -> String;
  auto addBiomeRegion(ConnectionId connectionId, String const& argumentString) -> String;
  auto expandBiomeRegion(ConnectionId connectionId, String const& argumentString) -> String;
  auto updatePlanetType(ConnectionId connectionId, String const& argumentString) -> String;
  auto setWeather(ConnectionId connectionId, String const& argumentString) -> String;
  auto setEnvironmentBiome(ConnectionId connectionId, String const& argumentString) -> String;

  mutable Mutex m_mutex;

  auto handleCommand(ConnectionId connectionId, String const& command, String const& argumentString) -> String;
  auto adminCheck(ConnectionId connectionId, String const& commandDescription) const -> std::optional<String>;
  auto localCheck(ConnectionId connectionId, String const& commandDescription) const -> std::optional<String>;
  auto makeCommandCallbacks() -> LuaCallbacks;

  UniverseServer* m_universe;
  ShellParser m_parser;

  LuaBaseComponent m_scriptComponent;
};

}// namespace Star
