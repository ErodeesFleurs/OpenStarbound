#pragma once

#include "StarLua.hpp"

namespace Star {

class TeamClient;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeTeamClientCallbacks(TeamClient& teamClient);
}
}
