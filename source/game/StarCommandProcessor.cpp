#include "StarCommandProcessor.hpp"

#include "StarAssets.hpp"
#include "StarCelestialDatabase.hpp"
#include "StarConfig.hpp"
#include "StarItem.hpp"
#include "StarItemDatabase.hpp"
#include "StarItemDrop.hpp"
#include "StarJsonExtra.hpp"
#include "StarLexicalCast.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLogging.hpp"
#include "StarMonster.hpp"
#include "StarNpc.hpp"// IWYU pragma: export
#include "StarNpcDatabase.hpp"
#include "StarPlayer.hpp"
#include "StarRoot.hpp"
#include "StarStagehand.hpp"
#include "StarStagehandDatabase.hpp"
#include "StarTreasure.hpp"
#include "StarUniverseServer.hpp"
#include "StarUniverseServerLuaBindings.hpp"
#include "StarVehicle.hpp"
#include "StarVehicleDatabase.hpp"
#include "StarWorldLuaBindings.hpp"
#include "StarWorldServer.hpp"

import std;

namespace Star {

CommandProcessor::CommandProcessor(UniverseServer* universe, Ptr<LuaRoot> luaRoot)
    : m_universe(universe) {
  ConstPtr<Assets> assets = Root::singleton().assets();
  m_scriptComponent.addCallbacks("universe", LuaBindings::makeUniverseServerCallbacks(m_universe));
  m_scriptComponent.addCallbacks("CommandProcessor", makeCommandCallbacks());
  m_scriptComponent.setScripts(jsonToStringList(assets->json("/universe_server.config:commandProcessorScripts")));
  luaRoot->luaEngine().setNullTerminated(false);
  m_scriptComponent.setLuaRoot(luaRoot);
  m_scriptComponent.init();
}

auto CommandProcessor::adminCommand(String const& command, String const& argumentString) -> String {
  MutexLocker locker(m_mutex);
  return handleCommand(ServerConnectionId, command, argumentString);
}

auto CommandProcessor::userCommand(ConnectionId connectionId, String const& command, String const& argumentString) -> String {
  MutexLocker locker(m_mutex);
  if (connectionId == ServerConnectionId)
    throw StarException("CommandProcessor::userCommand called with ServerConnectionId");
  return handleCommand(connectionId, command, argumentString);
}

auto CommandProcessor::help(ConnectionId connectionId, String const& argumentString) -> String {
  auto arguments = m_parser.tokenizeToStringList(argumentString);

  auto assets = Root::singleton().assets();
  auto basicCommands = assets->json("/help.config:basicCommands");
  auto openSbCommands = assets->json("/help.config:openSbCommands");
  auto adminCommands = assets->json("/help.config:adminCommands");
  auto debugCommands = assets->json("/help.config:debugCommands");
  auto openSbDebugCommands = assets->json("/help.config:openSbDebugCommands");

  if (arguments.size()) {
    auto const& cmdName = arguments[0];
    for (auto* cmdSet : {&basicCommands, &openSbCommands, &adminCommands,
                         &debugCommands, &openSbDebugCommands}) {
      if (auto helpText = cmdSet->optString(cmdName)) {
        return *helpText;
      }
    }
  }

  String res = "";

  auto commandDescriptions = [&](Json const& commandConfig) -> String {
    StringList commandList = commandConfig.toObject().keys();
    sort(commandList);
    return "/" + commandList.join(", /");
  };

  String basicHelpFormat = assets->json("/help.config:basicHelpText").toString();
  res = res + vstrf(basicHelpFormat.utf8Ptr(), commandDescriptions(basicCommands));

  String openSbHelpFormat = assets->json("/help.config:openSbHelpText").toString();
  res = res + "\n" + vstrf(openSbHelpFormat.utf8Ptr(), commandDescriptions(openSbCommands));

  if (!adminCheck(connectionId, "")) {
    String adminHelpFormat = assets->json("/help.config:adminHelpText").toString();
    res = res + "\n" + vstrf(adminHelpFormat.utf8Ptr(), commandDescriptions(adminCommands));

    String debugHelpFormat = assets->json("/help.config:debugHelpText").toString();
    res = res + "\n" + vstrf(debugHelpFormat.utf8Ptr(), commandDescriptions(debugCommands));

    String openSbDebugHelpFormat = assets->json("/help.config:openSbDebugHelpText").toString();
    res = res + "\n" + vstrf(openSbDebugHelpFormat.utf8Ptr(), commandDescriptions(openSbDebugCommands));
  }

  res = res + "\n" + basicCommands.getString("help");

  return res;
}

auto CommandProcessor::admin(ConnectionId connectionId, String const&) -> String {
  auto config = Root::singleton().configuration();
  if (m_universe->canBecomeAdmin(connectionId)) {
    if (connectionId == ServerConnectionId)
      return "Invalid client state";

    if (!config->get("allowAdminCommands").toBool())
      return "Admin commands disabled on this server.";

    bool wasAdmin = m_universe->isAdmin(connectionId);
    m_universe->setAdmin(connectionId, !wasAdmin);

    if (!wasAdmin)
      return strf("Admin privileges now given to player {}", m_universe->clientNick(connectionId));
    else
      return strf("Admin privileges taken away from {}", m_universe->clientNick(connectionId));
  } else {
    return "Insufficient privileges to make self admin.";
  }
}

auto CommandProcessor::pvp(ConnectionId connectionId, String const&) -> String {
  if (!m_universe->isPvp(connectionId)) {
    m_universe->setPvp(connectionId, true);
    if (m_universe->isPvp(connectionId))
      m_universe->adminBroadcast(strf("Player {} is now PVP", m_universe->clientNick(connectionId)));
  } else {
    m_universe->setPvp(connectionId, false);
    if (!m_universe->isPvp(connectionId))
      m_universe->adminBroadcast(strf("Player {} is a big wimp and is no longer PVP", m_universe->clientNick(connectionId)));
  }

  if (m_universe->isPvp(connectionId))
    return "PVP active";
  else
    return "PVP inactive";
}

auto CommandProcessor::whoami(ConnectionId connectionId, String const&) -> String {
  return strf("Server: You are {}. You are {}an Admin",
              m_universe->clientNick(connectionId),
              m_universe->isAdmin(connectionId) ? "" : "not ");
}

auto CommandProcessor::warp(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "do the space warp again"))
    return *errorMsg;

  try {
    m_universe->clientWarpPlayer(connectionId, parseWarpAction(argumentString));
    return "Lets do the space warp again";
  } catch (StarException const& e) {
    Logger::warn("Could not parse warp target: {}", outputException(e, false));
    return strf("Could not parse the argument {} as a warp target", argumentString);
  }
}

