import star.json;
import star.xx_hash;
import star.id_map;
import star.data_stream;
import star.bi_map;
#include "StarCasting.hpp"
import star.variant;
import star.rpc_promise;
#include "StarStrongTypedef.hpp"
import star.game_types;
import star.maybe;
import star.vector;
import star.a_star;
import star.ordered_set;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.color;
import star.string;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.either;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import star.version;
import star.weighted_pool;
import star.byte_array;
#include "StarDataStreamDevices.hpp"

import std;
typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
#include "StarBTree.hpp"
import star.lru_cache;
import star.btree_database;
import star.image;
import star.lock_file;

import star.uuid;
import star.item_descriptor;
import star.drawable;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.humanoid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
#include "StarLuaRoot.hpp"
import star.force_regions;
import star.world;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inspectable_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.game_timers;
import star.actor_movement_controller;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
#include "StarLuaComponents.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.host_address;
import star.sky_types;
import star.weather_types;
import star.world_parameters;
import star.celestial_parameters;
import star.chat_types;
import star.warping;
import star.socket;
import star.tcp;
import star.p2p_networking_service;
import star.liquid_types;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.wiring;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.universe_connection;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;
import star.cellular_liquid;
import star.spawn_type_database;
import star.spawner;
import star.world_server;
import star.world_server_thread;
import star.world_template;
import star.system_world_server;
import star.system_world_server_thread;
import star.universe_settings;
import star.universe_server;

namespace Star {

class TestUniverse {
public:
  TestUniverse(Vec2U clientWindowSize);
  ~TestUniverse();

  void warpPlayer(WorldId worldId);
  WorldId currentPlayerWorld() const;

  void update(unsigned times = 1);

  List<Drawable> currentClientDrawables();

private:
  Vec2U m_clientWindowSize;
  String m_storagePath;
  std::shared_ptr<UniverseServer> m_server;
  std::shared_ptr<UniverseClient> m_client;
  std::shared_ptr<Player> m_mainPlayer;
};

}
