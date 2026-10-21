import star.json;
import star.json_extra;
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
import star.json_rpc;
import star.strong_typedef;
import star.game_types;
import std;
import star.time;
import star.map;
import star.version;
import star.random;
import star.ordered_map;
import star.bi_map;
import star.io_device;
import star.ref_ptr;
import star.listener;
import star.text;
import star.logging;

import star.drawable;
import star.uuid;
import star.celestial_coordinate;
import star.warping;
import star.damage_types;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.team_manager;

import star.algorithm;
namespace Star {
  
// update this in the rare case that backwards compat code has to be set up in TeamClient
extern VersionNumber const TeamManagerVersion = 0;

TeamManager::TeamManager(Root& root)
  : m_root(root) {
  m_pvpTeamCounter = 1;
  m_maxTeamSize = m_root.configuration()->get("maxTeamSize").toUInt();
  m_polledInvitationTimeout = m_root.configuration()->getPath("teamInvitationTimeout", Json(600.0)).toDouble();
  m_secureTeams = m_root.configuration()->getPath("security.secureTeams").optBool().value_or(true);
}

JsonRpcHandlers TeamManager::rpcHandlers() {
  return JsonRpcHandlers{
    {"team.fetchTeamStatus", std::bind(&TeamManager::fetchTeamStatus, this, std::placeholders::_1)},
    {"team.updateStatus", std::bind(&TeamManager::updateStatus, this, std::placeholders::_1)},
    {"team.invite", std::bind(&TeamManager::invite, this, std::placeholders::_1)},
    {"team.pollInvitation", std::bind(&TeamManager::pollInvitation, this, std::placeholders::_1)},
    {"team.acceptInvitation", std::bind(&TeamManager::acceptInvitation, this, std::placeholders::_1)},
    {"team.makeLeader", std::bind(&TeamManager::makeLeader, this, std::placeholders::_1)},
    {"team.removeFromTeam", [&](Json request) -> Json { return removeFromTeam(request); }}
  };
}

JsonRpcHandlers TeamManager::authenticatedRpcHandlers(Uuid const& callerUuid) {
  auto wrap = [&](String const& name, JsonRpcRemoteFunction fn) -> std::pair<String, JsonRpcRemoteFunction> {
    return {name, [callerUuid, fn = std::move(fn)](Json const& args) -> Json {
        return fn(args.set("_callerUuid", callerUuid.hex()));
        }};
  };

  return JsonRpcHandlers{
    wrap("team.fetchTeamStatus", std::bind(&TeamManager::fetchTeamStatus, this, std::placeholders::_1)),
    wrap("team.updateStatus", std::bind(&TeamManager::updateStatus, this, std::placeholders::_1)),
    wrap("team.invite", std::bind(&TeamManager::invite, this, std::placeholders::_1)),
    wrap("team.pollInvitation", std::bind(&TeamManager::pollInvitation, this, std::placeholders::_1)),
    wrap("team.acceptInvitation", std::bind(&TeamManager::acceptInvitation, this, std::placeholders::_1)),
    wrap("team.makeLeader", std::bind(&TeamManager::makeLeader, this, std::placeholders::_1)),
    wrap("team.removeFromTeam", [this](Json const& request) -> Json { return removeFromTeam(request); })
  };
}

void TeamManager::setConnectedPlayers(StringMap<List<Uuid>> connectedPlayers) {
  m_connectedPlayers = std::move(connectedPlayers);
}

void TeamManager::playerDisconnected(Uuid const& playerUuid) {
  RecursiveMutexLocker lock(m_mutex);

  purgeInvitationsFor(playerUuid);
  purgeInvitationsFrom(playerUuid);

  for (auto teamUuid : m_teams.keys()) {
    auto& team = m_teams.get(teamUuid);
    if (team.members.contains(playerUuid))
      removeFromTeam(playerUuid, teamUuid);
  }
}

TeamNumber TeamManager::getPvpTeam(Uuid const& playerUuid) {
  RecursiveMutexLocker lock(m_mutex);
  for (auto const& teamPair : m_teams) {
    if (teamPair.second.members.contains(playerUuid))
      return teamPair.second.pvpTeamNumber;
  }
  return 0;
}

HashMap<Uuid, TeamNumber> TeamManager::getPvpTeams() {
  HashMap<Uuid, TeamNumber> result;
  for (auto const& teamPair : m_teams) {
    for (auto const& memberPair : teamPair.second.members)
      result[memberPair.first] = teamPair.second.pvpTeamNumber;
  }
  return result;
}

std::optional<Uuid> TeamManager::getTeam(Uuid const& playerUuid) const {
  for (auto const& teamPair : m_teams) {
    if (teamPair.second.members.contains(playerUuid))
      return teamPair.first;
  }
  return {};
}

void TeamManager::purgeInvitationsFor(Uuid const& playerUuid) {
  m_invitations.remove(playerUuid);
  m_polledInvitations.remove(playerUuid);
}

void TeamManager::purgeInvitationsFrom(Uuid const& playerUuid) {
  eraseWhere(m_invitations, [playerUuid](auto const& invitation) {
    return invitation.second.inviterUuid == playerUuid;
  });
  eraseWhere(m_polledInvitations, [playerUuid](auto const& polled) {
    return polled.second.inviterUuid == playerUuid;
  });
}

void TeamManager::expirePolledInvitations() {
  double now = Time::monotonicTime();
  eraseWhere(m_polledInvitations, [&](auto const& entry) {
    return (now - entry.second.polledAt) >= m_polledInvitationTimeout;
  });
}

bool TeamManager::playerWithUuidExists(Uuid const& playerUuid) const {
  for (auto const& p : m_connectedPlayers) {
    if (p.second.contains(playerUuid))
      return true;
  }
  return false;
}

Uuid TeamManager::createTeam(Uuid const& leaderUuid) {
  RecursiveMutexLocker lock(m_mutex);

  Uuid teamUuid(m_root.random());
  auto& team = m_teams.try_emplace(teamUuid, m_root.random()).first->second; // create

  int limiter = 256;
  while (true) {
    team.pvpTeamNumber = m_pvpTeamCounter++;
    if (m_pvpTeamCounter == 0)
      m_pvpTeamCounter = 1;
    bool done = true;
    for (auto k : m_teams.keys()) {
      auto t = m_teams.get(k);
      if (t.pvpTeamNumber == team.pvpTeamNumber) {
        if (k != teamUuid)
          done = false;
      }
    }
    if (done)
      break;
    if (limiter-- == 0) {
      team.pvpTeamNumber = 0;
      break;
    }
  }

  addToTeam(leaderUuid, teamUuid);
  team.leaderUuid = leaderUuid;

  return teamUuid;
}

bool TeamManager::addToTeam(Uuid const& playerUuid, Uuid const& teamUuid) {
  RecursiveMutexLocker lock(m_mutex);

  if (!m_teams.contains(teamUuid))
    return false;

  auto& team = m_teams.get(teamUuid);

  if (team.members.contains(playerUuid))
    return false;

  if (team.members.size() >= m_maxTeamSize)
    return false;

  purgeInvitationsFor(playerUuid);

  for (auto otherTeam : m_teams) {
    List<Uuid> alreadyMemberOf;
    if (otherTeam.second.members.contains(playerUuid))
      alreadyMemberOf.append(otherTeam.first);
    for (auto leaveTeamUuid : alreadyMemberOf)
      removeFromTeam(playerUuid, leaveTeamUuid);
  }

  team.members.insert(playerUuid, TeamMember());

  return true;
}

bool TeamManager::removeFromTeam(Uuid const& playerUuid, Uuid const& teamUuid) {
  RecursiveMutexLocker lock(m_mutex);

  if (!m_teams.contains(teamUuid))
    return false;

  auto& team = m_teams.get(teamUuid);

  if (!team.members.contains(playerUuid))
    return false;

  purgeInvitationsFrom(playerUuid);

  team.members.remove(playerUuid);
  if (team.members.size() <= 1)
    m_teams.remove(teamUuid);
  else if (team.leaderUuid == playerUuid)
    team.leaderUuid = m_root.random().randFrom(team.members).first;

  return true;
}

Json TeamManager::fetchTeamStatus(Json const& arguments) {
  RecursiveMutexLocker lock(m_mutex);
  auto playerUuid = Uuid(arguments.getString("playerUuid"));

  if (auto callerHex = arguments.optString("_callerUuid")) {
    if (m_secureTeams && Uuid(*callerHex) != playerUuid) {
      m_root.logger().warn("TeamManager: Rejected fetchTeamStatus from {} for {}", *callerHex, playerUuid.hex());
      return {};
    }
  }

  JsonObject result;
  result["version"] = TeamManagerVersion;
  if (auto teamUuid = getTeam(playerUuid)) {
    auto& team = m_teams.get(*teamUuid);
    auto& playerMem = team.members.get(playerUuid);

    result["teamUuid"] = teamUuid->hex();
    result["leader"] = team.leaderUuid.hex();
    JsonArray members;
    for (auto const& m : team.members) {
      JsonObject member;
      auto const& mem = m.second;
      member["name"] = mem.name;
      member["uuid"] = m.first.hex();
      member["leader"] = m.first == team.leaderUuid;
      member["entity"] = mem.entity;
      member["health"] = mem.healthPercentage;
      member["energy"] = mem.energyPercentage;
      member["x"] = mem.position[0];
      member["y"] = mem.position[1];
      if ((std::holds_alternative<CustomWorldId>(mem.world) || std::holds_alternative<ClientCustomWorldId>(mem.world)) && playerMem.version < 1) {
        // caller is too old to handle custom worlds, pretend they're testarena instead to prevent invalid worldid disconnects
        member["world"] = "InstanceWorld:testarena";
      } else {
        member["world"] = printWorldId(mem.world);
      }
      member["warpMode"] = WarpModeNames.getRight(mem.warpMode);
      member["portrait"] = jsonFromList(mem.portrait, std::mem_fn(&Drawable::toJson));
      members.push_back(member);
    }
    result["members"] = members;
  }
  return result;
}

Json TeamManager::updateStatus(Json const& arguments) {
  RecursiveMutexLocker lock(m_mutex);
  auto playerUuid = Uuid(arguments.getString("playerUuid"));

  if (auto callerHex = arguments.optString("_callerUuid")) {
    if (m_secureTeams && Uuid(*callerHex) != playerUuid) {
      m_root.logger().warn("TeamManager: Rejected updateStatus from {} spoofing {}", *callerHex, playerUuid.hex());
      return "unauthorized";
    }
  }

  if (auto teamUuid = getTeam(playerUuid)) {
    auto& team = m_teams.get(*teamUuid);
    auto& entry = team.members.get(playerUuid);
    if (arguments.contains("name"))
      entry.name = arguments.getString("name");
    if (arguments.contains("entity"))
      entry.entity = arguments.getInt("entity");
    entry.healthPercentage = arguments.getFloat("health");
    entry.energyPercentage = arguments.getFloat("energy");
    entry.position[0] = arguments.getFloat("x");
    entry.position[1] = arguments.getFloat("y");
    entry.warpMode = WarpModeNames.getLeft(arguments.getString("warpMode"));
    if (arguments.contains("world"))
      entry.world = parseWorldId(arguments.getString("world"));
    if (arguments.contains("portrait"))
      entry.portrait = jsonToList<Drawable>(arguments.get("portrait"), [this](Json const& value) {
        return Drawable(*m_root.imageMetadataDatabase(), value);
      });
    if (arguments.contains("version"))
      entry.version = arguments.getInt("version");
    return {};
  } else {
    return "notAMemberOfTeam";
  }
}

Json TeamManager::invite(Json const& arguments) {
  RecursiveMutexLocker lock(m_mutex);
  auto inviteeName = Text::stripEscapeCodes(arguments.getString("inviteeName"));

  if (inviteeName.empty())
    return "inviteeNotFound";

  auto inviterUuid = Uuid(arguments.getString("inviterUuid"));

  if (auto callerHex = arguments.optString("_callerUuid")) {
    if (m_secureTeams && Uuid(*callerHex) != inviterUuid) {
      m_root.logger().warn("TeamManager: Rejected invite from {} spoofing inviter {}", *callerHex, inviterUuid.hex());
      return "unauthorized";
    }
  }

  JsonArray invited;
  for (auto& entry : m_connectedPlayers) {
    if (!Text::stripEscapeCodes(entry.first).beginsWith(inviteeName, String::CaseInsensitive))
      continue;

    for (auto& inviteeUuid : entry.second) {
      Invitation invitation(m_root.random());
      invitation.inviterUuid = inviterUuid;
      invitation.inviterName = arguments.getString("inviterName");
      m_invitations.try_emplace(inviteeUuid, m_root.random()).first->second = invitation;
      m_polledInvitations.remove(inviteeUuid);
      invited.append(JsonArray{entry.first, inviteeUuid.hex()});
    }
  }

  if (!invited.empty())
    return invited;
  else
    return "inviteeNotFound";
}

Json TeamManager::pollInvitation(Json const& arguments) {
  RecursiveMutexLocker lock(m_mutex);
  auto playerUuid = Uuid(arguments.getString("playerUuid"));

  if (auto callerHex = arguments.optString("_callerUuid")) {
    if (m_secureTeams && Uuid(*callerHex) != playerUuid) {
      m_root.logger().warn("TeamManager: Rejected pollInvitation from {} for {}", *callerHex, playerUuid.hex());
      return {};
    }
  }

  expirePolledInvitations();

  if (m_invitations.contains(playerUuid)) {
    auto invite = m_invitations.take(playerUuid);
    m_polledInvitations.try_emplace(playerUuid, m_root.random()).first->second = PolledInvitation{invite.inviterUuid, Time::monotonicTime()};
    JsonObject result;
    result["inviterUuid"] = invite.inviterUuid.hex();
    result["inviterName"] = invite.inviterName;
    return result;
  }
  return {};
}

Json TeamManager::acceptInvitation(Json const& arguments) {
  RecursiveMutexLocker lock(m_mutex);
  auto inviterUuid = Uuid(arguments.getString("inviterUuid"));
  auto inviteeUuid = Uuid(arguments.getString("inviteeUuid"));

  if (auto callerHex = arguments.optString("_callerUuid")) {
    if (m_secureTeams && Uuid(*callerHex) != inviteeUuid) {
      m_root.logger().warn("TeamManager: Rejected acceptInvitation from {} spoofing invitee {}", *callerHex, inviteeUuid.hex());
      return "unauthorized";
    }
  }

  if (!playerWithUuidExists(inviterUuid) || !playerWithUuidExists(inviteeUuid))
    return "acceptInvitationFailed";

  expirePolledInvitations();

  auto polledIt = m_polledInvitations.find(inviteeUuid);
  if (polledIt == m_polledInvitations.end() || polledIt->second.inviterUuid != inviterUuid) {
    m_root.logger().warn("TeamManager: Rejected acceptInvitation - no valid polled invitation from {} to {}", inviterUuid.hex(), inviteeUuid.hex());
    return "noInvitationPending";
  }

  m_polledInvitations.remove(inviteeUuid);

  purgeInvitationsFrom(inviteeUuid);

  Uuid teamUuid(m_root.random());
  if (auto existingTeamUuid = getTeam(inviterUuid))
    teamUuid = *existingTeamUuid;
  else
    teamUuid = createTeam(inviterUuid);

  auto success = addToTeam(inviteeUuid, teamUuid);
  return success ? Json() : "acceptInvitationFailed";
}

Json TeamManager::removeFromTeam(Json const& arguments) {
  RecursiveMutexLocker lock(m_mutex);
  auto playerUuid = Uuid(arguments.getString("playerUuid"));
  auto teamUuid = Uuid(arguments.getString("teamUuid"));

  if (auto callerHex = arguments.optString("_callerUuid")) {
    Uuid callerUuid(*callerHex);
    if (m_secureTeams && callerUuid != playerUuid) {
      if (!m_teams.contains(teamUuid) || m_teams.get(teamUuid).leaderUuid != callerUuid) {
        m_root.logger().warn("TeamManager: Rejected removeFromTeam by non-leader {} targeting {}", callerUuid.hex(), playerUuid.hex());
        return "unauthorized";
      }
    }
  }

  auto success = removeFromTeam(playerUuid, teamUuid);
  return success ? Json() : "removeFromTeamFailed";
}

Json TeamManager::makeLeader(Json const& arguments) {
  RecursiveMutexLocker lock(m_mutex);
  auto playerUuid = Uuid(arguments.getString("playerUuid"));
  auto teamUuid = Uuid(arguments.getString("teamUuid"));

  if (!m_teams.contains(teamUuid))
    return "noSuchTeam";

  auto& team = m_teams.get(teamUuid);

  if (auto callerHex = arguments.optString("_callerUuid")) {
    if (m_secureTeams && Uuid(*callerHex) != team.leaderUuid) {
      m_root.logger().warn("TeamManager: Rejected makeLeader by non-leader {} in team {}", *callerHex, teamUuid.hex());
      return "unauthorized";
    }
  }

  if (!team.members.contains(playerUuid))
    return "notAMemberOfTeam";

  team.leaderUuid = playerUuid;

  return {};
}

}
