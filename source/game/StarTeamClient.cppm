module;
import star.json;

import star.data_stream;
import star.uuid;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.drawable;
import star.strong_typedef;
import star.vector;
import star.celestial_coordinate;
import star.rpc_promise;
import star.game_types;
import star.warping;
import star.json_rpc;
import std;
import star.version;

namespace Star {

extern VersionNumber const TeamClientVersion;
  
class Root;
class Player;
class ClientContext;

class TeamClient {
public:
  struct Member {
    String name;
    Uuid uuid;
    int entity;
    float healthPercentage;
    float energyPercentage;
    WorldId world;
    Vec2F position;
    WarpMode warpMode;
    List<Drawable> portrait;
  };

  TeamClient(Root& root, std::shared_ptr<Player> mainPlayer, std::shared_ptr<ClientContext> clientContext);

  void invitePlayer(String const& playerName);
  void acceptInvitation(Uuid const& inviterUuid);

  std::optional<Uuid> currentTeam() const;

  void makeLeader(Uuid const& playerUuid);
  void removeFromTeam(Uuid const& playerUuid);

  bool isTeamLeader();
  bool isTeamLeader(Uuid const& playerUuid);
  bool isMemberOfTeam();

  bool hasInvitationPending();
  std::pair<Uuid, String> pullInvitation();
  List<std::variant<std::pair<String, bool>, StringList>> pullInviteResults();

  void update();

  void pullFullUpdate();
  void statusUpdate();

  void forceUpdate();

  List<Member> members();

private:
  typedef std::pair<RpcPromise<Json>, std::function<void(Json const&)>> RpcResponseHandler;

  void invokeRemote(String const& method, Json const& args, std::function<void(Json const&)> responseFunction = {});
  void handleRpcResponses();

  void writePlayerData(JsonObject& request, std::shared_ptr<Player> player, bool fullWrite = false) const;

  void clearTeam();

  Root& m_root;
  std::shared_ptr<Player> m_mainPlayer;
  std::shared_ptr<ClientContext> m_clientContext;
  std::optional<Uuid> m_teamUuid;

  Uuid m_teamLeader;

  List<Member> m_members;

  bool m_hasPendingInvitation;
  std::pair<Uuid, String> m_pendingInvitation;
  double m_pollInvitationsTimer;
  List<std::variant<std::pair<String, bool>, StringList>> m_pendingInviteResults;

  bool m_fullUpdateRunning;
  double m_fullUpdateTimer;

  bool m_statusUpdateRunning;
  double m_statusUpdateTimer;

  List<RpcResponseHandler> m_pendingResponses;
  
  int m_teamManagerVersion = 0;
};

}

export module star.team_client;

export namespace Star {
using ::Star::TeamClientVersion;
using ::Star::Player;
using ::Star::ClientContext;
using ::Star::TeamClient;
}
