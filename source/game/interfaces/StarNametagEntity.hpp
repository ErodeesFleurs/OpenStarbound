#pragma once

#include "StarEntity.hpp"

namespace Star {

class NametagEntity;

class NametagEntity : public virtual Entity {
public:
  [[nodiscard]] virtual String nametag() const = 0;
  [[nodiscard]] virtual Maybe<String> statusText() const = 0;
  [[nodiscard]] virtual bool displayNametag() const = 0;
  [[nodiscard]] virtual Vec3B nametagColor() const = 0;
  [[nodiscard]] virtual Vec2F nametagOrigin() const = 0;
};

}
