module;
import star.json;

import star.string;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.drawable;
import star.uuid;
import star.json_rpc;
import star.strong_typedef;
import star.vector;
import star.celestial_coordinate;
import star.game_types;
import star.warping;
import star.damage_types;
import std;
import star.map;
import star.maybe;
import star.version;

namespace Star {
  
extern VersionNumber const TeamManagerVersion;
class Root;
class RandomService;

class TeamManager {
public:
  explicit TeamManager(Root& root);

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
    explicit Team(RandomService& random) : leaderUuid(random) {}

    Uuid leaderUuid;
    TeamNumber pvpTeamNumber;

    Map<Uuid, TeamMember> members;
  };

  struct Invitation {
    explicit Invitation(RandomService& random) : inviterUuid(random) {}

    Uuid inviterUuid;
    String inviterName;
  };

  struct PolledInvitation {
    explicit PolledInvitation(RandomService& random) : inviterUuid(random) {}
    PolledInvitation(Uuid uuid, double time) : inviterUuid(std::move(uuid)), polledAt(time) {}

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

  Root& m_root;
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
}