auto CommandProcessor::warpRandom(ConnectionId connectionId, String const& typeName) -> String {
  if (auto errorMsg = adminCheck(connectionId, "warp to random world"))
    return *errorMsg;

  Vec2I size = {2, 2};
  CelestialDatabase& celestialDatabase = m_universe->celestialDatabase();
  std::optional<CelestialCoordinate> target = {};

  auto validPlanet = [&celestialDatabase, &typeName](CelestialCoordinate const& p) -> bool {
    if (auto celestialParams = celestialDatabase.parameters(p)) {
      if (auto visitableParams = celestialParams->visitableParameters()) {
        if (visitableParams->typeName == typeName)
          return true;
      }
    }
    return false;
  };

  while (!target) {
    RectI region = RectI::withSize(Vec2I(Random::randi32(), Random::randi32()), size);

    while (!celestialDatabase.scanRegionFullyLoaded(region)) {
      celestialDatabase.scanSystems(region);
    }
    auto systems = celestialDatabase.scanSystems(region);
    for (auto s : systems) {
      for (auto planet : celestialDatabase.children(s)) {
        if (validPlanet(planet))
          target = planet;
        if (!target) {
          for (auto moon : celestialDatabase.children(planet)) {
            if (validPlanet(moon)) {
              target = moon;
              break;
            }
          }
        }
      }
    }

    if (size.magnitude() > 1024)
      return "could not find a matching world";
    size *= 2;
  }

  m_universe->clientWarpPlayer(connectionId, WarpToWorld(CelestialWorldId(*target)));
  return strf("warping to {}", *target);
}

auto CommandProcessor::timewarp(ConnectionId connectionId, String const& argumentsString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "do the time warp again"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentsString);
  if (arguments.empty())
    return "Not enough arguments to /timewarp";

  try {
    auto time = lexicalCast<double>(arguments.at(0));
    if (time == 0.0)
      return "You suck at time travel.";
    else if (time < 0.0 && (arguments.size() < 2 || arguments[1] != "please"))
      return "Great Scott! We can't go back in time!";

    m_universe->universeClock()->adjustTime(time);
    return time > 0.0 ? "It's just a jump to the left..." : "And then a step to the right...";
  } catch (BadLexicalCast const&) {
    return strf("Could not parse the argument {} as a time adjustment", arguments[0]);
  }
}

auto CommandProcessor::timescale(ConnectionId connectionId, String const& argumentsString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "mess with time"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentsString);

  if (arguments.empty())
    return strf("Current timescale is {:6.6f}x", GlobalTimescale);

  float timescale = clamp(lexicalCast<float>(arguments[0]), 0.001f, 32.0f);
  m_universe->setTimescale(timescale);
  return strf("Set timescale to {:6.6f}x", timescale);
}

auto CommandProcessor::tickrate(ConnectionId connectionId, String const& argumentsString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "change the tick rate"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentsString);

  if (arguments.empty())
    return strf("Current tick rate is {:4.2f}Hz", 1.0f / ServerGlobalTimestep);

  float tickRate = clamp(lexicalCast<float>(arguments[0]), 5.f, 500.f);
  m_universe->setTickRate(tickRate);
  return strf("Set tick rate to {:4.2f}Hz", tickRate);
}

