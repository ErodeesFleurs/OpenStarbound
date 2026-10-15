module;
import std;
import star.json;
import star.id_map;
#include "StarCasting.hpp"
import star.data_stream;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.bi_map;

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

class EmoteEntity : public virtual Entity {
public:
  virtual void playEmote(HumanoidEmote emote) = 0;
};

}

export module star.emote_entity;

export namespace Star {
  using ::Star::EmoteEntity;
}
