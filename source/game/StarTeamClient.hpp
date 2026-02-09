#pragma once

#include "StarConfig.hpp"
#include "StarDrawable.hpp"
#include "StarRpcPromise.hpp"
#include "StarUuid.hpp"
#include "StarWarping.hpp"

import std;

namespace Star {

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

  TeamClient(Ptr<Player> mainPlayer, Ptr<ClientContext> clientContext);

  void invitePlayer(String const& playerName);
  void acceptInvitation(Uuid const& inviterUuid);

  [[nodiscard]] auto currentTeam() const -> std::optional<Uuid>;

  void makeLeader(Uuid const& playerUuid);
  void removeFromTeam(Uuid const& playerUuid);

  auto isTeamLeader() -> bool;
  auto isTeamLeader(Uuid const& playerUuid) -> bool;
  auto isMemberOfTeam() -> bool;

  auto hasInvitationPending() -> bool;
  auto pullInvitation() -> std::pair<Uuid, String>;
  auto pullInviteResults() -> List<Variant<std::pair<String, bool>, StringList>>;

  void update();

  void pullFullUpdate();
  void statusUpdate();

  void forceUpdate();

  auto members() -> List<Member>;

private:
  using RpcResponseHandler = std::pair<RpcPromise<Json>, std::function<void(Json const&)>>;

  void invokeRemote(String const& method, Json const& args, std::function<void(Json const&)> responseFunction = {});
  void handleRpcResponses();

  void writePlayerData(JsonObject& request, Ptr<Player> player, bool fullWrite = false) const;

  void clearTeam();

  Ptr<Player> m_mainPlayer;
  Ptr<ClientContext> m_clientContext;
  std::optional<Uuid> m_teamUuid;

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
};

}// namespace Star
