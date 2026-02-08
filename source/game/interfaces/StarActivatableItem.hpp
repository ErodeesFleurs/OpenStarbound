#pragma once

namespace Star {

class ActivatableItem {
public:
  virtual ~ActivatableItem() = default;
  [[nodiscard]] virtual auto active() const -> bool = 0;
  virtual void setActive(bool active) = 0;
  [[nodiscard]] virtual auto usable() const -> bool = 0;
  virtual void activate() = 0;
};

}// namespace Star
