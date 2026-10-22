module;
import std;
import star.format;
import star.lua;

namespace Star {
class LuaCallbacks;
class TeamClient;
}

export module star.team_client_lua_bindings;

export namespace Star::LuaBindings {
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeTeamClientCallbacks(TeamClient* teamClient);
}
