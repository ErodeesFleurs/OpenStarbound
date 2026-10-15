module;
import std;
import star.list;
import star.json;
import star.id_map;
#include "StarCasting.hpp"
import star.vector;
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
import star.chat_action;

namespace Star {

class ChattyEntity : public virtual Entity {
public:
  virtual Vec2F mouthPosition() const = 0;
  virtual Vec2F mouthPosition(bool) const = 0;
  virtual List<ChatAction> pullPendingChatActions() = 0;
};

}

export module star.chatty_entity;

export namespace Star {
  using ::Star::ChattyEntity;
}
