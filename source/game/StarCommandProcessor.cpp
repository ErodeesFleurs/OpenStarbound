import star.json_extra;
import star.json;
import star.xx_hash;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.game_types;
import star.strong_typedef;
import star.list;
import star.rpc_promise;
import star.vector;
import star.rect;
import star.bi_map;
import star.a_star;
import star.data_stream;
import star.string;
import star.byte_array;
import star.encode;
import star.bytes;
import star.format;
import star.optional;
import star.lexical_cast;
import star.version;
import star.weighted_pool;
import star.parametric_function;
import star.logging;
import star.set;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.lua;
import star.periodic_function;
import star.audio;
import star.map;
import star.either;
import star.image;
import star.math_common;
import star.worker_pool;

import std;
import star.sector_array_2d;
import star.net_compatibility;
import star.perlin;
import star.btree;
import star.data_stream_devices;
import star.btree_database;
import star.ordered_set;
import star.lock_file;
import star.io_device;

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.ref_ptr;
import star.listener;

import star.lua_root;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;
import star.drawable;
import star.item;
import star.item_descriptor;
import star.item_recipe;
import star.shell_parser;
import star.lua_components;

import star.command_processor;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.humanoid;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.entity_rendering_types;
import star.lounging_entities;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.npc_database;
import star.effect_emitter;
import star.npc;
import star.tile_sector_array;
import star.liquid_types;
import star.weather_types;
import star.sky_types;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.warping;
import star.wiring;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.cellular_light_array;
import star.cellular_lighting;
import star.cellular_liquid;
import star.world_structure;
import star.sky_render_data;
import star.parallax;
import star.world_render_data;
import star.world_client_state;
import star.interpolation_tracker;
import star.spawn_type_database;
import star.spawner;
import star.world_server;
import star.universe;
import star.host_address;
import star.world_server_thread;
import star.world_template;
import star.system_world_server;
import star.system_world_server_thread;
import star.socket;
import star.tcp;
import star.p2p_networking_service;
import star.net_packet_socket;
import star.universe_connection;
import star.universe_server;
import star.universe_settings;
import star.asset_source;
import star.assets;
import star.root;
import star.configuration;
import star.item_drop;
import star.inventory_types;
import star.ai_types;
import star.entity_rendering;
import star.radio_message_database;
import star.player;
import star.aggressive_entity;
import star.monster_database;
import star.monster;
import star.stagehand;
import star.chat_processor;
import star.vehicle;
import star.treasure;

import star.vehicle_database;
import star.liquids_database;
import star.universe_server_lua_bindings;
import star.celestial_lua_bindings;
import star.stagehand_database;
import star.item_database;
import star.celestial_database;


