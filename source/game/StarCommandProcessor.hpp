#pragma once

#include "StarGameTypes.hpp"
#include "StarShellParser.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaRoot.hpp"
#include "StarAssets.hpp"
#include "StarConfiguration.hpp"

namespace Star {

class UniverseServer;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class TreasureDatabase;
using TreasureDatabaseConstPtr = SharedPtr<TreasureDatabase const>;
class MonsterDatabase;
using MonsterDatabaseConstPtr = SharedPtr<MonsterDatabase const>;
class NpcDatabase;
using NpcDatabaseConstPtr = SharedPtr<NpcDatabase const>;
class VehicleDatabase;
using VehicleDatabaseConstPtr = SharedPtr<VehicleDatabase const>;
class StagehandDatabase;
using StagehandDatabaseConstPtr = SharedPtr<StagehandDatabase const>;
class LiquidsDatabase;
using LiquidsDatabaseConstPtr = SharedPtr<LiquidsDatabase const>;
class CommandProcessor;
using CommandProcessorPtr = SharedPtr<CommandProcessor>;

class CommandProcessor {
public:
  CommandProcessor(UniverseServer& universe,
      LuaRootPtr luaRoot,
      AssetsConstPtr assets,
      ConfigurationPtr configuration,
      ItemDatabaseConstPtr itemDatabase,
      TreasureDatabaseConstPtr treasureDatabase,
      MonsterDatabaseConstPtr monsterDatabase,
      NpcDatabaseConstPtr npcDatabase,
      VehicleDatabaseConstPtr vehicleDatabase,
      StagehandDatabaseConstPtr stagehandDatabase,
      LiquidsDatabaseConstPtr liquidsDatabase,
      function<void()> reloadRoot);

  [[nodiscard]] String adminCommand(String const& command, String const& argumentString);
  [[nodiscard]] String userCommand(ConnectionId clientId, String const& command, String const& argumentString);

private:
  [[nodiscard]] static Maybe<ConnectionId> playerCidFromCommand(String const& player, UniverseServer& universe);

  [[nodiscard]] String help(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String admin(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String pvp(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String whoami(ConnectionId connectionId, String const& argumentString);

  [[nodiscard]] String warp(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String warpRandom(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String timewarp(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String timescale(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String tickrate(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String setTileProtection(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String setDungeonId(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String setPlayerStart(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String spawnItem(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String spawnTreasure(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String spawnMonster(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String spawnNpc(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String spawnVehicle(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String spawnStagehand(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String clearStagehand(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String spawnLiquid(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String kick(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String ban(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String unbanIp(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String unbanUuid(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String list(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String clientCoordinate(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String serverReload(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String eval(ConnectionId connectionId, String const& lua);
  [[nodiscard]] String entityEval(ConnectionId connectionId, String const& lua);
  [[nodiscard]] String enableSpawning(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String disableSpawning(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String placeDungeon(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String setUniverseFlag(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String resetUniverseFlags(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String addBiomeRegion(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String expandBiomeRegion(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String updatePlanetType(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String setWeather(ConnectionId connectionId, String const& argumentString);
  [[nodiscard]] String setEnvironmentBiome(ConnectionId connectionId, String const& argumentString);

  static const StringMap<std::function<String(CommandProcessor*, ConnectionId, String)>> s_commandMap;

  mutable Mutex m_mutex;

  [[nodiscard]] String handleCommand(ConnectionId connectionId, String const& command, String const& argumentString);
  [[nodiscard]] Maybe<String> adminCheck(ConnectionId connectionId, String const& commandDescription) const;
  [[nodiscard]] Maybe<String> localCheck(ConnectionId connectionId, String const& commandDescription) const;
  [[nodiscard]] LuaCallbacks makeCommandCallbacks();

  UniverseServer& m_universe;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  ItemDatabaseConstPtr m_itemDatabase;
  TreasureDatabaseConstPtr m_treasureDatabase;
  MonsterDatabaseConstPtr m_monsterDatabase;
  NpcDatabaseConstPtr m_npcDatabase;
  VehicleDatabaseConstPtr m_vehicleDatabase;
  StagehandDatabaseConstPtr m_stagehandDatabase;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  function<void()> m_reloadRoot;
  ShellParser m_parser;

  LuaBaseComponent m_scriptComponent;
};

}
