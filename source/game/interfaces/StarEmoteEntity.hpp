#pragma once

#include "StarHumanoid.hpp"
#include "StarEntity.hpp"

namespace Star {

class EmoteEntity;

class EmoteEntity : public virtual Entity {
public:
  virtual void playEmote(HumanoidEmote emote) = 0;
};

}
