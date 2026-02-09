#pragma once

#include "StarDamageTypes.hpp"
#include "StarDrawable.hpp"
#include "StarJsonRpc.hpp"
#include "StarThread.hpp"
#include "StarUuid.hpp"
#include "StarWarping.hpp"

import std;

namespace Star {

class TeamManager {
public:
  TeamManager();

  auto rpcHandlers() -> JsonRpcHandlers;

  void setConnectedPlayers(StringMap<List<Uuid>> connectedPlayers);
  void playerDisconnected(Uuid const& playerUuid);

  auto getPvpTeam(Uuid const& playerUuid) -> TeamNumber;
  auto getPvpTeams() -> HashMap<Uuid, TeamNumber>;
  [[nodiscard]] auto getTeam(Uuid const& playerUuid) const -> std::optional<Uuid>;

private:
  struct TeamMember {
    String name;
    int entity;
    float healthPercentage;
    float energyPercentage;
    WorldId world;
    Vec2F position;
    WarpMode warpMode;

    List<Drawable> portrait;
  };

  struct Team {
    Uuid leaderUuid;
    TeamNumber pvpTeamNumber;

    Map<Uuid, TeamMember> members;
  };

  struct Invitation {
    Uuid inviterUuid;
    String inviterName;
  };

  void purgeInvitationsFor(Uuid const& playerUuid);
  void purgeInvitationsFrom(Uuid const& playerUuid);

  [[nodiscard]] auto playerWithUuidExists(Uuid const& playerUuid) const -> bool;

  auto createTeam(Uuid const& leaderUuid) -> Uuid;
  auto addToTeam(Uuid const& playerUuid, Uuid const& teamUuid) -> bool;
  auto removeFromTeam(Uuid const& playerUuid, Uuid const& teamUuid) -> bool;

  RecursiveMutex m_mutex;
  Map<Uuid, Team> m_teams;
  StringMap<List<Uuid>> m_connectedPlayers;
  Map<Uuid, Invitation> m_invitations;

  unsigned m_maxTeamSize;

  TeamNumber m_pvpTeamCounter;

  auto fetchTeamStatus(Json const& args) -> Json;
  auto updateStatus(Json const& args) -> Json;
  auto invite(Json const& args) -> Json;
  auto pollInvitation(Json const& args) -> Json;
  auto acceptInvitation(Json const& args) -> Json;
  auto removeFromTeam(Json const& args) -> Json;
  auto makeLeader(Json const& args) -> Json;
};

}// namespace Star