auto CommandProcessor::setTileProtection(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "modify world properties")) {
    return *errorMsg;
  }

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.size() < 2)
    return "Not enough arguments to /settileprotection. Use /settileprotection <dungeonId> <protected>";

  try {
    bool isProtected = Json::parse(arguments.takeLast()).toBool();
    List<DungeonId> dungeonIds;
    for (auto& banana : arguments) {
      auto slices = banana.split("..");
      auto it = slices.begin();
      DungeonId previous = 0;
      while (it != slices.end()) {
        auto current = lexicalCast<DungeonId>(*it);
        dungeonIds.append(current);
        if (it++ != slices.begin() && previous != current) {
          if (current < previous)
            std::swap(previous, current);
          for (DungeonId id = previous + 1; id != current; ++id)
            dungeonIds.append(id);
        }
        previous = current;
      }
    }
    size_t changed = 0;
    if (!m_universe->executeForClient(connectionId, [&](WorldServer* world, Ptr<Player> const&) -> void {
          changed = world->setTileProtection(dungeonIds, isProtected);
        })) {
      return "Invalid client state";
    }
    String output = strf("{} {} dungeon IDs", isProtected ? "Protected" : "Unprotected", changed);
    return changed < dungeonIds.size() ? strf("{} ({} unchanged)", output, dungeonIds.size() - changed) : output;
  } catch (BadLexicalCast const&) {
    return strf("Could not parse /settileprotection parameters. Use /settileprotection <dungeonId...> <protected>", argumentString);
  }
}

auto CommandProcessor::setDungeonId(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "set dungeon id")) {
    return *errorMsg;
  }

  auto arguments = m_parser.tokenizeToStringList(argumentString);
  if (arguments.size() < 1)
    return "Not enough arguments to /setdungeonid. Use /setdungeonid <dungeonId>";

  try {
    auto dungeonId = lexicalCast<DungeonId>(arguments.at(0));

    bool done = m_universe->executeForClient(connectionId, [dungeonId](WorldServer* world, Ptr<Player> const& player) -> void {
      world->setDungeonId(RectI::withSize(Vec2I(player->aimPosition()), Vec2I(1, 1)), dungeonId);
    });

    return done ? "" : "Failed to set dungeon id.";
  } catch (BadLexicalCast const&) {
    return strf("Could not parse /setdungeonid parameters. Use /setdungeonid <dungeonId>!", argumentString);
  }
}

auto CommandProcessor::setPlayerStart(ConnectionId connectionId, String const&) -> String {
  if (auto errorMsg = adminCheck(connectionId, "modify world properties"))
    return *errorMsg;

  m_universe->executeForClient(connectionId, [](WorldServer* world, Ptr<Player> const& player) -> void {
    world->setPlayerStart(player->position() + player->feetOffset());
  });

  return "";
}

