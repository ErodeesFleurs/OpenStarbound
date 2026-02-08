#pragma once

#include "StarEntity.hpp"

namespace Star {

class AggressiveEntity : public virtual Entity {
public:
  [[nodiscard]] virtual auto aggressive() const -> bool = 0;
};

}// namespace Star
