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

class EmoteEntity;
using EmoteEntityPtr = std::shared_ptr<EmoteEntity>;
using EmoteEntityConstPtr = std::shared_ptr<const EmoteEntity>;
using EmoteEntityWeakPtr = std::weak_ptr<EmoteEntity>;
using EmoteEntityConstWeakPtr = std::weak_ptr<const EmoteEntity>;
using EmoteEntityUPtr = std::unique_ptr<EmoteEntity>;
using EmoteEntityConstUPtr = std::unique_ptr<const EmoteEntity>;

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