auto CommandProcessor::spawnItem(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "spawn items"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.empty())
    return "Not enough arguments to /spawnitem";

  try {
    String kind = arguments.at(0);
    Json parameters = JsonObject();
    unsigned amount = 1;
    std::optional<float> level;
    std::optional<std::uint64_t> seed;

    if (arguments.size() >= 2)
      amount = lexicalCast<unsigned>(arguments.at(1));

    if (arguments.size() >= 3)
      parameters = Json::parse(arguments.at(2));

    if (arguments.size() >= 4)
      level = lexicalCast<float>(arguments.at(3));

    if (arguments.size() >= 5)
      seed = lexicalCast<std::uint64_t>(arguments.at(4));

    bool done = m_universe->executeForClient(connectionId, [&](WorldServer* world, Ptr<Player> const& player) -> void {
      ConstPtr<ItemDatabase> itemDatabase = Root::singleton().itemDatabase();
      world->addEntity(ItemDrop::createRandomizedDrop(itemDatabase->item(ItemDescriptor(kind, amount, parameters), level, seed, true), player->aimPosition()));
    });

    return done ? "" : "Invalid client state";
  } catch (JsonParsingException const& exception) {
    Logger::warn("Error while processing /spawnitem '{}' command. Json parse problem: {}", arguments.at(0), outputException(exception, false));
    return "Could not parse item parameters";
  } catch (ItemException const& exception) {
    Logger::warn("Error while processing /spawnitem '{}' command. Item instantiation problem: {}", arguments.at(0), outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  } catch (BadLexicalCast const& exception) {
    Logger::warn("Error while processing /spawnitem command. Number expected. Got something else: {}", outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  } catch (StarException const& exception) {
    Logger::warn("Error while processing /spawnitem command '{}', exception caught: {}", argumentString, outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  }
}

auto CommandProcessor::spawnTreasure(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "spawn items"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.empty())
    return "Not enough arguments to /spawntreasure";

  try {
    String treasurePool = arguments.at(0);
    unsigned level = 1;

    if (arguments.size() >= 2)
      level = lexicalCast<unsigned>(arguments.at(1));

    bool done = m_universe->executeForClient(connectionId, [&](WorldServer* world, Ptr<Player> const& player) -> void {
      ConstPtr<TreasureDatabase> treasureDatabase = Root::singleton().treasureDatabase();
      for (auto const& treasureItem : treasureDatabase->createTreasure(treasurePool, level, Random::randu64()))
        world->addEntity(ItemDrop::createRandomizedDrop(treasureItem, player->aimPosition()));
    });

    return done ? "" : "Invalid client state";
  } catch (JsonParsingException const& exception) {
    Logger::warn("Error while processing /spawntreasure '{}' command. Json parse problem: {}", arguments.at(0), outputException(exception, false));
    return "Could not parse item parameters";
  } catch (ItemException const& exception) {
    Logger::warn("Error while processing /spawntreasure '{}' command. Item instantiation problem: {}", arguments.at(0), outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  } catch (BadLexicalCast const& exception) {
    Logger::warn("Error while processing /spawntreasure command. Number expected. Got something else: {}", outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  } catch (StarException const& exception) {
    Logger::warn("Error while processing /spawntreasure command '{}', exception caught: {}", argumentString, outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  }
}

auto CommandProcessor::spawnMonster(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "spawn monsters"))
    return *errorMsg;

  try {
    auto arguments = m_parser.tokenizeToStringList(argumentString);

    auto monsterDatabase = Root::singleton().monsterDatabase();
    Ptr<Monster> monster;

    float level = 1;
    if (arguments.size() >= 2)
      level = lexicalCast<float>(arguments.at(1));

    Json parameters = JsonObject();
    if (arguments.size() >= 3)
      parameters = parameters.setAll(Json::parse(arguments.at(2)).toObject());

    monster = monsterDatabase->createMonster(monsterDatabase->randomMonster(arguments.at(0), parameters.toObject()), level);
    bool done = m_universe->executeForClient(connectionId,
                                             [&](WorldServer* world, Ptr<Player> const& player) -> void {
                                               monster->setPosition(player->aimPosition());
                                               world->addEntity(monster);
                                             });

    return done ? "" : "Invalid client state";
  } catch (StarException const& exception) {
    Logger::warn("Could not spawn Monster of type '{}', exception caught: {}", argumentString, outputException(exception, false));
    return strf("Could not spawn Monster of type '{}'", argumentString);
  }
}

auto CommandProcessor::spawnNpc(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "spawn NPCs"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  try {
    ConstPtr<NpcDatabase> npcDatabase = Root::singleton().npcDatabase();
    float npcLevel = 1;
    std::uint64_t seed = Random::randu64();
    Json overrides;

    if (arguments.size() < 2)
      return "You must specify a species and NPC type to spawn.";

    if (arguments.size() >= 3)
      npcLevel = lexicalCast<float>(arguments.at(2));
    if (arguments.size() >= 4)
      seed = lexicalCast<std::uint64_t>(arguments.at(3));
    if (arguments.size() >= 5)
      overrides = Json::parse(arguments.at(4)).toObject();

    auto npc = npcDatabase->createNpc(npcDatabase->generateNpcVariant(arguments.at(0), arguments.at(1), npcLevel, seed, overrides));
    bool done = m_universe->executeForClient(connectionId, [&](WorldServer* world, Ptr<Player> const& player) -> void {
      npc->setPosition(player->aimPosition());
      world->addEntity(npc);
    });

    return done ? "" : "Invalid client state";
  } catch (StarException const& exception) {
    Logger::warn("Could not spawn NPC of species '{}', exception caught: {}", argumentString, outputException(exception, true));
    return strf("Could not spawn NPC of species '{}'", argumentString);
  }
}

auto CommandProcessor::spawnVehicle(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "spawn vehicles"))
    return *errorMsg;

  try {
    ConstPtr<VehicleDatabase> vehicleDatabase = Root::singleton().vehicleDatabase();
    auto arguments = m_parser.tokenizeToStringList(argumentString);

    Ptr<Vehicle> vehicle;

    String name = arguments.at(0);

    Json parameters = JsonObject();
    if (arguments.size() >= 2)
      parameters = Json::parse(arguments.at(1)).toObject();

    vehicle = vehicleDatabase->create(name, parameters);
    bool done = m_universe->executeForClient(connectionId,
                                             [&](WorldServer* world, Ptr<Player> const& player) -> void {
                                               vehicle->setPosition(player->aimPosition());
                                               world->addEntity(std::move(vehicle));
                                             });

    return done ? "" : "Invalid client state";
  } catch (StarException const& exception) {
    Logger::warn("Could not spawn vehicle, exception caught: {}", outputException(exception, false));
    return strf("Could not spawn vehicle");
  }
}

auto CommandProcessor::spawnStagehand(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "spawn stagehands"))
    return *errorMsg;

  try {
    auto arguments = m_parser.tokenizeToStringList(argumentString);

    auto stagehandDatabase = Root::singleton().stagehandDatabase();

    Json parameters = JsonObject();
    if (arguments.size() >= 2)
      parameters = Json::parse(arguments.at(1)).toObject();

    auto stagehand = stagehandDatabase->createStagehand(arguments.at(0), parameters);
    bool done = m_universe->executeForClient(connectionId, [&](WorldServer* world, Ptr<Player> player) -> void {
      stagehand->setPosition(player->aimPosition());
      world->addEntity(stagehand);
    });

    return done ? "" : "Invalid client state";
  } catch (StarException const& exception) {
    Logger::warn("Could not spawn Stagehand of type '{}', exception caught: {}", argumentString, outputException(exception, false));
    return strf("Could not spawn Stagehand of type '{}'", argumentString);
  }
}

