#pragma once

#include "StarEntity.hpp"

import std;

namespace Star {

class NametagEntity : public virtual Entity {
public:
  [[nodiscard]] virtual auto nametag() const -> String = 0;
  [[nodiscard]] virtual auto statusText() const -> std::optional<String> = 0;
  [[nodiscard]] virtual auto displayNametag() const -> bool = 0;
  [[nodiscard]] virtual auto nametagColor() const -> Vec3B = 0;
  [[nodiscard]] virtual auto nametagOrigin() const -> Vec2F = 0;
};

}// namespace Star
