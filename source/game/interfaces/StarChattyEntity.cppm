module;
#include "StarIdMap.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"


import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.chat_action;

namespace Star {

STAR_CLASS(ChattyEntity);

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
  using ::Star::ChattyEntityPtr;
  using ::Star::ChattyEntityConstPtr;
  using ::Star::ChattyEntityWeakPtr;
  using ::Star::ChattyEntityConstWeakPtr;
  using ::Star::ChattyEntityUPtr;
  using ::Star::ChattyEntityConstUPtr;
}
