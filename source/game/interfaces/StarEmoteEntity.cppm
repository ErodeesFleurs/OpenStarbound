module;
#include "StarJson.hpp"
import star.id_map;
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
import star.game_types;
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

namespace Star {
enum class HumanoidEmote;
}

namespace Star {

STAR_CLASS(EmoteEntity);

class EmoteEntity : public virtual Entity {
public:
  virtual void playEmote(HumanoidEmote emote) = 0;
};

}

export module star.emote_entity;

export namespace Star {
  using ::Star::EmoteEntity;
  using ::Star::EmoteEntityPtr;
  using ::Star::EmoteEntityConstPtr;
  using ::Star::EmoteEntityWeakPtr;
  using ::Star::EmoteEntityConstWeakPtr;
  using ::Star::EmoteEntityUPtr;
  using ::Star::EmoteEntityConstUPtr;
}
