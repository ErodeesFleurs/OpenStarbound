module;
import star.xx_hash;
import star.json;
#include "StarJsonExtra.hpp"
import star.lua;
import star.string;
import star.either;
import star.data_stream;
#include "StarStrongTypedef.hpp"
import star.ordered_set;
import star.io_device;
import star.thread;
import star.byte_array;
#include "StarDataStreamDevices.hpp"

import std;
typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
import std;
import star.id_map;
import star.list;
import star.version;
import star.ordered_map;
import star.rect;
#include "StarCasting.hpp"
import star.set;
import star.vector;
import star.worker_pool;

import star.sector_array_2d;
import star.bi_map;
import star.random;
import star.perlin;
#include "StarBTree.hpp"
import star.lru_cache;
import star.btree_database;
import star.rpc_promise;
import star.image;
import star.audio;
import star.map;
import star.variant;
import star.game_types;
import star.color;
import star.maybe;
import star.weighted_pool;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
#include "StarLuaGameConverters.hpp"
import star.host_address;
import star.chat_types;
import star.uuid;
import star.warping;
import star.item_descriptor;
import star.quest_descriptor;
import star.ai_types;
import star.socket;
import star.tcp;
import star.p2p_networking_service;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.world_geometry;
import star.wiring;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
#include "StarLuaRoot.hpp"
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.universe_connection;
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.chat_action;
import star.mixer;
import star.entity_rendering;
import star.world;
#include "StarLuaComponents.hpp"
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.game_timers;
import star.world_client;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;
import star.system_world_client;
import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_graphics;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

module star.celestial_lua_bindings;
import star.biome_database;
import star.celestial_database;
import star.sky;


