#pragma once

#include "StarChatAction.hpp"
#include "StarEntity.hpp"

namespace Star {

class ChattyEntity;

class ChattyEntity : public virtual Entity {
public:
  [[nodiscard]] virtual Vec2F mouthPosition() const = 0;
  [[nodiscard]] virtual Vec2F mouthPosition(bool) const = 0;
  [[nodiscard]] virtual List<ChatAction> pullPendingChatActions() = 0;
};

}
