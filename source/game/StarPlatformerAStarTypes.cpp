#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"
import star.platformer_astar_types;

namespace Star {
namespace PlatformerAStar {

EnumMap<Action> const ActionNames{
  {Action::Walk, "Walk"},
  {Action::Jump, "Jump"},
  {Action::Arc, "Arc"},
  {Action::Drop, "Drop"},
  {Action::Swim, "Swim"},
  {Action::Fly, "Fly"},
  {Action::Land, "Land"}
};

Node Node::withVelocity(Vec2F velocity) const {
  return {position, velocity};
}

}
}
