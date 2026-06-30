#pragma once

#include "StarDrawable.hpp"
#include "StarUuid.hpp"
#include "StarJsonRpc.hpp"
#include "StarWarping.hpp"
#include "StarThread.hpp"
#include "StarDamageTypes.hpp"
#include "StarConfiguration.hpp"

namespace Star {

class TeamManager;
using TeamManagerPtr = SharedPtr<TeamManager>;

class TeamManager {
public:
  TeamManager(ConfigurationPtr configuration);

  [[nodiscard]] JsonRpcHandlers rpcHandlers();

  [[nodiscard]] JsonRpcHandlers authenticatedRpcHandlers(Uuid const& callerUuid);

  void setConnectedPlayers(StringMap<List<Uuid>> connectedPlayers);
  void playerDisconnected(Uuid const& playerUuid);

  [[nodiscard]] TeamNumber getPvpTeam(Uuid const& playerUuid);
  [[nodiscard]] HashMap<Uuid, TeamNumber> getPvpTeams();
  [[nodiscard]] Maybe<Uuid> getTeam(Uuid const& playerUuid) const;

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

  struct PolledInvitation {
    Uuid inviterUuid;
    double polledAt;
  };

  void purgeInvitationsFor(Uuid const& playerUuid);
  void purgeInvitationsFrom(Uuid const& playerUuid);
  void expirePolledInvitations();

  [[nodiscard]] bool playerWithUuidExists(Uuid const& playerUuid) const;

  [[nodiscard]] Uuid createTeam(Uuid const& leaderUuid);
  [[nodiscard]] bool addToTeam(Uuid const& playerUuid, Uuid const& teamUuid);
  [[nodiscard]] bool removeFromTeam(Uuid const& playerUuid, Uuid const& teamUuid);

  RecursiveMutex m_mutex;
  Map<Uuid, Team> m_teams;
  StringMap<List<Uuid>> m_connectedPlayers;
  Map<Uuid, Invitation> m_invitations;
  Map<Uuid, PolledInvitation> m_polledInvitations;

  unsigned m_maxTeamSize;
  double m_polledInvitationTimeout;
  bool m_secureTeams;

  TeamNumber m_pvpTeamCounter;

  [[nodiscard]] Json fetchTeamStatus(Json const& args);
  [[nodiscard]] Json updateStatus(Json const& args);
  [[nodiscard]] Json invite(Json const& args);
  [[nodiscard]] Json pollInvitation(Json const& args);
  [[nodiscard]] Json acceptInvitation(Json const& args);
  [[nodiscard]] Json removeFromTeam(Json const& args);
  [[nodiscard]] Json makeLeader(Json const& args);
};

}
