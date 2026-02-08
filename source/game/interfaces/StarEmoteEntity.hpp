#pragma once

#include "StarEntity.hpp"
#include "StarHumanoid.hpp"

namespace Star {

class EmoteEntity : public virtual Entity {
public:
  virtual void playEmote(HumanoidEmote emote) = 0;
};

}// namespace Star
