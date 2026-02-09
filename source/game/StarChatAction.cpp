#include "StarChatAction.hpp"

import std;

namespace Star {

SayChatAction::SayChatAction() {
  entity = NullEntityId;
}

SayChatAction::SayChatAction(EntityId entity, String const& text, Vec2F const& position)
    : entity(entity), text(std::move(text)), position(position) {}

SayChatAction::SayChatAction(EntityId entity, String const& text, Vec2F const& position, Json const& config)
    : entity(entity), text(std::move(text)), position(position), config(std::move(config)) {}

SayChatAction::operator bool() const {
  return !text.empty();
}

PortraitChatAction::PortraitChatAction() {
  entity = NullEntityId;
}

PortraitChatAction::PortraitChatAction(
  EntityId entity, String const& portrait, String const& text, Vec2F const& position)
    : entity(entity), portrait(std::move(portrait)), text(std::move(text)), position(position) {}

PortraitChatAction::PortraitChatAction(
  EntityId entity, String const& portrait, String const& text, Vec2F const& position, Json const& config)
    : entity(entity), portrait(std::move(portrait)), text(std::move(text)), position(position), config(std::move(config)) {}
PortraitChatAction::operator bool() const {
  return !text.empty();
}

}// namespace Star
