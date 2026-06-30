#pragma once

#include "StarUuid.hpp"
#include "StarDrawable.hpp"
#include "StarAssets.hpp"
#include "StarWarping.hpp"
#include "StarJsonRpc.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;
class ClientContext;
using ClientContextPtr = SharedPtr<ClientContext>;
class TeamClient;
using TeamClientPtr = SharedPtr<TeamClient>;

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

  TeamClient(AssetsConstPtr assets, PlayerPtr mainPlayer, ClientContextPtr clientContext);

  void invitePlayer(String const& playerName);
  void acceptInvitation(Uuid const& inviterUuid);

  [[nodiscard]] Maybe<Uuid> currentTeam() const;

  void makeLeader(Uuid const& playerUuid);
  void removeFromTeam(Uuid const& playerUuid);

  [[nodiscard]] bool isTeamLeader() const;
  [[nodiscard]] bool isTeamLeader(Uuid const& playerUuid) const;
  [[nodiscard]] bool isMemberOfTeam() const;

  [[nodiscard]] bool hasInvitationPending() const;
  [[nodiscard]] pair<Uuid, String> pullInvitation();
  [[nodiscard]] List<Variant<pair<String, bool>, StringList>> pullInviteResults();

  void update();

  void pullFullUpdate();
  void statusUpdate();

  void forceUpdate();

  [[nodiscard]] List<Member> members() const;

private:
  struct PendingInvitation {
    Uuid inviterUuid;
    String inviterName;
  };
  struct RpcResponseHandler {
    RpcPromise<Json> responsePromise;
    function<void(Json const&)> responseFunction;
  };

  void invokeRemote(String const& method, Json const& args, function<void(Json const&)> responseFunction = {});
  void handleRpcResponses();

  void writePlayerData(JsonObject& request, PlayerPtr player, bool fullWrite = false) const;

  void clearTeam();

  PlayerPtr m_mainPlayer;
  ClientContextPtr m_clientContext;
  AssetsConstPtr m_assets;
  Maybe<Uuid> m_teamUuid;

  Uuid m_teamLeader;

  List<Member> m_members;

  bool m_hasPendingInvitation;
  PendingInvitation m_pendingInvitation;
  double m_pollInvitationsTimer;
  List<Variant<pair<String, bool>, StringList>> m_pendingInviteResults;

  bool m_fullUpdateRunning;
  double m_fullUpdateTimer;

  bool m_statusUpdateRunning;
  double m_statusUpdateTimer;

  List<RpcResponseHandler> m_pendingResponses;
};

}
