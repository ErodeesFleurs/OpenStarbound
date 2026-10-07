module;
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
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
#include "StarDataStream.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarDirectives.hpp"
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"


#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
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

import star.game_timers;

namespace Star {

class FarmableObject : public Object {
public:
  FarmableObject(ObjectConfigConstPtr config, Json const& parameters);

  void update(float dt, uint64_t currentStep) override;

  bool damageTiles(List<Vec2I> const& position, Vec2F const& sourcePosition, TileDamage const& tileDamage) override;
  InteractAction interact(InteractRequest const& request) override;

  bool harvest();
  int stage() const;

protected:
  void readStoredData(Json const& diskStore) override;
  Json writeStoredData() const override;

private:
  void enterStage(int newStage);

  int m_stage;
  int m_stageAlt;
  double m_stageEnterTime;
  double m_nextStageTime;

  SlidingWindow m_immersion;
  float m_minImmersion;
  float m_maxImmersion;

  bool m_consumeSoilMoisture;

  JsonArray m_stages;
  bool m_finalStage;
};

}

export module star.farmable_object;

export namespace Star {
  using ::Star::FarmableObject;
}