auto CommandProcessor::clearStagehand(ConnectionId connectionId, String const&) -> String {
  if (auto errorMsg = adminCheck(connectionId, "remove stagehands"))
    return *errorMsg;

  unsigned removed = 0;
  bool done = m_universe->executeForClient(connectionId,
                                           [&](WorldServer* world, Ptr<Player> player) -> void {
                                             auto queryRect = RectF::withCenter(player->aimPosition(), Vec2F{2, 2});
                                             for (auto stagehand : world->query<Stagehand>(queryRect)) {
                                               world->removeEntity(stagehand->entityId(), true);
                                               ++removed;
                                             }
                                           });
  return done ? strf("Removed {} stagehands", removed) : "Invalid client state";
}

auto CommandProcessor::spawnLiquid(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "spawn liquid"))
    return *errorMsg;

  try {
    auto arguments = m_parser.tokenizeToStringList(argumentString);

    ConstPtr<LiquidsDatabase> liquidsDatabase = Root::singleton().liquidsDatabase();

    if (!liquidsDatabase->isLiquidName(arguments.at(0)))
      return strf("No such liquid {}", arguments.at(0));

    LiquidId liquid = liquidsDatabase->liquidId(arguments.at(0));

    float quantity = 1.0f;
    if (arguments.size() > 1) {
      if (auto maybeQuantity = maybeLexicalCast<float>(arguments.at(1)))
        quantity = *maybeQuantity;
      else
        return strf("Could not parse quantity value '{}'", arguments.at(1));
    }

    bool done = m_universe->executeForClient(connectionId, [&](WorldServer* world, Ptr<Player> const& player) -> void {
      world->modifyTile(Vec2I(player->aimPosition().floor()), PlaceLiquid{.liquid = liquid, .liquidLevel = quantity}, true);
    });
    return done ? "" : "Invalid client state";

  } catch (StarException const& exception) {
    Logger::warn(
      "Could not spawn liquid '{}', exception caught: {}", argumentString, outputException(exception, false));
    return "Could not spawn liquid.";
  }
}

auto CommandProcessor::kick(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "kick a user"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.empty())
    return "No player specified";

  auto toKick = playerCidFromCommand(arguments[0], m_universe);
  if (!toKick)
    return strf("No user with specifier {} found.", arguments[0]);

  // Like IRC, if only the nick is passed then the nick is used as the reason
  if (arguments.size() == 1)
    arguments.append(m_universe->clientNick(*toKick));

  m_universe->disconnectClient(*toKick, arguments[1]);

  return strf("Successfully kicked user with specifier {}. ConnectionId: {}. Reason given: {}",
              arguments[0],
              toKick,
              arguments[1]);
}

auto CommandProcessor::ban(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "ban a user"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.empty())
    return "No player specified";

  auto toKick = playerCidFromCommand(arguments[0], m_universe);
  if (!toKick)
    return strf("No user with specifier {} found.", arguments[0]);

  String reason = arguments[0];
  if (arguments.size() < 2)
    reason = m_universe->clientNick(*toKick);
  else
    reason = arguments[1];

  std::pair<bool, bool> type = {true, true};

  if (arguments.size() >= 3) {
    if (arguments[2] == "ip") {
      type = {true, false};
    } else if (arguments[2] == "uuid") {
      type = {false, true};
    } else if (arguments[2] == "both") {
      type = {true, true};
    } else {
      return strf("Invalid argument {} passed as ban type to /ban.  Options are ip, uuid, or both.", arguments[2]);
    }
  }

  std::optional<int> banTime;
  if (arguments.size() == 4) {
    try {
      banTime = lexicalCast<int>(arguments[3]);
    } catch (BadLexicalCast const&) {
      return strf("Invalid argument {} passed as ban time to /ban.", arguments[3]);
    }
  }

  m_universe->banUser(*toKick, reason, type, banTime);

  return strf("Successfully kicked user with specifier {}. ConnectionId: {}. Reason given: {}",
              arguments[0], toKick, reason);
}

auto CommandProcessor::unbanIp(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "unban a user"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.empty())
    return "No IP specified";

  bool success = m_universe->unbanIp(arguments[0]);

  if (success)
    return strf("Successfully removed IP {} from ban list", arguments[0]);
  else
    return strf("'{}' is not a valid IP or was not found in the bans list", arguments[0]);
}

auto CommandProcessor::unbanUuid(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "unban a user"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.empty())
    return "No UUID specified";

  bool success = m_universe->unbanUuid(arguments[0]);

  if (success)
    return strf("Successfully removed UUID {} from ban list", arguments[0]);
  else
    return strf("'{}' is not a valid UUID or was not found in the bans list", arguments[0]);
}

