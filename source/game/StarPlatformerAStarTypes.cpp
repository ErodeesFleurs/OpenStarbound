#include "StarPlatformerAStarTypes.hpp"

namespace Star::PlatformerAStar {

EnumMap<Action> const ActionNames{
  {Action::Walk, "Walk"},
  {Action::Jump, "Jump"},
  {Action::Arc, "Arc"},
  {Action::Drop, "Drop"},
  {Action::Swim, "Swim"},
  {Action::Fly, "Fly"},
  {Action::Land, "Land"}};

auto Node::withVelocity(Vec2F velocity) const -> Node {
  return {.position = position, .velocity = velocity};
}

}// namespace Star::PlatformerAStar
