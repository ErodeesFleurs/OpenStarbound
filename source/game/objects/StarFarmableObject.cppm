module;
#include "StarJson.hpp"
import star.game_types;
import star.id_map;
import star.random;
import star.periodic;
import star.periodic_function;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarSet.hpp"
import star.color;
#include "StarLua.hpp"
import star.vector;
#include "StarDataStream.hpp"
import star.audio;
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarString.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarOrderedMap.hpp"


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
