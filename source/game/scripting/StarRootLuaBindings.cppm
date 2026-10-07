module;
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarNetElementSystem.hpp"
#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarLua.hpp"

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
import star.scripted_entity;
import star.status_effect_entity;
import star.movement_controller;
import star.animation;
import star.particle;
#include "StarLuaComponents.hpp"
import star.effect_emitter;
import star.projectile;

export module star.root_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeRootCallbacks();
}

