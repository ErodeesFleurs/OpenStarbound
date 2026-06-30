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

  String adminCommand(String const& command, String const& argumentString);
  String userCommand(ConnectionId clientId, String const& command, String const& argumentString);

private:
  static Maybe<ConnectionId> playerCidFromCommand(String const& player, UniverseServer& universe);

  String help(ConnectionId connectionId, String const& argumentString);
  String admin(ConnectionId connectionId, String const& argumentString);
  String pvp(ConnectionId connectionId, String const& argumentString);
  String whoami(ConnectionId connectionId, String const& argumentString);
	
  String warp(ConnectionId connectionId, String const& argumentString);
  String warpRandom(ConnectionId connectionId, String const& argumentString);
  String timewarp(ConnectionId connectionId, String const& argumentString);
  String timescale(ConnectionId connectionId, String const& argumentString);
  String tickrate(ConnectionId connectionId, String const& argumentString);
  String setTileProtection(ConnectionId connectionId, String const& argumentString);
  String setDungeonId(ConnectionId connectionId, String const& argumentString);
  String setPlayerStart(ConnectionId connectionId, String const& argumentString);
  String spawnItem(ConnectionId connectionId, String const& argumentString);
  String spawnTreasure(ConnectionId connectionId, String const& argumentString);
  String spawnMonster(ConnectionId connectionId, String const& argumentString);
  String spawnNpc(ConnectionId connectionId, String const& argumentString);
  String spawnVehicle(ConnectionId connectionId, String const& argumentString);
  String spawnStagehand(ConnectionId connectionId, String const& argumentString);
  String clearStagehand(ConnectionId connectionId, String const& argumentString);
  String spawnLiquid(ConnectionId connectionId, String const& argumentString);
  String kick(ConnectionId connectionId, String const& argumentString);
  String ban(ConnectionId connectionId, String const& argumentString);
  String unbanIp(ConnectionId connectionId, String const& argumentString);
  String unbanUuid(ConnectionId connectionId, String const& argumentString);
  String list(ConnectionId connectionId, String const& argumentString);
  String clientCoordinate(ConnectionId connectionId, String const& argumentString);
  String serverReload(ConnectionId connectionId, String const& argumentString);
  String eval(ConnectionId connectionId, String const& lua);
  String entityEval(ConnectionId connectionId, String const& lua);
  String enableSpawning(ConnectionId connectionId, String const& argumentString);
  String disableSpawning(ConnectionId connectionId, String const& argumentString);
  String placeDungeon(ConnectionId connectionId, String const& argumentString);
  String setUniverseFlag(ConnectionId connectionId, String const& argumentString);
  String resetUniverseFlags(ConnectionId connectionId, String const& argumentString);
  String addBiomeRegion(ConnectionId connectionId, String const& argumentString);
  String expandBiomeRegion(ConnectionId connectionId, String const& argumentString);
  String updatePlanetType(ConnectionId connectionId, String const& argumentString);
  String setWeather(ConnectionId connectionId, String const& argumentString);
  String setEnvironmentBiome(ConnectionId connectionId, String const& argumentString);

  static const StringMap<std::function<String(CommandProcessor*, ConnectionId, String)>> s_commandMap;

  mutable Mutex m_mutex;

  String handleCommand(ConnectionId connectionId, String const& command, String const& argumentString);
  Maybe<String> adminCheck(ConnectionId connectionId, String const& commandDescription) const;
  Maybe<String> localCheck(ConnectionId connectionId, String const& commandDescription) const;
  LuaCallbacks makeCommandCallbacks();

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
