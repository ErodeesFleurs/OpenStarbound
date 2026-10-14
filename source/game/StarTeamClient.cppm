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
#include "StarStrongTypedef.hpp"
import star.vector;
import star.celestial_coordinate;
import star.rpc_promise;
import star.game_types;
import star.warping;
import star.json_rpc;
import std;
import star.variant;
import star.maybe;
import star.version;

namespace Star {

extern VersionNumber const TeamClientVersion;
  
class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<const Player>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<const Player>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<const Player>;
class ClientContext;
using ClientContextPtr = std::shared_ptr<ClientContext>;
using ClientContextConstPtr = std::shared_ptr<const ClientContext>;
using ClientContextWeakPtr = std::weak_ptr<ClientContext>;
using ClientContextConstWeakPtr = std::weak_ptr<const ClientContext>;
using ClientContextUPtr = std::unique_ptr<ClientContext>;
using ClientContextConstUPtr = std::unique_ptr<const ClientContext>;
class TeamClient;
using TeamClientPtr = std::shared_ptr<TeamClient>;
using TeamClientConstPtr = std::shared_ptr<const TeamClient>;
using TeamClientWeakPtr = std::weak_ptr<TeamClient>;
using TeamClientConstWeakPtr = std::weak_ptr<const TeamClient>;
using TeamClientUPtr = std::unique_ptr<TeamClient>;
using TeamClientConstUPtr = std::unique_ptr<const TeamClient>;

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

  TeamClient(PlayerPtr mainPlayer, ClientContextPtr clientContext);

  void invitePlayer(String const& playerName);
  void acceptInvitation(Uuid const& inviterUuid);

  Maybe<Uuid> currentTeam() const;

  void makeLeader(Uuid const& playerUuid);
  void removeFromTeam(Uuid const& playerUuid);

  bool isTeamLeader();
  bool isTeamLeader(Uuid const& playerUuid);
  bool isMemberOfTeam();

  bool hasInvitationPending();
  std::pair<Uuid, String> pullInvitation();
  List<Variant<std::pair<String, bool>, StringList>> pullInviteResults();

  void update();

  void pullFullUpdate();
  void statusUpdate();

  void forceUpdate();

  List<Member> members();

private:
  typedef std::pair<RpcPromise<Json>, std::function<void(Json const&)>> RpcResponseHandler;

  void invokeRemote(String const& method, Json const& args, std::function<void(Json const&)> responseFunction = {});
  void handleRpcResponses();

  void writePlayerData(JsonObject& request, PlayerPtr player, bool fullWrite = false) const;

  void clearTeam();

  PlayerPtr m_mainPlayer;
  ClientContextPtr m_clientContext;
  Maybe<Uuid> m_teamUuid;

  Uuid m_teamLeader;

  List<Member> m_members;

  bool m_hasPendingInvitation;
  std::pair<Uuid, String> m_pendingInvitation;
  double m_pollInvitationsTimer;
  List<Variant<std::pair<String, bool>, StringList>> m_pendingInviteResults;

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
using ::Star::PlayerPtr;
using ::Star::PlayerConstPtr;
using ::Star::PlayerWeakPtr;
using ::Star::PlayerConstWeakPtr;
using ::Star::PlayerUPtr;
using ::Star::PlayerConstUPtr;
using ::Star::ClientContext;
using ::Star::ClientContextPtr;
using ::Star::ClientContextConstPtr;
using ::Star::ClientContextWeakPtr;
using ::Star::ClientContextConstWeakPtr;
using ::Star::ClientContextUPtr;
using ::Star::ClientContextConstUPtr;
using ::Star::TeamClient;
using ::Star::TeamClientPtr;
using ::Star::TeamClientConstPtr;
using ::Star::TeamClientWeakPtr;
using ::Star::TeamClientConstWeakPtr;
using ::Star::TeamClientUPtr;
using ::Star::TeamClientConstUPtr;
}