auto CommandProcessor::list(ConnectionId connectionId, String const&) -> String {
  if (auto errorMsg = adminCheck(connectionId, "list clients"))
    return *errorMsg;

  StringList res;

  auto assets = Root::singleton().assets();
  for (auto cid : m_universe->clientIds())
    res.append(strf("${} : {} : $${}", cid, m_universe->clientNick(cid), m_universe->uuidForClient(cid)->hex()));

  return res.join("\n");
}

auto CommandProcessor::clientCoordinate(ConnectionId connectionId, String const& argumentString) -> String {
  ConnectionId targetClientId = connectionId;
  String targetLabel = "Your";
  auto arguments = m_parser.tokenizeToStringList(argumentString);
  if (!adminCheck(connectionId, "find other players")) {
    if (arguments.size() > 0) {
      auto cid = playerCidFromCommand(arguments[0], m_universe);
      if (!cid)
        return strf("No user with specifier {} found.", arguments[0]);
      targetClientId = *cid;
      targetLabel = strf("Client {}'s", arguments[0]);
    }
  }

  if (targetClientId) {
    auto worldId = m_universe->clientWorld(targetClientId);
    return strf("{} current location is {}", targetLabel, worldId);
  } else {
    return "";
  }
}

auto CommandProcessor::serverReload(ConnectionId connectionId, String const&) -> String {
  if (auto errorMsg = adminCheck(connectionId, "trigger root reload"))
    return *errorMsg;

  auto& root = Root::singleton();
  root.reload();
  root.fullyLoad();
  return "";
}

auto CommandProcessor::eval(ConnectionId connectionId, String const& lua) -> String {
  if (auto errorMsg = localCheck(connectionId, "execute server script"))
    return *errorMsg;

  if (auto errorMsg = adminCheck(connectionId, "execute server script"))
    return *errorMsg;

  return toString(m_scriptComponent.context()->eval(lua));
}

auto CommandProcessor::entityEval(ConnectionId connectionId, String const& lua) -> String {
  if (auto errorMsg = localCheck(connectionId, "execute server entity script"))
    return *errorMsg;

  if (auto errorMsg = adminCheck(connectionId, "execute server entity script"))
    return *errorMsg;

  String message;
  bool done = m_universe->executeForClient(connectionId,
                                           [&lua, &message](WorldServer* world, Ptr<Player> const& player) -> void {
                                             auto queryRect = RectF::withCenter(player->aimPosition(), Vec2F{2, 2});
                                             auto entities = world->query<ScriptedEntity>(queryRect);
                                             if (entities.empty()) {
                                               message = "Could not find scripted entity at cursor";
                                               return;
                                             }

                                             Ptr<ScriptedEntity> targetEntity;
                                             for (auto const& entity : entities) {
                                               if (!targetEntity
                                                   || vmagSquared(entity->position() - player->aimPosition())
                                                     < vmagSquared(targetEntity->position() - player->aimPosition()))
                                                 targetEntity = entity;
                                             }

                                             if (auto res = targetEntity->evalScript(lua))
                                               message = toString(*res);
                                             else
                                               message = "Error evaluating script in entity context, check log";
                                           });

  return done ? message : "failed to do entity eval";
}

auto CommandProcessor::enableSpawning(ConnectionId connectionId, String const&) -> String {
  if (auto errorMsg = adminCheck(connectionId, "enable world spawning"))
    return *errorMsg;

  bool done = m_universe->executeForClient(
    connectionId, [](WorldServer* world, Ptr<Player> const&) -> void { world->setSpawningEnabled(true); });
  return done ? "enabled monster spawning" : "enabling monster spawning failed";
}

auto CommandProcessor::disableSpawning(ConnectionId connectionId, String const&) -> String {
  if (auto errorMsg = adminCheck(connectionId, "disable world spawning"))
    return *errorMsg;

  bool done = m_universe->executeForClient(
    connectionId, [](WorldServer* world, Ptr<Player> const&) -> void { world->setSpawningEnabled(false); });
  return done ? "disabled monster spawning" : "disabling monster spawning failed";
}

auto CommandProcessor::placeDungeon(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "place dungeons"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);
  String dungeonName = arguments.at(0);

  std::optional<Vec2I> targetPosition;
  if (arguments.size() > 1) {
    auto pos = arguments.at(1).split(",", 1);
    targetPosition = Vec2I(lexicalCast<int>(pos.at(0)), lexicalCast<int>(pos.at(1)));
  }

  bool done = m_universe->executeForClient(connectionId,
                                           [dungeonName, targetPosition](WorldServer* world, Ptr<Player> const& player) -> void {
                                             world->placeDungeon(dungeonName, targetPosition.value_or(Vec2I::floor(player->aimPosition())), true);
                                           });

  return done ? "" : "Unable to place dungeon " + dungeonName;
}

auto CommandProcessor::setUniverseFlag(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "set universe flags"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);
  String flag = arguments.at(0);
  m_universe->universeSettings()->setFlag(flag);

  return "set universe flag " + flag;
}

