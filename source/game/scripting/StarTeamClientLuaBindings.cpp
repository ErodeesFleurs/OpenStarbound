module;
import std;
import star.string;
import star.json;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.data_stream;
import star.lua;
import star.uuid;

module star.team_client_lua_bindings;
import star.team_client;

namespace Star {

LuaCallbacks LuaBindings::makeTeamClientCallbacks(TeamClient* teamClient) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<void>("isMemberOfTeam", std::bind(std::mem_fn(&TeamClient::isMemberOfTeam), teamClient));
  callbacks.registerCallbackWithSignature<void, String>("invitePlayer", std::bind(std::mem_fn(&TeamClient::invitePlayer), teamClient, std::placeholders::_1));

  callbacks.registerCallback("isTeamLeader", [teamClient](std::optional<String>  const& playerUuid) -> bool {
      if (playerUuid)
        return teamClient->isTeamLeader(Uuid(*playerUuid));
      return teamClient->isTeamLeader();
    });
  callbacks.registerCallback("currentTeam", [teamClient]() -> std::optional<String> {
      auto teamUuid = teamClient->currentTeam();
      if (teamUuid)
        return teamUuid->hex();
      return {};
    });
  callbacks.registerCallback("makeLeader", [teamClient](String const& playerUuid) {
      teamClient->makeLeader(Uuid(playerUuid));
    });
  callbacks.registerCallback("removeFromTeam", [teamClient](String const& playerUuid) {
      teamClient->removeFromTeam(Uuid(playerUuid));
    });

  return callbacks;
}

}
