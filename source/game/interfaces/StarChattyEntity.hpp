#pragma once

#include "StarChatAction.hpp"
#include "StarEntity.hpp"

namespace Star {

class ChattyEntity : public virtual Entity {
public:
  [[nodiscard]] virtual auto mouthPosition() const -> Vec2F = 0;
  [[nodiscard]] virtual auto mouthPosition(bool) const -> Vec2F = 0;
  virtual auto pullPendingChatActions() -> List<ChatAction> = 0;
};

}// namespace Star
