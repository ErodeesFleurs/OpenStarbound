#pragma once

namespace Star {

class DurabilityItem {
public:
  virtual ~DurabilityItem() = default;
  virtual auto durabilityStatus() -> float = 0;
};

}// namespace Star
