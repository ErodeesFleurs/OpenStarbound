#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarIdMap.hpp"
#include "StarPeriodic.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarNetElementSystem.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarSet.hpp"
#include "StarColor.hpp"
#include "StarLua.hpp"
#include "StarVector.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarMap.hpp"
#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarAssetPath.hpp"
#include "StarDirectives.hpp"
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"
#include "StarLexicalCast.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarListener.hpp"
#include "StarVersion.hpp"
#include "StarRandom.hpp"
#include "StarMultiArray.hpp"
#include "StarVariant.hpp"
#include "StarEither.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"
#include "StarMathCommon.hpp"
#include "StarArray.hpp"
#include "StarRpcPromise.hpp"
#include "StarSectorArray2D.hpp"
#include <functional>
#include "StarNetCompatibility.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarOrderedSet.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarBlockAllocator.hpp"
#include "StarGameTypes.hpp"
#include "StarWeightedPool.hpp"
#include "StarParametricFunction.hpp"
#include "StarLogging.hpp"

import star.collision_block;
import star.item_descriptor;
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.damage_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.object;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.plant;
#include "StarLuaRoot.hpp"
import star.worker_pool;
import star.tile_sector_array;
import star.tile_modification;
import star.force_regions;
import star.world;
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
import star.physics_entity;
import star.movement_controller;
import star.mobile_entity;
import star.game_timers;
import star.item_drop;
import star.treasure;

import star.farmable_object;
import star.plant_database;
import star.object_database;
import star.material_database;


namespace Star {

FarmableObject::FarmableObject(ObjectConfigConstPtr config, Json const& parameters) : Object(config, parameters) {
  m_stages = configValue("stages", JsonArray({JsonObject()})).toArray();
  m_stage = configValue("startingStage", 0).toInt();

  m_stageAlt = -1;
  m_stageEnterTime = 0.0;
  m_nextStageTime = 0.0;
  m_finalStage = false;

  auto assets = Root::singleton().assets();
  m_minImmersion = configValue("minImmersion", 0).toFloat();
  m_maxImmersion = configValue("maxImmersion", 2).toFloat();
  m_immersion = SlidingWindow(assets->json("/farming.config:immersionWindow").toFloat(),
      assets->json("/farming.config:immersionResolution").toUInt(), (m_minImmersion + m_maxImmersion) / 2);

  m_consumeSoilMoisture = configValue("consumeSoilMoisture", true).toBool();
}

void FarmableObject::update(float dt, uint64_t currentStep) {
  Object::update(dt, currentStep);

  if (isMaster()) {
    if (m_nextStageTime == 0) {
      m_nextStageTime = world()->epochTime();
      enterStage(m_stage);
    }


    while (!m_finalStage && world()->epochTime() >= m_nextStageTime) {
      int lastStage = m_stage;
      enterStage(m_stage + 1);
      if (m_stage == lastStage)
        break;
    }

    // update immersion and check whether farmable should break
    m_immersion.update(bind(&Object::liquidFillLevel, this));
    if (m_immersion.average() > m_maxImmersion || m_immersion.average() < m_minImmersion)
      breakObject(false);
  }
}

bool FarmableObject::damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) {
  if ((tileDamage.type != TileDamageType::Beamish && tileDamage.type != TileDamageType::Blockish && tileDamage.type != TileDamageType::Plantish) || !harvest())
    return Object::damageTiles(position, sourcePosition, tileDamage);

  return false;
}

InteractAction FarmableObject::interact(InteractRequest const&) {
  harvest();
  return {};
}

bool FarmableObject::harvest() {
  if (isMaster() && m_stages.get(m_stage).contains("harvestPool")) {
    try {
      for (auto const& treasureItem : Root::singleton().treasureDatabase()->createTreasure(m_stages.get(m_stage).getString("harvestPool"), world()->threatLevel()))
        world()->addEntity(ItemDrop::createRandomizedDrop(treasureItem, position()));
    } catch (StarException const& e) {
      Logger::warn("Failed to create treasure for farmable object '{}': {}", name(), outputException(e, false));
    }

    if (m_stages.get(m_stage).contains("resetToStage")) {
      m_nextStageTime = world()->epochTime();
      enterStage(m_stages.get(m_stage).getInt("resetToStage"));
    } else
      breakObject(true);

    return true;
  }
  return false;
}