import star.algorithm;
namespace Star {

CommandProcessor::CommandProcessor(Root& root, UniverseServer* universe)
  : m_root(root), m_universe(universe), m_scriptComponent(root) {
  auto assets = m_root.assets();
  auto universeConfig = assets->json("/universe_server.config");
  
  m_luaRoot = std::make_shared<LuaRoot>(m_root);
  m_luaRoot->tuneAutoGarbageCollection(universeConfig.getFloat("luaGcPause"), universeConfig.getFloat("luaGcStepMultiplier"));
  m_scriptComponent.addCallbacks("universe", LuaBindings::makeUniverseServerCallbacks(m_root, m_universe));
  m_scriptComponent.addCallbacks("celestial", LuaBindings::makeCelestialCallbacks(m_root, m_universe));
  m_scriptComponent.addThreadCallbacks("universe", LuaBindings::makeUniverseServerCallbacks(m_root, m_universe));
  m_scriptComponent.addThreadCallbacks("celestial", LuaBindings::makeCelestialCallbacks(m_root, m_universe));
  m_scriptComponent.addCallbacks("CommandProcessor", makeCommandCallbacks());
  m_scriptComponent.setScripts(jsonToStringList(universeConfig.get("commandProcessorScripts")));
  m_luaRoot->luaEngine().setNullTerminated(false);
  m_scriptComponent.setLuaRoot(m_luaRoot);
  m_scriptComponent.init();
}

ServerCommandResult CommandProcessor::adminCommand(String const& command, String const& argumentString) {
  MutexLocker locker(m_mutex);
  return handleCommand(ServerConnectionId, command, argumentString);
}

ServerCommandResult CommandProcessor::userCommand(ConnectionId connectionId, String const& command, String const& argumentString) {
  MutexLocker locker(m_mutex);
  if (connectionId == ServerConnectionId)
    throw StarException("CommandProcessor::userCommand called with ServerConnectionId");
  return handleCommand(connectionId, command, argumentString);
}

String CommandProcessor::help(ConnectionId connectionId, String const& argumentString) {
  auto arguments = m_parser.tokenizeToStringList(argumentString);

  auto assets = m_root.assets();
  auto basicCommands = assets->json("/help.config:basicCommands");
  auto openSbCommands = assets->json("/help.config:openSbCommands");
  auto adminCommands = assets->json("/help.config:adminCommands");
  auto debugCommands = assets->json("/help.config:debugCommands");
  auto openSbDebugCommands = assets->json("/help.config:openSbDebugCommands");

  if (arguments.size()) {
    if (arguments.size() >= 1) {
      auto helpTexts = std::array{
          basicCommands.optString(arguments[0]), openSbCommands.optString(arguments[0]),
          adminCommands.optString(arguments[0]), debugCommands.optString(arguments[0]),
          openSbDebugCommands.optString(arguments[0])};
      for (auto const& helpText : helpTexts) {
        if (helpText)
          return *helpText;
      }
    }
  }

  String res = "";

  auto commandDescriptions = [&](Json const& commandConfig) {
      StringList commandList = commandConfig.toObject().keys();
      std::sort(commandList.begin(), commandList.end(), std::less<String>());
      return "/" + commandList.join(", /");
    };

  String basicHelpFormat = assets->json("/help.config:basicHelpText").toString();
  res = res + strf(basicHelpFormat.utf8Ptr(), commandDescriptions(basicCommands));

  String openSbHelpFormat = assets->json("/help.config:openSbHelpText").toString();
  res = res + "\n" + strf(openSbHelpFormat.utf8Ptr(), commandDescriptions(openSbCommands));

  if (!adminCheck(connectionId, "")) {
    String adminHelpFormat = assets->json("/help.config:adminHelpText").toString();
    res = res + "\n" + strf(adminHelpFormat.utf8Ptr(), commandDescriptions(adminCommands));

    String debugHelpFormat = assets->json("/help.config:debugHelpText").toString();
    res = res + "\n" + strf(debugHelpFormat.utf8Ptr(), commandDescriptions(debugCommands));

    String openSbDebugHelpFormat = assets->json("/help.config:openSbDebugHelpText").toString();
    res = res + "\n" + strf(openSbDebugHelpFormat.utf8Ptr(), commandDescriptions(openSbDebugCommands));
  }

  res = res + "\n" + basicCommands.getString("help");

  return res;
}

String CommandProcessor::admin(ConnectionId connectionId, String const& argumentString) {
  auto config = m_root.configuration();
  auto arguments = m_parser.tokenizeToStringList(argumentString);

  ConnectionId targetClientId = connectionId;

  if (!arguments.empty()) {
    if (auto errorMsg = adminCheck(connectionId, "make user admin"))
      return *errorMsg;

    auto targetCid = playerCidFromCommand(arguments[0], m_universe);
    if (!targetCid)
      return strf("No user with specifier {} found.", arguments[0]);

    targetClientId = *targetCid;
  } else {
    if (!m_universe->canBecomeAdmin(connectionId) && !m_universe->isAdmin(connectionId))
      return "Insufficient privileges to make self admin.";
  }

  if (targetClientId == ServerConnectionId)
    return "Invalid client state";

  if (!config->get("allowAdminCommands").toBool())
    return "Admin commands disabled on this server.";

  bool wasAdmin = m_universe->isAdmin(targetClientId);
  m_universe->setAdmin(targetClientId, !wasAdmin);

  if (!wasAdmin)
    return strf("Admin privileges now given to {}", m_universe->clientNick(targetClientId));
  else
    return strf("Admin privileges taken away from {}", m_universe->clientNick(targetClientId));
}

String CommandProcessor::serverDebug(ConnectionId connectionId, String const&) {
  if (auto errorMsg = adminCheck(connectionId, "debug server"))
    return *errorMsg;
  if (m_universe->isLocal(connectionId))
    return "Server is local, server debug already active";
  if (m_universe->clientConnectionVersion(connectionId) < 17)
    return "Client is not new enough to debug server";
  
  bool nowEnabled = !m_universe->serverDebug(connectionId);
  m_universe->setServerDebug(connectionId,nowEnabled);
  return strf("Server debug {}", nowEnabled ? "enabled" : "disabled");
}

String CommandProcessor::pvp(ConnectionId connectionId, String const&) {
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

String CommandProcessor::whoami(ConnectionId connectionId, String const&) {
  return strf("Server: You are {}. You are {}an Admin",
      m_universe->clientNick(connectionId),
      m_universe->isAdmin(connectionId) ? "" : "not ");
}

String CommandProcessor::warp(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "do the space warp again"))
    return *errorMsg;

