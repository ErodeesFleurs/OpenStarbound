module;

#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
import star.drawable;
#include "StarUuid.hpp"
#include "StarJsonRpc.hpp"
#include "StarStrongTypedef.hpp"
#include "StarUuid.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
#include "StarGameTypes.hpp"
import star.warping;
#include "StarThread.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
import star.damage_types;
#include "StarVersion.hpp"

namespace Star {
  
extern VersionNumber const TeamManagerVersion;

STAR_CLASS(TeamManager);

class TeamManager {
public:
  TeamManager();

  JsonRpcHandlers rpcHandlers();

  JsonRpcHandlers authenticatedRpcHandlers(Uuid const& callerUuid);

  void setConnectedPlayers(StringMap<List<Uuid>> connectedPlayers);
  void playerDisconnected(Uuid const& playerUuid);

  TeamNumber getPvpTeam(Uuid const& playerUuid);
  HashMap<Uuid, TeamNumber> getPvpTeams();
  Maybe<Uuid> getTeam(Uuid const& playerUuid) const;

private:
  struct TeamMember {
    String name;
    int entity;
    float healthPercentage;
    float energyPercentage;
    WorldId world;
    Vec2F position;
    WarpMode warpMode;
    
    int version = 0;

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

  bool playerWithUuidExists(Uuid const& playerUuid) const;

  Uuid createTeam(Uuid const& leaderUuid);
  bool addToTeam(Uuid const& playerUuid, Uuid const& teamUuid);
  bool removeFromTeam(Uuid const& playerUuid, Uuid const& teamUuid);

  RecursiveMutex m_mutex;
  Map<Uuid, Team> m_teams;
  StringMap<List<Uuid>> m_connectedPlayers;
  Map<Uuid, Invitation> m_invitations;
  Map<Uuid, PolledInvitation> m_polledInvitations;

  unsigned m_maxTeamSize;
  double m_polledInvitationTimeout;
  bool m_secureTeams;

  TeamNumber m_pvpTeamCounter;

  Json fetchTeamStatus(Json const& args);
  Json updateStatus(Json const& args);
  Json invite(Json const& args);
  Json pollInvitation(Json const& args);
  Json acceptInvitation(Json const& args);
  Json removeFromTeam(Json const& args);
  Json makeLeader(Json const& args);
};

}

export module star.team_manager;

export namespace Star {
using ::Star::TeamManagerVersion;
using ::Star::TeamManager;
using ::Star::TeamManagerPtr;
using ::Star::TeamManagerConstPtr;
using ::Star::TeamManagerWeakPtr;
using ::Star::TeamManagerConstWeakPtr;
using ::Star::TeamManagerUPtr;
using ::Star::TeamManagerConstUPtr;
}
