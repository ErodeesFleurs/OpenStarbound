import star.json;
#include "StarJsonExtra.hpp"
import star.id_map;
import star.variant;
#include "StarCasting.hpp"
import star.data_stream;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.list;
import star.rpc_promise;
import star.maybe;
import star.string;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.lua;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.audio;
import star.map;
import star.set;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
#include "StarConfig.hpp"
import star.format;
import star.version;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
#include "StarLuaRoot.hpp"
import star.force_regions;
import star.world;
import star.physics_entity;
import star.mobile_entity;
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.networked_animator;
import star.movement_controller;
#include "StarLuaComponents.hpp"
import star.anchorable_entity;
import star.entity_rendering_types;
import star.lounging_entities;
import star.scripted_entity;
#include "StarLuaAnimationComponent.hpp"
import star.vehicle;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
#include "StarUtilityLuaBindings.hpp"
import star.rebuilder;

import star.vehicle_database;

#include "StarDataStreamDevices.hpp"

import star.net_compatibility;
import star.byte_array;
namespace Star {

VehicleDatabase::VehicleDatabase() : m_rebuilder(make_shared<Rebuilder>("vehicle")) {
  auto assets = Root::singleton().assets();
  auto& files = assets->scanExtension("vehicle");
  assets->queueJsons(files);
  for (String file : files) {
    try {
      auto config = assets->json(file);
      String name = config.getString("name");

      if (m_vehicles.contains(name))
        throw VehicleDatabaseException::format("Repeat vehicle name '{}'", name);

      m_vehicles.add(std::move(name), make_pair(std::move(file), std::move(config)));
    } catch (StarException const& e) {
      throw VehicleDatabaseException(strf("Error loading vehicle '{}'", file), e);
    }
  }
}

VehiclePtr VehicleDatabase::create(String const& vehicleName, Json const& extraConfig) const {
  auto configPair = m_vehicles.ptr(vehicleName);
  if (!configPair)
    throw VehicleDatabaseException::format("No such vehicle named '{}'", vehicleName);
  return make_shared<Vehicle>(configPair->second, configPair->first, extraConfig);
}

ByteArray VehicleDatabase::netStore(VehiclePtr const& vehicle, NetCompatibilityRules rules) const {
  DataStreamBuffer ds;
  ds.setStreamCompatibilityVersion(rules);

  ds.write(vehicle->baseConfig().getString("name"));
  ds.write(vehicle->dynamicConfig());
  return ds.takeData();
}

VehiclePtr VehicleDatabase::netLoad(ByteArray const& netStore, NetCompatibilityRules rules) const {
  DataStreamBuffer ds(netStore);
  ds.setStreamCompatibilityVersion(rules);

  String name = ds.read<String>();
  auto dynamicConfig = ds.read<Json>();
  auto vehicle = create(name, dynamicConfig);

  return vehicle;
}

Json VehicleDatabase::diskStore(VehiclePtr const& vehicle) const {
  return JsonObject{
    {"name", vehicle->baseConfig().getString("name")},
    {"dynamicConfig", vehicle->dynamicConfig()},
    {"state", vehicle->diskStore()}
  };
}

VehiclePtr VehicleDatabase::diskLoad(Json const& diskStore) const {
  VehiclePtr vehicle;
  try {
    vehicle = create(diskStore.getString("name"), diskStore.get("dynamicConfig"));
    vehicle->diskLoad(diskStore.get("state"));
    return vehicle;
  } catch (std::exception const& e) {
    vehicle.reset();
    auto exception = std::current_exception();
    bool success = m_rebuilder->rebuild(diskStore, strf("{}", outputException(e, false)), [&](Json const& store) -> String {
      try {
        vehicle = create(store.getString("name"), store.get("dynamicConfig"));
        vehicle->diskLoad(store.get("state"));
      } catch (std::exception const& e) {
        exception = std::current_exception();
        return strf("{}", outputException(e, false));
      }
      return {};
    });

    if (!success)
      std::rethrow_exception(exception);
  }
  return vehicle;
}

}