  try {
    m_universe->clientWarpPlayer(connectionId, parseWarpAction(argumentString));
    return "Lets do the space warp again";
  } catch (StarException const& e) {
    m_root.logger().warn("Could not parse warp target: {}", outputException(e, false));
    return strf("Could not parse the argument {} as a warp target", argumentString);
  }
}

String CommandProcessor::warpRandom(ConnectionId connectionId, String const& typeName) {
  if (auto errorMsg = adminCheck(connectionId, "warp to random world"))
    return *errorMsg;

	Vec2I size = {2, 2};
	auto celestialDatabase = m_universe->celestialDatabase();
	std::optional<CelestialCoordinate> target = {};

	auto validPlanet = [celestialDatabase, &typeName](CelestialCoordinate const& p) {
			if (auto celestialParams = celestialDatabase->parameters(p)) {
				if (auto visitableParams = celestialParams->visitableParameters()) {
					if (visitableParams->typeName == typeName)
						return true;
				}
			}
			return false;
		};

	while (!target.has_value()) {
		RectI region = RectI::withSize(Vec2I(m_root.random().randi32(), m_root.random().randi32()), size);

		while (!celestialDatabase->scanRegionFullyLoaded(region)) {
			celestialDatabase->scanSystems(region);
		}
		auto systems = celestialDatabase->scanSystems(region);
		for (auto s : systems) {
			for (auto planet : celestialDatabase->children(s)) {
				if (validPlanet(planet))
					target = planet;
				if (!target.has_value()) {
					for (auto moon : celestialDatabase->children(planet)) {
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

String CommandProcessor::timewarp(ConnectionId connectionId, String const& argumentsString) {
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

String CommandProcessor::timescale(ConnectionId connectionId, String const& argumentsString) {
  if (auto errorMsg = adminCheck(connectionId, "mess with time"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentsString);

  if (arguments.empty())
    return strf("Current timescale is {:6.6f}x", m_root.timing().timescale);

  float timescale = clamp(lexicalCast<float>(arguments[0]), 0.001f, 32.0f);
  m_universe->setTimescale(timescale);
  return strf("Set timescale to {:6.6f}x", timescale);
}

String CommandProcessor::tickrate(ConnectionId connectionId, String const& argumentsString) {
  if (auto errorMsg = adminCheck(connectionId, "change the tick rate"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentsString);

  if (arguments.empty())
    return strf("Current tick rate is {:4.2f}Hz", 1.0f / m_root.timing().serverTimestep);

  float tickRate = clamp(lexicalCast<float>(arguments[0]), 5.f, 500.f);
  m_universe->setTickRate(tickRate);
  return strf("Set tick rate to {:4.2f}Hz", tickRate);
}

String CommandProcessor::setTileProtection(ConnectionId connectionId, String const& argumentString) {
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
        DungeonId current = lexicalCast<DungeonId>(*it);
        dungeonIds.append(current);
        if (it++ != slices.begin() && previous != current) {
          if (current < previous) std::swap(previous, current);
          for (DungeonId id = previous + 1; id != current; ++id)
            dungeonIds.append(id);
        }
        previous = current;
      }
    }
    std::size_t changed = 0;
    if (!m_universe->executeForClient(connectionId, [&](WorldServer* world, std::shared_ptr<Player> const&) {
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

String CommandProcessor::setDungeonId(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "set dungeon id")) {
    return *errorMsg;
  }

  auto arguments = m_parser.tokenizeToStringList(argumentString);
  if (arguments.size() < 1)
    return "Not enough arguments to /setdungeonid. Use /setdungeonid <dungeonId>";

  try {
    DungeonId dungeonId = lexicalCast<DungeonId>(arguments.at(0));

    bool done = m_universe->executeForClient(connectionId, [dungeonId](WorldServer* world, std::shared_ptr<Player> const& player) {
        world->setDungeonId(RectI::withSize(Vec2I(player->aimPosition()), Vec2I(1, 1)), dungeonId);
      });

    return done ? "" : "Failed to set dungeon id.";
  } catch (BadLexicalCast const&) {
    return strf("Could not parse /setdungeonid parameters. Use /setdungeonid <dungeonId>!", argumentString);
  }
}

String CommandProcessor::setPlayerStart(ConnectionId connectionId, String const&) {
  if (auto errorMsg = adminCheck(connectionId, "modify world properties"))
    return *errorMsg;

  m_universe->executeForClient(connectionId, [](WorldServer* world, std::shared_ptr<Player> const& player) {
      world->setPlayerStart(player->position() + player->feetOffset());
    });

  return "";
}

String CommandProcessor::spawnItem(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "spawn items"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.empty())
    return "Not enough arguments to /spawnitem";

  try {
    String kind = arguments.at(0);
    Json parameters = JsonObject();
    std::uint64_t amount = 1;
    std::optional<float> level;
    std::optional<std::uint64_t> seed;

    if (arguments.size() >= 2)
      amount = lexicalCast<std::uint64_t>(arguments.at(1));

    if (arguments.size() >= 3)
      parameters = Json::parse(arguments.at(2));

    if (arguments.size() >= 4)
      level = lexicalCast<float>(arguments.at(3));

    if (arguments.size() >= 5)
      seed = lexicalCast<std::uint64_t>(arguments.at(4));

    bool done = m_universe->executeForClient(connectionId, [&](WorldServer* world, std::shared_ptr<Player> const& player) {
        auto itemDatabase = m_root.itemDatabase();
        world->addEntity(ItemDrop::createRandomizedDrop(m_root, itemDatabase->item(ItemDescriptor(kind, amount, parameters), level, seed, true), player->aimPosition()));
      });

    return done ? "" : "Invalid client state";
  } catch (JsonParsingException const& exception) {
    m_root.logger().warn("Error while processing /spawnitem '{}' command. Json parse problem: {}", arguments.at(0), outputException(exception, false));
    return "Could not parse item parameters";
  } catch (ItemException const& exception) {
    m_root.logger().warn("Error while processing /spawnitem '{}' command. Item instantiation problem: {}", arguments.at(0), outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  } catch (BadLexicalCast const& exception) {
    m_root.logger().warn("Error while processing /spawnitem command. Number expected. Got something else: {}", outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  } catch (StarException const& exception) {
    m_root.logger().warn("Error while processing /spawnitem command '{}', exception caught: {}", argumentString, outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  }
}

String CommandProcessor::spawnTreasure(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "spawn items"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.empty())
    return "Not enough arguments to /spawntreasure";

  try {
    String treasurePool = arguments.at(0);
    float level = 1;

    if (arguments.size() >= 2)
      level = lexicalCast<float>(arguments.at(1));

    bool done = m_universe->executeForClient(connectionId, [&](WorldServer* world, std::shared_ptr<Player> const& player) {
        auto treasureDatabase = m_root.treasureDatabase();
        for (auto const& treasureItem : treasureDatabase->createTreasure(treasurePool, level, m_root.random().randu64()))
          world->addEntity(ItemDrop::createRandomizedDrop(m_root, treasureItem, player->aimPosition()));
      });

    return done ? "" : "Invalid client state";
  } catch (JsonParsingException const& exception) {
    m_root.logger().warn("Error while processing /spawntreasure '{}' command. Json parse problem: {}", arguments.at(0), outputException(exception, false));
    return "Could not parse item parameters";
  } catch (ItemException const& exception) {
    m_root.logger().warn("Error while processing /spawntreasure '{}' command. Item instantiation problem: {}", arguments.at(0), outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  } catch (BadLexicalCast const& exception) {
    m_root.logger().warn("Error while processing /spawntreasure command. Number expected. Got something else: {}", outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  } catch (StarException const& exception) {
    m_root.logger().warn("Error while processing /spawntreasure command '{}', exception caught: {}", argumentString, outputException(exception, false));
    return strf("Could not load item '{}'", arguments.at(0));
  }
}

String CommandProcessor::spawnMonster(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "spawn monsters"))
    return *errorMsg;

  try {
    auto arguments = m_parser.tokenizeToStringList(argumentString);

    auto monsterDatabase = m_root.monsterDatabase();
    std::shared_ptr<Monster> monster;

    float level = 1;
    if (arguments.size() >= 2)
      level = lexicalCast<float>(arguments.at(1));

    Json parameters = JsonObject();
    if (arguments.size() >= 3)
      parameters = parameters.setAll(Json::parse(arguments.at(2)).toObject());

    monster = monsterDatabase->createMonster(monsterDatabase->randomMonster(arguments.at(0), parameters.toObject()), level);
    bool done = m_universe->executeForClient(connectionId,
        [&](WorldServer* world, std::shared_ptr<Player> const& player) {
          monster->setPosition(player->aimPosition());
          world->addEntity(monster);
        });

    return done ? "" : "Invalid client state";
  } catch (StarException const& exception) {
    m_root.logger().warn("Could not spawn Monster of type '{}', exception caught: {}", argumentString, outputException(exception, false));
    return strf("Could not spawn Monster of type '{}'", argumentString);
  }
}

String CommandProcessor::spawnNpc(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "spawn NPCs"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  try {
    auto npcDatabase = m_root.npcDatabase();
    float npcLevel = 1;
    std::uint64_t seed = m_root.random().randu64();
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
    bool done = m_universe->executeForClient(connectionId, [&](WorldServer* world, std::shared_ptr<Player> const& player) {
        npc->setPosition(player->aimPosition());
        world->addEntity(npc);
      });

    return done ? "" : "Invalid client state";
  } catch (StarException const& exception) {
    m_root.logger().warn("Could not spawn NPC of species '{}', exception caught: {}", argumentString, outputException(exception, true));
    return strf("Could not spawn NPC of species '{}'", argumentString);
  }
}

String CommandProcessor::spawnVehicle(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "spawn vehicles"))
    return *errorMsg;

  try {
    auto vehicleDatabase = m_root.vehicleDatabase();
    auto arguments = m_parser.tokenizeToStringList(argumentString);

    std::shared_ptr<Vehicle> vehicle;

    String name = arguments.at(0);

    Json parameters = JsonObject();
    if (arguments.size() >= 2)
      parameters = Json::parse(arguments.at(1)).toObject();

    vehicle = vehicleDatabase->create(name, parameters);
    bool done = m_universe->executeForClient(connectionId,
        [&](WorldServer* world, std::shared_ptr<Player> const& player) {
          vehicle->setPosition(player->aimPosition());
          world->addEntity(std::move(vehicle));
        });

    return done ? "" : "Invalid client state";
  } catch (StarException const& exception) {
    m_root.logger().warn("Could not spawn vehicle, exception caught: {}", outputException(exception, false));
    return strf("Could not spawn vehicle");
  }
}

String CommandProcessor::spawnStagehand(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "spawn stagehands"))
    return *errorMsg;

  try {
    auto arguments = m_parser.tokenizeToStringList(argumentString);

    auto stagehandDatabase = m_root.stagehandDatabase();

    Json parameters = JsonObject();
    if (arguments.size() >= 2)
      parameters = Json::parse(arguments.at(1)).toObject();

    auto stagehand = stagehandDatabase->createStagehand(arguments.at(0), parameters);
    bool done = m_universe->executeForClient(connectionId, [&](WorldServer* world, std::shared_ptr<Player> player) {
        stagehand->setPosition(player->aimPosition());
        world->addEntity(stagehand);
      });

    return done ? "" : "Invalid client state";
  } catch (StarException const& exception) {
    m_root.logger().warn("Could not spawn Stagehand of type '{}', exception caught: {}", argumentString, outputException(exception, false));
    return strf("Could not spawn Stagehand of type '{}'", argumentString);
  }
}

String CommandProcessor::clearStagehand(ConnectionId connectionId, String const&) {
  if (auto errorMsg = adminCheck(connectionId, "remove stagehands"))
    return *errorMsg;

  unsigned removed = 0;
  bool done = m_universe->executeForClient(connectionId,
      [&](WorldServer* world, std::shared_ptr<Player> player) {
        auto queryRect = RectF::withCenter(player->aimPosition(), Vec2F{2, 2});
        for (auto stagehand : world->query<Stagehand>(queryRect)) {
          world->removeEntity(stagehand->entityId(), true);
          ++removed;
        }
      });
  return done ? strf("Removed {} stagehands", removed) : "Invalid client state";
}

String CommandProcessor::spawnLiquid(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "spawn liquid"))
    return *errorMsg;

  try {
    auto arguments = m_parser.tokenizeToStringList(argumentString);

    auto liquidsDatabase = m_root.liquidsDatabase();

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

    bool done = m_universe->executeForClient(connectionId, [&](WorldServer* world, std::shared_ptr<Player> const& player) {
        world->modifyTile(Vec2I(player->aimPosition().floor()), PlaceLiquid{liquid, quantity}, true);
      });
    return done ? "" : "Invalid client state";

  } catch (StarException const& exception) {
    m_root.logger().warn(
        "Could not spawn liquid '{}', exception caught: {}", argumentString, outputException(exception, false));
    return "Could not spawn liquid.";
  }
}

String CommandProcessor::kick(ConnectionId connectionId, String const& argumentString) {
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
      outputOptional(toKick),
      arguments[1]);
}

String CommandProcessor::ban(ConnectionId connectionId, String const& argumentString) {
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
      arguments[0], outputOptional(toKick), reason);
}

String CommandProcessor::unbanIp(ConnectionId connectionId, String const& argumentString) {
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

String CommandProcessor::unbanUuid(ConnectionId connectionId, String const& argumentString) {
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

String CommandProcessor::list(ConnectionId connectionId, String const&) {
  if (auto errorMsg = adminCheck(connectionId, "list clients"))
    return *errorMsg;

  StringList res;

  auto assets = m_root.assets();
  for (auto cid : m_universe->clientIds())
    res.append(strf("${} : {} : $${}", cid, m_universe->clientNick(cid), m_universe->uuidForClient(cid)->hex()));

  return res.join("\n");
}

String CommandProcessor::clientCoordinate(ConnectionId connectionId, String const& argumentString) {
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
    return strf("{} current location is {}", targetLabel, printWorldId(worldId));
  } else {
    return "";
  }
}

String CommandProcessor::serverReload(ConnectionId connectionId, String const&) {
  if (auto errorMsg = adminCheck(connectionId, "trigger root reload"))
    return *errorMsg;

  auto& root = m_root;
  root.reload();
  root.fullyLoad();
  return "";
}

String CommandProcessor::eval(ConnectionId connectionId, String const& lua) {
  if (auto errorMsg = localCheck(connectionId, "execute server script"))
    return *errorMsg;

  if (auto errorMsg = adminCheck(connectionId, "execute server script"))
    return *errorMsg;

  return toString(outputLuaValue(m_scriptComponent.context()->eval(lua)));
}

String CommandProcessor::entityEval(ConnectionId connectionId, String const& lua) {
  if (auto errorMsg = localCheck(connectionId, "execute server entity script"))
    return *errorMsg;

  if (auto errorMsg = adminCheck(connectionId, "execute server entity script"))
    return *errorMsg;

  String message;
  bool done = m_universe->executeForClient(connectionId,
      [&lua, &message](WorldServer* world, std::shared_ptr<Player> const& player) {
        auto queryRect = RectF::withCenter(player->aimPosition(), Vec2F{2, 2});
        auto entities = world->query<ScriptedEntity>(queryRect);
        if (entities.empty()) {
          message = "Could not find scripted entity at cursor";
          return;
        }

        std::shared_ptr<ScriptedEntity> targetEntity;
        for (auto const& entity : entities) {
          if (!targetEntity
              || vmagSquared(entity->position() - player->aimPosition())
                  < vmagSquared(targetEntity->position() - player->aimPosition()))
            targetEntity = entity;
        }

        if (auto res = targetEntity->evalScript(lua))
          message = toString(outputLuaValue(*res));
        else
          message = "Error evaluating script in entity context, check log";
      });

  return done ? message : "failed to do entity eval";
}

String CommandProcessor::enableSpawning(ConnectionId connectionId, String const&) {
  if (auto errorMsg = adminCheck(connectionId, "enable world spawning"))
    return *errorMsg;

  bool done = m_universe->executeForClient(
      connectionId, [](WorldServer* world, std::shared_ptr<Player> const&) { world->setSpawningEnabled(true); });
  return done ? "enabled monster spawning" : "enabling monster spawning failed";
}

String CommandProcessor::disableSpawning(ConnectionId connectionId, String const&) {
  if (auto errorMsg = adminCheck(connectionId, "disable world spawning"))
    return *errorMsg;

  bool done = m_universe->executeForClient(
      connectionId, [](WorldServer* world, std::shared_ptr<Player> const&) { world->setSpawningEnabled(false); });
  return done ? "disabled monster spawning" : "disabling monster spawning failed";
}

String CommandProcessor::placeDungeon(ConnectionId connectionId, String const& argumentString) {
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
      [dungeonName, targetPosition](WorldServer* world, std::shared_ptr<Player> const& player) {
        world->placeDungeon(dungeonName, targetPosition.value_or(Vec2I::floor(player->aimPosition())), true);
      });

  return done ? "" : "Unable to place dungeon " + dungeonName;
}

String CommandProcessor::setUniverseFlag(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "set universe flags"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);
  String flag = arguments.at(0);
  m_universe->universeSettings()->setFlag(flag);

  return "set universe flag " + flag;
}

String CommandProcessor::resetUniverseFlags(ConnectionId connectionId, String const&) {
  if (auto errorMsg = adminCheck(connectionId, "reset universe flags"))
    return *errorMsg;

  m_universe->universeSettings()->resetFlags();
  return "universe flags reset!";
}

String CommandProcessor::addBiomeRegion(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "add biome regions"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  String biomeName = arguments.at(0);
  int width = lexicalCast<int>(arguments.at(1));

  String subBlockSelector = "largeClumps";
  if (arguments.size() > 2)
    subBlockSelector = arguments.at(2);

  bool done = m_universe->executeForClient(connectionId,
      [biomeName, width, subBlockSelector](WorldServer* world, std::shared_ptr<Player> const& player) {
        world->addBiomeRegion(Vec2I::floor(player->aimPosition()), biomeName, subBlockSelector, width);
      });

  return done ? strf("added region of biome {} with width {}", biomeName, width) : "failed to add biome region";
}

String CommandProcessor::expandBiomeRegion(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "expand biome regions"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  int newWidth = lexicalCast<int>(arguments.at(0));

  bool done = m_universe->executeForClient(connectionId,
      [newWidth](WorldServer* world, std::shared_ptr<Player> const& player) {
        world->expandBiomeRegion(Vec2I::floor(player->aimPosition()), newWidth);
      });

  return done ? strf("expanded region to width {}", newWidth) : "failed to expand biome region";
}

String CommandProcessor::updatePlanetType(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "update planet type"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  auto coordinate = CelestialCoordinate(arguments.at(0));
  auto newType = arguments.at(1);
  auto weatherBiome = arguments.at(2);

  bool done = m_universe->updatePlanetType(coordinate, newType, weatherBiome);

  return done ? strf("set planet at {} to type {} weatherBiome {}", coordinate, newType, weatherBiome) : "failed to update planet type";
}

String CommandProcessor::setWeather(ConnectionId connectionId, String const& argumentString) {
  if (auto errorMsg = adminCheck(connectionId, "set weather"))
    return *errorMsg;

  auto arguments = m_parser.tokenizeToStringList(argumentString);

  if (arguments.empty()) {
    StringList list;
    bool done = m_universe->executeForClient(connectionId,
                                             [&list](WorldServer* world, std::shared_ptr<Player> const& player) { list = world->weatherList(player->position()); });
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
                                        [weatherName, force](WorldServer* world, std::shared_ptr<Player> const& player) { world->setWeather(player->position(), weatherName, force); });
  } else {
    done = m_universe->setWeather(coordinate, weatherName, force);
  }

  return done ? (coordinate.isNull() ? strf("set weather to {}{}", weatherName, force ? " (forced)" : "") : strf("set weather for {} to {}{}", coordinate, weatherName, force ? " (forced)" : "")) : "failed to set weather";
}