int FarmableObject::stage() const {
  return m_stage;
}

void FarmableObject::enterStage(int newStage) {
  newStage = clamp<int>(newStage, 0, m_stages.size() - 1);

  // attempt to consume water from the soil if needed
  if (m_consumeSoilMoisture && newStage > m_stage) {
    if (auto orientation = currentOrientation()) {
      auto assets = Root::singleton().assets();
      auto materialDatabase = Root::singleton().materialDatabase();
      auto wetToDryMods = assets->json("/farming.config:wetToDryMods");

      // try to transform all anchor spaces, back out and reset stage time if
      // they're not wet
      for (auto anchor : orientation->anchors) {
        auto pos = tilePosition() + anchor.position;
        if (auto newMod = wetToDryMods.optString(materialDatabase->modName(world()->mod(pos, anchor.layer)))) {
          world()->modifyTile(pos, PlaceMod{anchor.layer, materialDatabase->modId(*newMod), MaterialHue()}, true);
        } else {
          Vec2F durationRange = jsonToVec2F(m_stages.get(m_stage).get("duration", JsonArray({0, 0})));
          m_nextStageTime = world()->epochTime() + Random::randf(durationRange[0], durationRange[1]);

          return;
        }
      }
    }
  }

  // TODO: remove this hacky tree stuff and make plants handle it
  if (m_stages.get(newStage).getBool("tree", false)) {
    String stemName = configValue("stemName").toString();
    float stemHueShift = configValue("stemHueShift", 0).toFloat();
    String foliageName = configValue("foliageName", "").toString();
    float foliageHueShift = configValue("foliageHueShift", 0).toFloat();
    Vec2I position = tilePosition();

    TreeVariant tv;
    auto plantDatabase = Root::singleton().plantDatabase();
    if (!foliageName.empty())
      tv = plantDatabase->buildTreeVariant(stemName, stemHueShift, foliageName, foliageHueShift);
    else
      tv = plantDatabase->buildTreeVariant(stemName, stemHueShift);

    auto plant = plantDatabase->createPlant(tv, Random::randi64());
    plant->setTilePosition(position);

    if (anySpacesOccupied(plant->spaces()) || !allSpacesOccupied(plant->roots())) {
      newStage = 0;
    } else {
      world()->timer(2.f / 60.f, [plant](World* world) {
        world->addEntity(plant);
      });

      m_finalStage = true;
      breakObject(true);
      return;
    }
  }

  if (newStage == (int)m_stages.size() - 1) {
    m_finalStage = true;
  } else {
    m_finalStage = false;
    m_stageEnterTime = m_nextStageTime;
    Vec2F durationRange = jsonToVec2F(m_stages.get(newStage).get("duration", JsonArray({0, 0})));
    m_nextStageTime += Random::randf(durationRange[0], durationRange[1]);
  }

  m_interactive.set(m_stages.get(newStage).contains("harvestPool"));

  // keep the same variant if stages have same number of alts
  if (m_stageAlt == -1 || m_stages.get(newStage).getInt("alts", 1) != m_stages.get(m_stage).getInt("alts", 1))
    m_stageAlt = Random::randInt(m_stages.get(newStage).getInt("alts", 1) - 1);

  m_stage = newStage;

  setImageKey("stage", toString(m_stage));
  setImageKey("alt", toString(m_stageAlt));
}

void FarmableObject::readStoredData(Json const& diskStore) {
  Object::readStoredData(diskStore);

  m_stage = diskStore.getInt("stage");
  m_stageAlt = diskStore.getInt("stageAlt");
  m_stageEnterTime = diskStore.getDouble("stageEnterTime");
  m_nextStageTime = diskStore.getDouble("nextStageTime");

  m_finalStage = (m_stage == (int)m_stages.size() - 1);
  setImageKey("stage", toString(m_stage));
  setImageKey("alt", toString(m_stageAlt));
}

Json FarmableObject::writeStoredData() const {
  return Object::writeStoredData().setAll({
      {"stage", m_stage},
      {"stageAlt", m_stageAlt},
      {"stageEnterTime", m_stageEnterTime},
      {"nextStageTime", m_nextStageTime}
    });
}

}