namespace Star {
LuaCallbacks LuaBindings::makeCelestialCallbacks(CelestialDatabasePtr celestialDatabase) {
  LuaCallbacks callbacks;
  
  callbacks.registerCallback("planetParameters", [celestialDatabase](Json const& coords) -> Json {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      if (auto celestialParameters = celestialDatabase->parameters(coordinate))
        return celestialParameters->parameters();
      return Json();
    });
  callbacks.registerCallback("visitableParameters", [celestialDatabase](Json const& coords) -> Json {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      if (auto parameters = celestialDatabase->parameters(coordinate)) {
        if (auto visitableParameters = parameters->visitableParameters())
          return visitableParameters->store();
      }
      return {};
    });
  callbacks.registerCallback("planetName", [celestialDatabase](Json const& coords) -> Maybe<String> {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      if (auto parameters = celestialDatabase->parameters(coordinate))
        return parameters->name();
      return {};
    });
  callbacks.registerCallback("planetSeed", [celestialDatabase](Json const& coords) -> Maybe<std::uint64_t> {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      if (auto parameters = celestialDatabase->parameters(coordinate))
        return parameters->seed();
      return {};
    });
  callbacks.registerCallback("planetOres", [celestialDatabase](Json const& coords, float threatLevel) -> List<String> {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      auto parameters = celestialDatabase->parameters(coordinate);
      if (!parameters)
        return {};

      auto visitableParameters = parameters->visitableParameters();
      if (!visitableParameters)
        return {};

      auto biomeDatabase = Root::singleton().biomeDatabase();
      auto addOres = [biomeDatabase, threatLevel](StringSet& oreList, String const& biomeName) {
          oreList.addAll(biomeDatabase->biomeOres(biomeName, threatLevel));
        };

      StringSet planetOres;
      if (auto const& terrestrialParameters = as<TerrestrialWorldParameters>(visitableParameters)) {
        addOres(planetOres, terrestrialParameters->primaryBiome);
        addOres(planetOres, terrestrialParameters->surfaceLayer.primaryRegion.biome);
        addOres(planetOres, terrestrialParameters->subsurfaceLayer.primaryRegion.biome);
        for (auto layer : terrestrialParameters->undergroundLayers)
          addOres(planetOres, layer.primaryRegion.biome);
        addOres(planetOres, terrestrialParameters->coreLayer.primaryRegion.biome);
      } else if (auto const& asteroidParameters = as<AsteroidsWorldParameters>(visitableParameters)) {
        addOres(planetOres, asteroidParameters->asteroidBiome);
      }

      return planetOres.values();
    });

  callbacks.registerCallback("hasChildren", [celestialDatabase](Json const& coords) -> Maybe<bool> {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      return celestialDatabase->hasChildren(coordinate);
    });
  callbacks.registerCallback("children", [celestialDatabase](Json const& coords) -> List<Json> {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      return celestialDatabase->children(coordinate).transformed([](CelestialCoordinate const& c) { return c.toJson(); });
    });
  callbacks.registerCallback("childOrbits", [celestialDatabase](Json const& coords) -> List<int> {
      return celestialDatabase->childOrbits(CelestialCoordinate(coords));
    });
  callbacks.registerCallback("scanSystems", [celestialDatabase](RectI const& region, Maybe<StringSet> const& includedTypes) -> List<Json> {
      return celestialDatabase->scanSystems(region, includedTypes).transformed([](CelestialCoordinate const& c) { return c.toJson(); });
    });
  callbacks.registerCallback("scanConstellationLines", [celestialDatabase](RectI const& region) -> List<std::pair<Vec2I, Vec2I>> {
      return celestialDatabase->scanConstellationLines(region);
    });
  callbacks.registerCallback("scanRegionFullyLoaded", [celestialDatabase](RectI const& region) -> bool {
      return celestialDatabase->scanRegionFullyLoaded(region);
    });

  callbacks.registerCallback("centralBodyImages", [celestialDatabase](Json const& coords) -> List<std::pair<String, float>> {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      return CelestialGraphics::drawSystemCentralBody(celestialDatabase, coordinate);
    });
  callbacks.registerCallback("planetaryObjectImages", [celestialDatabase](Json const& coords) -> List<std::pair<String, float>> {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      return CelestialGraphics::drawSystemPlanetaryObject(celestialDatabase, coordinate);
    });
  callbacks.registerCallback("worldImages", [celestialDatabase](Json const& coords) -> List<std::pair<String, float>> {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      return CelestialGraphics::drawWorld(celestialDatabase, coordinate);
    });
  callbacks.registerCallback("starImages", [celestialDatabase](Json const& coords, float twinkleTime) -> List<std::pair<String, float>> {
      CelestialCoordinate coordinate = CelestialCoordinate(coords);
      return CelestialGraphics::drawSystemTwinkle(celestialDatabase, coordinate, twinkleTime);
    });
  
  return callbacks;
}

LuaCallbacks LuaBindings::makeCelestialCallbacks(Universe* universe) {
  auto celestialDatabase = universe->celestialDatabase();
  
  LuaCallbacks callbacks = makeCelestialCallbacks(celestialDatabase);
  
  if (auto client = as<UniverseClient>(universe)) {
    auto systemWorld = client->systemWorldClient();

    callbacks.registerCallback("skyFlying", [client]() {
        return client->currentSky()->flying();
      });
    callbacks.registerCallback("skyFlyingType", [client]() -> String {
        return FlyingTypeNames.getRight(client->currentSky()->flyingType());
      });
    callbacks.registerCallback("skyWarpPhase", [client]() -> String {
        return WarpPhaseNames.getRight(client->currentSky()->warpPhase());
      });
    callbacks.registerCallback("skyWarpProgress", [client]() -> float {
        return client->currentSky()->warpProgress();
      });
    callbacks.registerCallback("skyInHyperspace", [client]() -> bool {
        return client->currentSky()->inHyperspace();
      });

    callbacks.registerCallback("flyShip", [client,systemWorld](Vec3I const& system, Json const& destination, Json const& settings) {
        auto location = jsonToSystemLocation(destination);
        client->flyShip(system, location, settings);
      });
    callbacks.registerCallback("flying", [systemWorld]() {
        return systemWorld->flying();
      });
    callbacks.registerCallback("shipSystemPosition", [systemWorld]() -> Maybe<Vec2F> {
        return systemWorld->shipPosition();
      });
    callbacks.registerCallback("shipDestination", [systemWorld]() -> Json {
        return jsonFromSystemLocation(systemWorld->shipDestination());
      });
    callbacks.registerCallback("shipLocation", [systemWorld]() -> Json {
        return jsonFromSystemLocation(systemWorld->shipLocation());
      });
    callbacks.registerCallback("currentSystem", [systemWorld]() -> Json {
        return systemWorld->currentSystem().toJson();
      });

    callbacks.registerCallback("planetSize", [systemWorld](Json const& coords) -> float {
        return systemWorld->planetSize(CelestialCoordinate(coords));
      });
    callbacks.registerCallback("planetPosition", [systemWorld](Json const& coords) -> Vec2F {
        return systemWorld->planetPosition(CelestialCoordinate(coords));
      });
    callbacks.registerCallback("clusterSize", [systemWorld](Json const& coords) -> float {
        return systemWorld->clusterSize(CelestialCoordinate(coords));
      });

    callbacks.registerCallback("systemPosition", [systemWorld](Json const& l) -> Maybe<Vec2F> {
        auto location = jsonToSystemLocation(l);
        return systemWorld->systemLocationPosition(location);
      });
    callbacks.registerCallback("orbitPosition", [systemWorld](Json const& orbit) -> Vec2F {
        return systemWorld->orbitPosition(CelestialOrbit::fromJson(orbit));
      });

    callbacks.registerCallback("systemObjects", [systemWorld]() -> List<String> {
        return systemWorld->objects().transformed([](SystemObjectPtr object) { return object->uuid().hex(); });
      });
    callbacks.registerCallback("objectType", [systemWorld](String const& uuid) -> Maybe<String> {
        if (auto object = systemWorld->getObject(Uuid(uuid)))
          return object->name();
        else
          return {};
      });
    callbacks.registerCallback("objectParameters", [systemWorld](String const& uuid) -> Json {
        if (auto object = systemWorld->getObject(Uuid(uuid)))
          return object->parameters();
        else
          return {};
      });
    callbacks.registerCallback("objectWarpActionWorld", [systemWorld](String const& uuid) -> Maybe<String> {
        if (Maybe<WarpAction> action = systemWorld->objectWarpAction((Uuid(uuid)))) {
          return action.get().maybe<WarpToWorld>().apply([](WarpToWorld const& warp) { return printWorldId(warp.world); });
        } else {
          return {};
        }
      });
    callbacks.registerCallback("objectOrbit", [systemWorld](String const& uuid) -> Json {
        if (auto object = systemWorld->getObject(Uuid(uuid)))
          return jsonFromMaybe<CelestialOrbit>(object->orbit(), [](CelestialOrbit const& orbit) { return orbit.toJson(); });
        else
          return {};
      });
    callbacks.registerCallback("objectPosition", [systemWorld](String const& uuid) -> Maybe<Vec2F> {
        if (auto object = systemWorld->getObject(Uuid(uuid)))
          return object->position();
        else
          return {};
      });
    callbacks.registerCallback("objectTypeConfig", [](String const& typeName) -> Json {
        return SystemWorld::systemObjectTypeConfig(typeName);
      });
    callbacks.registerCallback("systemSpawnObject", [systemWorld](String const& typeName, Maybe<Vec2F> const& position, Maybe<String> uuidHex, Maybe<JsonObject> parameters) -> String {
        Maybe<Uuid> uuid = uuidHex.apply([](auto const& u) { return Uuid(u); });
        return systemWorld->spawnObject(typeName, position, uuid, parameters.value({})).hex();
      });

    callbacks.registerCallback("playerShips", [systemWorld]() -> List<String> {
        return systemWorld->ships().transformed([](SystemClientShipPtr ship) { return ship->uuid().hex(); });
      });
    callbacks.registerCallback("playerShipPosition", [systemWorld](String const& uuid) -> Maybe<Vec2F> {
        if (auto ship = systemWorld->getShip(Uuid(uuid)))
          return ship->position();
        else
          return {};
      });
  }

  return callbacks;
}

}