String CommandProcessor::setEnvironmentBiome(ConnectionId connectionId, String const&) {
  if (auto errorMsg = adminCheck(connectionId, "update layer environment biome"))
    return *errorMsg;

  bool done = m_universe->executeForClient(connectionId,
      [](WorldServer* world, std::shared_ptr<Player> const& player) {
        world->setLayerEnvironmentBiome(Vec2I::floor(player->aimPosition()));
      });

  return done ? "set environment biome for world layer" : "failed to set environment biome";
}

std::optional<ConnectionId> CommandProcessor::playerCidFromCommand(String const& player, UniverseServer* universe) {
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

CaseInsensitiveStringMap<CommandProcessor::Command> CommandProcessor::makeCommandMap() {
  CaseInsensitiveStringMap<Command> map;

  auto add = [&map](char const* cmd, Command func) {
    map[cmd] = func;
  };

  // Register all commands
  add("admin", &CommandProcessor::admin);
  add("serverdebug", &CommandProcessor::serverDebug);
  add("timewarp", &CommandProcessor::timewarp);
  add("timescale", &CommandProcessor::timescale);
  add("tickrate", &CommandProcessor::tickrate);
  add("settileprotection", &CommandProcessor::setTileProtection);
  add("setdungeonid", &CommandProcessor::setDungeonId);
  add("setspawnpoint", &CommandProcessor::setPlayerStart);
  add("spawnitem", &CommandProcessor::spawnItem);
  add("spawntreasure", &CommandProcessor::spawnTreasure);
  add("spawnmonster", &CommandProcessor::spawnMonster);
  add("spawnnpc", &CommandProcessor::spawnNpc);
  add("spawnstagehand", &CommandProcessor::spawnStagehand);
  add("clearstagehand", &CommandProcessor::clearStagehand);
  add("spawnvehicle", &CommandProcessor::spawnVehicle);
  add("spawnliquid", &CommandProcessor::spawnLiquid);
  add("pvp", &CommandProcessor::pvp);
  add("serverwhoami", &CommandProcessor::whoami);
  add("kick", &CommandProcessor::kick);
  add("ban", &CommandProcessor::ban);
  add("unbanip", &CommandProcessor::unbanIp);
  add("unbanuuid", &CommandProcessor::unbanUuid);
  add("list", &CommandProcessor::list);
  add("help", &CommandProcessor::help);
  add("warp", &CommandProcessor::warp);
  add("warprandom", &CommandProcessor::warpRandom);
  add("whereami", &CommandProcessor::clientCoordinate);
  add("whereis", &CommandProcessor::clientCoordinate);
  add("serverreload", &CommandProcessor::serverReload);
  add("eval", &CommandProcessor::eval);
  add("entityeval", &CommandProcessor::entityEval);
  add("enablespawning", &CommandProcessor::enableSpawning);
  add("disablespawning", &CommandProcessor::disableSpawning);
  add("placedungeon", &CommandProcessor::placeDungeon);
  add("setuniverseflag", &CommandProcessor::setUniverseFlag);
  add("resetuniverseflags", &CommandProcessor::resetUniverseFlags);
  add("addbiomeregion", &CommandProcessor::addBiomeRegion);
  add("expandbiomeregion", &CommandProcessor::expandBiomeRegion);
  add("updateplanettype", &CommandProcessor::updatePlanetType);
  add("setweather", &CommandProcessor::setWeather);
  add("setenvironmentbiome", &CommandProcessor::setEnvironmentBiome);

  return map;
}

const CaseInsensitiveStringMap<CommandProcessor::Command> CommandProcessor::s_commandMap = makeCommandMap();

ServerCommandResult CommandProcessor::handleCommand(ConnectionId connectionId, String const& command, String const& argumentString) {
  auto it = s_commandMap.find(command);
  if (it != s_commandMap.end()) {
    return (this->*it->second)(connectionId, argumentString);
  }
  if (auto res = m_scriptComponent.invoke<std::variant<RpcPromise<Json>,LuaValue>>("command", command, connectionId, jsonFromStringList(m_parser.tokenizeToStringList(argumentString)))) {
    if (std::holds_alternative<RpcPromise<Json>>(*res)) {
      return std::get<RpcPromise<Json>>(*res).wrap([](Json res) -> String {
        if (res.isType(Json::Type::String))
          return *res.stringPtr();
        else if (!res.isNull())
          return res.repr(1, true);
        else
          return "";
      });
    } else {
      return String(toString(outputLuaValue(std::get<LuaValue>(*res))));
    }
  }
  return String(strf("No such command {}", command));
}

std::optional<String> CommandProcessor::adminCheck(ConnectionId connectionId, String const& commandDescription) const {
  if (connectionId == ServerConnectionId)
    return {};

  auto config = m_root.configuration();
  if (!config->get("allowAdminCommands").toBool())
    return {"Admin commands disabled on this server."};
  if (!config->get("allowAdminCommandsFromAnyone").toBool()) {
    if (!m_universe->isAdmin(connectionId))
      return {strf("Insufficient privileges to {}.", commandDescription)};
  }

  return {};
}

std::optional<String> CommandProcessor::localCheck(ConnectionId connectionId, String const& commandDescription) const {
  if (connectionId == ServerConnectionId)
    return {};

  if (!m_universe->isLocal(connectionId))
    return {strf("The {} command can only be used locally.", commandDescription)};

  return {};
}

LuaCallbacks CommandProcessor::makeCommandCallbacks() {
  LuaCallbacks callbacks;
  callbacks.registerCallbackWithSignature<std::optional<String>, ConnectionId, String>(
      "adminCheck", std::bind(&CommandProcessor::adminCheck, this, std::placeholders::_1, std::placeholders::_2));
  return callbacks;
}

}
