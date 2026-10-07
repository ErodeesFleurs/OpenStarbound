#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarFile.hpp"
#include "StarRandom.hpp"
#include "StarLexicalCast.hpp"
#include "StarLogging.hpp"
#include "StarIdMap.hpp"
#include "StarConfig.hpp"
#include "StarLockFile.hpp"
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarEither.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarRpcPromise.hpp"
#include "StarIODevice.hpp"
#include "StarZSTDCompression.hpp"
#include "StarNetCompatibility.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarCasting.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include "StarPerlin.hpp"
#include "StarWeightedPool.hpp"
#include "StarStrongTypedef.hpp"
#include <functional>
#include "StarRect.hpp"
#include "StarSectorArray2D.hpp"
#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarSet.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarMap.hpp"
#include "StarBlockAllocator.hpp"
#include "StarLruCache.hpp"
#include <atomic>
#include <memory>
#include "StarRefPtr.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarVersion.hpp"
#include "StarString.hpp"
#include "StarVariant.hpp"
#include "StarOrderedMap.hpp"
#include "StarOrderedSet.hpp"
#include "StarVersion.hpp"

#include "StarLuaRoot.hpp"
import star.universe;
import star.worker_pool;
import star.celestial_coordinate;
import star.host_address;
import star.uuid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.tile_modification;
import star.world;
import star.celestial_types;
import star.chat_types;
import star.warping;
import star.wiring;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.cellular_light_array;
import star.cellular_lighting;
import star.cellular_liquid;
import star.world_structure;
#include "StarLuaComponents.hpp"
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.world_render_data;
import star.world_client_state;
import star.interpolation_tracker;
import star.spawn_type_database;
import star.spawner;
import star.world_server;
import star.world_server_thread;
import star.world_template;
import star.system_world_server;
import star.system_world_server_thread;
import star.socket;
import star.tcp;
#include "StarP2PNetworkingService.hpp"
import star.net_packet_socket;
import star.universe_connection;
import star.universe_settings;
import star.universe_server;
import star.asset_source;
import star.assets;
import star.root_base;
import star.root;
import star.root_loader;
import star.configuration;
import star.option_parser;
import star.version_option_parser;

#if defined STAR_SYSTEM_WINDOWS
#include <windows.h>
#endif

import star.signal_handler;
import star.server_query;
import star.server_rcon;

using namespace Star;

Json const AdditionalDefaultConfiguration = Json::parseJson(R"JSON(
    {
      "configurationVersion" : {
        "server" : 4
      },

      "runQueryServer" : false,
      "queryServerPort" : 21025,
      "queryServerBind" : "::",

      "runRconServer" : false,
      "rconServerPort" : 21026,
      "rconServerBind" : "::",
      "rconServerPassword" : "",
      "rconServerTimeout" : 1000,

      "allowAssetsMismatch" : true,
      "serverOverrideAssetsDigest" : null
    }
  )JSON");

int main(int argc, char** argv) {
  try {
    #if defined STAR_SYSTEM_WINDOWS
    unsigned long exceptionStackSize = 131072;
    SetThreadStackGuarantee(&exceptionStackSize);
    #endif
    RootLoader rootLoader({{}, AdditionalDefaultConfiguration, String("starbound_server.log"), LogLevel::Info, false, String("starbound_server.config")});
    rootLoader.setVersionName("Server");
    RootUPtr root = rootLoader.commandInitOrDie(argc, argv).first;
    root->fullyLoad();

    SignalHandler signalHandler;
    signalHandler.setHandleFatal(true);
    signalHandler.setHandleInterrupt(true);

    auto configuration = root->configuration();
    {
      Logger::info("{}", rootLoader.getVersionString());

      float updateRate = 1.0f / GlobalTimestep;
      if (auto jUpdateRate = configuration->get("updateRate")) {
        updateRate = jUpdateRate.toFloat();
        ServerGlobalTimestep = GlobalTimestep = 1.0f / updateRate;
        Logger::info("Configured tick rate is {:4.2f}hz", updateRate);
      }

      UniverseServerUPtr server = make_unique<UniverseServer>(root->toStoragePath("universe"));
      server->setListeningTcp(true);
      server->start();

      ServerQueryThreadUPtr queryServer;
      if (configuration->get("runQueryServer").toBool()) {
        queryServer = make_unique<ServerQueryThread>(server.get(), HostAddressWithPort(configuration->get("queryServerBind").toString(), configuration->get("queryServerPort").toInt()));
        queryServer->start();
      }

      ServerRconThreadUPtr rconServer;
      if (configuration->get("runRconServer").toBool()) {
        rconServer = make_unique<ServerRconThread>(server.get(), HostAddressWithPort(configuration->get("rconServerBind").toString(), configuration->get("rconServerPort").toInt()));
        rconServer->start();
      }

      while (server->isRunning()) {
        if (signalHandler.interruptCaught()) {
          Logger::info("Interrupt caught!");
          server->stop();
          break;
        }
        Thread::sleep(100);
      }

      server->join();

      if (queryServer) {
        queryServer->stop();
        queryServer->join();
      }

      if (rconServer) {
        rconServer->stop();
        rconServer->join();
      }
    }

    Logger::info("Server shutdown gracefully");
  } catch (std::exception const& e) {
    fatalException(e, true);
  }

  return 0;
}
