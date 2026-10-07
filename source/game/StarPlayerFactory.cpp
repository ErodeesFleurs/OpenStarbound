#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarIdMap.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarNetElementSystem.hpp"
#include "StarCasting.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarStrongTypedef.hpp"
#include "StarGameTypes.hpp"
#include "StarMaybe.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarOrderedSet.hpp"
#include "StarPoly.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarEither.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarListener.hpp"
#include "StarVersion.hpp"
#include "StarThread.hpp"

#include "StarLuaRoot.hpp"
import star.animation;
import star.particle;
import star.uuid;
import star.item_descriptor;
import star.drawable;
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
#include "StarUtilityLuaBindings.hpp"
import star.rebuilder;
import star.player_factory;

import star.entity_splash;

namespace Star {

PlayerConfig::PlayerConfig(JsonObject const& cfg) {
  defaultIdentity = HumanoidIdentity(cfg.value("defaultHumanoidIdentity"));
  humanoidTiming = Humanoid::HumanoidTiming(cfg.value("humanoidTiming"));

  for (Json v : cfg.value("defaultItems", JsonArray()).toArray())
    defaultItems.append(ItemDescriptor(v));

  for (Json v : cfg.value("defaultBlueprints", JsonObject()).getArray("tier1", JsonArray()))
    defaultBlueprints.append(ItemDescriptor(v));

  metaBoundBox = jsonToRectF(cfg.get("metaBoundBox"));

  movementParameters = cfg.get("movementParameters");
  zeroGMovementParameters = cfg.get("zeroGMovementParameters");
  statusControllerSettings = cfg.get("statusControllerSettings");

  footstepTiming = cfg.get("footstepTiming").toFloat();
  footstepSensor = jsonToVec2F(cfg.get("footstepSensor"));

  underwaterSensor = jsonToVec2F(cfg.get("underwaterSensor"));
  underwaterMinWaterLevel = cfg.get("underwaterMinWaterLevel").toFloat();

  splashConfig = EntitySplashConfig(cfg.get("splashConfig"));

  companionsConfig = cfg.get("companionsConfig");

  deploymentConfig = cfg.get("deploymentConfig");

  effectsAnimator = cfg.get("effectsAnimator").toString();

  teleportInTime = cfg.get("teleportInTime").toFloat();
  teleportOutTime = cfg.get("teleportOutTime").toFloat();

  deployInTime = cfg.get("deployInTime").toFloat();
  deployOutTime = cfg.get("deployOutTime").toFloat();

  bodyMaterialKind = cfg.get("bodyMaterialKind").toString();

  for (auto& p : cfg.get("genericScriptContexts").optObject().value(JsonObject()))
    genericScriptContexts[p.first] = p.second.toString();
}

PlayerFactory::PlayerFactory() : m_rebuilder(make_shared<Rebuilder>("player")) {
  auto assets = Root::singleton().assets();
  m_config = make_shared<PlayerConfig>(assets->json("/player.config").toObject());
}

PlayerPtr PlayerFactory::create() const {
  return make_shared<Player>(m_config);
}

PlayerPtr PlayerFactory::diskLoadPlayer(Json const& diskStore) const {
  PlayerPtr player;
  try {
    player = make_shared<Player>(m_config, diskStore);
  } catch (std::exception const& e) {
    auto exception = std::current_exception();
    bool success = m_rebuilder->rebuild(diskStore, strf("{}", outputException(e, false)), [&](Json const& store) -> String {
      try {
        player = make_shared<Player>(m_config, store);
      } catch (std::exception const& e) {
        exception = std::current_exception();
        return strf("{}", outputException(e, false));
      }
      return {};
    });

    if (!success)
      std::rethrow_exception(exception);
  }
  return player;
}

PlayerPtr PlayerFactory::netLoadPlayer(ByteArray const& netStore, NetCompatibilityRules rules) const {
  return make_shared<Player>(m_config, netStore, rules);
}

}
