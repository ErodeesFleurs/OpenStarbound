module;

namespace Star {
class LuaCallbacks;
class TeamClient;
}

export module star.team_client_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeTeamClientCallbacks(TeamClient* teamClient);
}
