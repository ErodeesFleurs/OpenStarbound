#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarFile.hpp"
import star.random;
import star.lexical_cast;
#include "StarLogging.hpp"
import star.id_map;
#include "StarConfig.hpp"
#include "StarMaybe.hpp"
#include "StarString.hpp"
import star.lock_file;
import star.game_types;
import star.either;
#include "StarDataStream.hpp"
import star.rpc_promise;
#include "StarIODevice.hpp"
import star.byte_array;
#include "StarDataStreamDevices.hpp"

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
#include "StarSet.hpp"
#include "StarBTree.hpp"
#include "StarOrderedMap.hpp"
import star.lru_cache;
#include "StarThread.hpp"
import star.btree_database;
#include "StarCasting.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
import star.perlin;
import star.weighted_pool;
#include "StarStrongTypedef.hpp"
#include <functional>
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
import star.color;
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
import star.image;
#include "StarMap.hpp"
#include <atomic>
#include <memory>
import star.ref_ptr;
import star.listener;
import star.version;
#include "StarVariant.hpp"
#include "StarOrderedSet.hpp"

#include "StarLuaRoot.hpp"
import star.universe;
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
import star.p2p_networking_service;
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
