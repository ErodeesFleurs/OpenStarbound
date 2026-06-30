#pragma once

#include "StarJson.hpp"
#include "StarGameTypes.hpp"

namespace Star {

struct SayChatAction {
  SayChatAction() = default;
  SayChatAction(EntityId entity, String const& text, Vec2F const& position);
  SayChatAction(EntityId entity, String const& text, Vec2F const& position, Json const& config);

  explicit operator bool() const;

  EntityId entity = NullEntityId;
  String text;
  Vec2F position;
  Json config;
};

struct PortraitChatAction {
  PortraitChatAction() = default;
  PortraitChatAction(EntityId entity, String const& portrait, String const& text, Vec2F const& position);
  PortraitChatAction(EntityId entity, String const& portrait, String const& text, Vec2F const& position, Json const& config);

  explicit operator bool() const;

  EntityId entity = NullEntityId;
  String portrait;
  String text;
  Vec2F position;
  Json config;
};

using ChatAction = MVariant<SayChatAction, PortraitChatAction>;

}