auto CommandProcessor::resetUniverseFlags(ConnectionId connectionId, String const&) -> String {
  if (auto errorMsg = adminCheck(connectionId, "reset universe flags"))
    return *errorMsg;

  m_universe->universeSettings()->resetFlags();
  return "universe flags reset!";
}

auto CommandProcessor::addBiomeRegion(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "add biome regions"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  String biomeName = arguments.at(0);
  int width = lexicalCast<int>(arguments.at(1));

  String subBlockSelector = "largeClumps";
  if (arguments.size() > 2)
    subBlockSelector = arguments.at(2);

  bool done = m_universe->executeForClient(connectionId,
                                           [biomeName, width, subBlockSelector](WorldServer* world, Ptr<Player> const& player) -> void {
                                             world->addBiomeRegion(Vec2I::floor(player->aimPosition()), biomeName, subBlockSelector, width);
                                           });

  return done ? strf("added region of biome {} with width {}", biomeName, width) : "failed to add biome region";
}

auto CommandProcessor::expandBiomeRegion(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "expand biome regions"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  int newWidth = lexicalCast<int>(arguments.at(0));

  bool done = m_universe->executeForClient(connectionId,
                                           [newWidth](WorldServer* world, Ptr<Player> const& player) -> void {
                                             world->expandBiomeRegion(Vec2I::floor(player->aimPosition()), newWidth);
                                           });

  return done ? strf("expanded region to width {}", newWidth) : "failed to expand biome region";
}

auto CommandProcessor::updatePlanetType(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "update planet type"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  auto coordinate = CelestialCoordinate(arguments.at(0));
  auto newType = arguments.at(1);
  auto weatherBiome = arguments.at(2);

  bool done = m_universe->updatePlanetType(coordinate, newType, weatherBiome);

  return done ? strf("set planet at {} to type {} weatherBiome {}", coordinate, newType, weatherBiome) : "failed to update planet type";
}

