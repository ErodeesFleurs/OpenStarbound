#pragma once

#include "StarLua.hpp"

namespace Star {

class TeamClient;

namespace LuaBindings {
  LuaCallbacks makeTeamClientCallbacks(TeamClient* teamClient);
}
}
