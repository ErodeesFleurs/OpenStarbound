#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarConfig.hpp"
#include "StarMaybe.hpp"
#include "StarString.hpp"
import star.lock_file;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarString.hpp"
#include "StarEither.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarRpcPromise.hpp"
#include "StarIODevice.hpp"
#include "StarByteArray.hpp"
#include "StarDataStreamDevices.hpp"

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
#include "StarNetCompatibility.hpp"
#include "StarSet.hpp"
#include "StarBTree.hpp"
#include "StarOrderedMap.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
import star.btree_database;
#include "StarCasting.hpp"
#include "StarOrderedSet.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
import star.version;
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.perlin;
import star.weighted_pool;
#include "StarStrongTypedef.hpp"
#include <functional>
#include "StarRect.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarMaybe.hpp"
#include "StarColor.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarVariant.hpp"
#include "StarSet.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"
#include "StarOrderedMap.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarMap.hpp"
#include "StarRandom.hpp"
#include "StarBlockAllocator.hpp"
#include <atomic>
#include <memory>
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;

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
import star.configuration;
import star.root;

#include "gtest/gtest.h"

using namespace Star;

TEST(ServerTest, Run) {
  UniverseServer server(Root::singleton().toStoragePath("universe"));
  server.start();
  server.stop();
  server.join();
}