auto CommandProcessor::setWeather(ConnectionId connectionId, String const& argumentString) -> String {
  if (auto errorMsg = adminCheck(connectionId, "set weather"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.empty()) {
    StringList list;
    bool done = m_universe->executeForClient(connectionId,
                                             [&list](WorldServer* world, Ptr<Player> const&) -> void { list = world->weatherList(); });
    return done ? strf("weathers: {}", list.join(", ")) : "failed to query weather";
  }

  String weatherName = arguments.at(0);
  bool force = false;
  CelestialCoordinate coordinate;

  if (arguments.size() >= 2) {
    if (arguments.at(1) == "force") {
      force = true;
      if (arguments.size() >= 3)
        coordinate = CelestialCoordinate(arguments.at(2));
    } else {
      coordinate = CelestialCoordinate(arguments.at(1));
    }
  }

  bool done;
  if (coordinate.isNull()) {
    done = m_universe->executeForClient(connectionId,
                                        [weatherName, force](WorldServer* world, Ptr<Player> const&) -> void { world->setWeather(weatherName, force); });
  } else {
    done = m_universe->setWeather(coordinate, weatherName, force);
  }

  return done ? (coordinate.isNull() ? strf("set weather to {}{}", weatherName, force ? " (forced)" : "") : strf("set weather for {} to {}{}", coordinate, weatherName, force ? " (forced)" : "")) : "failed to set weather";
}

auto CommandProcessor::setEnvironmentBiome(ConnectionId connectionId, String const&) -> String {
  if (auto errorMsg = adminCheck(connectionId, "update layer environment biome"))
    return *errorMsg;

  bool done = m_universe->executeForClient(connectionId,
                                           [](WorldServer* world, Ptr<Player> const& player) -> void {
                                             world->setLayerEnvironmentBiome(Vec2I::floor(player->aimPosition()));
                                           });

  return done ? "set environment biome for world layer" : "failed to set environment biome";
}

auto CommandProcessor::playerCidFromCommand(String const& player, UniverseServer* universe) -> std::optional<ConnectionId> {
  char const* const UsernamePrefix = "@";
  char const* const CidPrefix = "$";
  char const* const UUIDPrefix = "$$";

  if (player.beginsWith(UsernamePrefix)) {
    return universe->findNick(player.substr(std::strlen(UsernamePrefix)));
  } else if (player.beginsWith(UUIDPrefix)) {
    try {
      auto uuidString = player.substr(std::strlen(UUIDPrefix));
      return universe->clientForUuid(Uuid(uuidString));
    } catch (UuidException const&) {
      // pass to base case
    }
  } else if (player.beginsWith(CidPrefix)) {
    auto cidString = player.substr(std::strlen(CidPrefix));
    auto cid = maybeLexicalCast<ConnectionId>(cidString).value_or(ServerConnectionId);
    if (universe->isConnectedClient(cid))
      return cid;
  }

  return universe->findNick(player);
}

//wow, wtf. TODO: replace with hashmap
auto CommandProcessor::handleCommand(ConnectionId connectionId, String const& command, String const& argumentString) -> String {
  if (command == "admin") {
    return admin(connectionId, argumentString);
  } else if (command == "timewarp") {
    return timewarp(connectionId, argumentString);
  } else if (command == "timescale") {
    return timescale(connectionId, argumentString);
  } else if (command == "tickrate") {
    return tickrate(connectionId, argumentString);
  } else if (command == "settileprotection") {
    return setTileProtection(connectionId, argumentString);
  } else if (command == "setdungeonid") {
    return setDungeonId(connectionId, argumentString);
  } else if (command == "setspawnpoint") {
    return setPlayerStart(connectionId, argumentString);
  } else if (command == "spawnitem") {
    return spawnItem(connectionId, argumentString);
  } else if (command == "spawntreasure") {
    return spawnTreasure(connectionId, argumentString);
  } else if (command == "spawnmonster") {
    return spawnMonster(connectionId, argumentString);
  } else if (command == "spawnnpc") {
    return spawnNpc(connectionId, argumentString);
  } else if (command == "spawnstagehand") {
    return spawnStagehand(connectionId, argumentString);
  } else if (command == "clearstagehand") {
    return clearStagehand(connectionId, argumentString);
  } else if (command == "spawnvehicle") {
    return spawnVehicle(connectionId, argumentString);
  } else if (command == "spawnliquid") {
    return spawnLiquid(connectionId, argumentString);
  } else if (command == "pvp") {
    return pvp(connectionId, argumentString);
  } else if (command == "serverwhoami") {
    return whoami(connectionId, argumentString);
  } else if (command == "kick") {
    return kick(connectionId, argumentString);
  } else if (command == "ban") {
    return ban(connectionId, argumentString);
  } else if (command == "unbanip") {
    return unbanIp(connectionId, argumentString);
  } else if (command == "unbanuuid") {
    return unbanUuid(connectionId, argumentString);
  } else if (command == "list") {
    return list(connectionId, argumentString);
  } else if (command == "help") {
    return help(connectionId, argumentString);
  } else if (command == "warp") {
    return warp(connectionId, argumentString);
  } else if (command == "warprandom") {
    return warpRandom(connectionId, argumentString);
  } else if (command == "whereami") {
    return clientCoordinate(connectionId, argumentString);
  } else if (command == "whereis") {
    return clientCoordinate(connectionId, argumentString);
  } else if (command == "serverreload") {
    return serverReload(connectionId, argumentString);
  } else if (command == "eval") {
    return eval(connectionId, argumentString);
  } else if (command == "entityeval") {
    return entityEval(connectionId, argumentString);
  } else if (command == "enablespawning") {
    return enableSpawning(connectionId, argumentString);
  } else if (command == "disablespawning") {
    return disableSpawning(connectionId, argumentString);
  } else if (command == "placedungeon") {
    return placeDungeon(connectionId, argumentString);
  } else if (command == "setuniverseflag") {
    return setUniverseFlag(connectionId, argumentString);
  } else if (command == "resetuniverseflags") {
    return resetUniverseFlags(connectionId, argumentString);
  } else if (command == "addbiomeregion") {
    return addBiomeRegion(connectionId, argumentString);
  } else if (command == "expandbiomeregion") {
    return expandBiomeRegion(connectionId, argumentString);
  } else if (command == "updateplanettype") {
    return updatePlanetType(connectionId, argumentString);
  } else if (command == "setweather") {
    return setWeather(connectionId, argumentString);
  } else if (command == "setenvironmentbiome") {
    return setEnvironmentBiome(connectionId, argumentString);
  } else if (auto res = m_scriptComponent.invoke("command", command, connectionId, jsonFromStringList(m_parser.tokenizeToStringList(argumentString)))) {
    return toString(*res);
  } else {
    return strf("No such command {}", command);
  }
}

auto CommandProcessor::adminCheck(ConnectionId connectionId, String const& commandDescription) const -> std::optional<String> {
  if (connectionId == ServerConnectionId)
    return {};

  auto config = Root::singleton().configuration();
  if (!config->get("allowAdminCommands").toBool())
    return {"Admin commands disabled on this server."};
  if (!config->get("allowAdminCommandsFromAnyone").toBool()) {
    if (!m_universe->isAdmin(connectionId))
      return {strf("Insufficient privileges to {}.", commandDescription)};
  }

  return {};
}

auto CommandProcessor::localCheck(ConnectionId connectionId, String const& commandDescription) const -> std::optional<String> {
  if (connectionId == ServerConnectionId)
    return {};

  if (!m_universe->isLocal(connectionId))
    return {strf("The {} command can only be used locally.", commandDescription)};

  return {};
}

auto CommandProcessor::makeCommandCallbacks() -> LuaCallbacks {
  LuaCallbacks callbacks;
  callbacks.registerCallbackWithSignature<std::optional<String>, ConnectionId, String>(
    "adminCheck", [this](auto&& PH1, auto&& PH2) -> auto { return adminCheck(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  return callbacks;
}

}// namespace Star
