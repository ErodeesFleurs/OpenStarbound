import star.json;
import star.xx_hash;
import star.json_extra;
import star.data_stream;
import star.string;
import star.vector;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.strong_typedef;
import star.game_types;
import star.time;
import star.version;
import star.ordered_map;
import star.lru_cache;
import star.bi_map;
import star.random;
import star.perlin;
import star.weighted_pool;
import star.id_map;
import star.set;
import star.casting;
import star.either;
import star.rpc_promise;
import star.a_star;
import star.ordered_set;
import star.audio;
import star.map;
import star.periodic_function;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import star.image;
import star.net_compatibility;
import std;
import star.worker_pool;

import star.sector_array_2d;
import star.btree;
import star.data_stream_devices;
import star.btree_database;
import star.json_rpc;

import star.lua_root;
import star.uuid;
import star.drawable;
import star.celestial_coordinate;
import star.warping;
import star.animation;
import star.particle;
import star.weather_types;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_damage;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.celestial_types;
import star.sky_parameters;
import star.world_template;
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.humanoid;
import star.tile_modification;
import star.world;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.game_timers;
import star.actor_movement_controller;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;
import star.player_log;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.liquid_types;
import star.tile_sector_array;
import star.collision_generator;
import star.world_tiles;
import star.chat_types;
import star.wiring;
import star.versioning_database;
import star.world_storage;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;

import star.client_context;
import star.team_client;

import star.algorithm;
namespace Star {
  
// update this in the rare case that backwards compat code has to be set up in TeamManager
extern VersionNumber const TeamClientVersion = 1;
  
TeamClient::TeamClient(Root& root, std::shared_ptr<Player> mainPlayer, std::shared_ptr<ClientContext> clientContext)
  : m_root(root), m_teamLeader(root.random()), m_pendingInvitation(Uuid(root.random()), String()) {
  m_mainPlayer = mainPlayer;
  m_clientContext = clientContext;

  m_hasPendingInvitation = false;
  m_pollInvitationsTimer = 0;

  m_fullUpdateRunning = false;
  m_fullUpdateTimer = 0;

  m_statusUpdateRunning = false;
  m_statusUpdateTimer = 0;
}

bool TeamClient::isTeamLeader() {
  if (!m_teamUuid)
    return false;
  return m_teamLeader == m_clientContext->playerUuid();
}

bool TeamClient::isTeamLeader(Uuid const& playerUuid) {
  if (!m_teamUuid)
    return false;
  return m_teamLeader == playerUuid;
}

bool TeamClient::isMemberOfTeam() {
  return (bool)m_teamUuid;
}

void TeamClient::invitePlayer(String const& playerName) {
  if (playerName.empty())
    return;

  JsonObject request;
  request["inviteeName"] = playerName;
  request["inviterUuid"] = m_clientContext->playerUuid().hex();
  request["inviterName"] = m_mainPlayer->name();
  invokeRemote("team.invite", request, [=, this](Json response) {
    if (!response)
      m_pendingInviteResults.append(std::make_pair(playerName, true));
    else if (response == "inviteeNotFound")
      m_pendingInviteResults.append(std::make_pair(playerName, false));
    else if (response.isType(Json::Type::Array)) {
      m_pendingInviteResults.push_back(StringList());
      StringList& invited = std::get<StringList>(m_pendingInviteResults.back());
      for (auto& entry : response.toArray()) {
        if (!entry.isType(Json::Type::Array))
          continue;
        auto name = entry.get(0, Json());
        if (name.isType(Json::Type::String))
          invited.append(name.toString());
      }
    }
  });
}

void TeamClient::acceptInvitation(Uuid const& inviterUuid) {
  JsonObject request;
  request["inviterUuid"] = inviterUuid.hex();
  request["inviteeUuid"] = m_clientContext->playerUuid().hex();
  invokeRemote("team.acceptInvitation", request, [this](Json) { forceUpdate(); });
}

std::optional<Uuid> TeamClient::currentTeam() const {
  return m_teamUuid;
}

void TeamClient::makeLeader(Uuid const& playerUuid) {
  if (!m_teamUuid)
    return;
  if (!isTeamLeader())
    return;
  JsonObject request;
  request["teamUuid"] = m_teamUuid->hex();
  request["playerUuid"] = playerUuid.hex();
  invokeRemote("team.makeLeader", request, [this](Json) { forceUpdate(); });
}

void TeamClient::removeFromTeam(Uuid const& playerUuid) {
  if (!m_teamUuid)
    return;
  if (!isTeamLeader() && playerUuid != m_clientContext->playerUuid())
    return;
  JsonObject request;
  request["teamUuid"] = m_teamUuid->hex();
  request["playerUuid"] = playerUuid.hex();
  invokeRemote("team.removeFromTeam", request, [this](Json) { forceUpdate(); });
}

bool TeamClient::hasInvitationPending() {
  return m_hasPendingInvitation;
}

std::pair<Uuid, String> TeamClient::pullInvitation() {
  m_hasPendingInvitation = false;
  return m_pendingInvitation;
}

List<std::variant<std::pair<String, bool>, StringList>> TeamClient::pullInviteResults() {
  return take(m_pendingInviteResults);
}

void TeamClient::update() {
  handleRpcResponses();

  if (!m_hasPendingInvitation) {
    if (Time::monotonicTime() - m_pollInvitationsTimer > m_root.assets()->json("/interface.config:invitationPollInterval").toFloat()) {
      m_pollInvitationsTimer = Time::monotonicTime();
      JsonObject request;
      request["playerUuid"] = m_clientContext->playerUuid().hex();
      invokeRemote("team.pollInvitation", request, [this](Json response) {
          if (response.isNull())
            return;
          if (m_hasPendingInvitation)
            return;
          m_pendingInvitation = {Uuid(response.getString("inviterUuid")), response.getString("inviterName")};
          m_hasPendingInvitation = true;
        });
    }
  }
  if (!m_fullUpdateRunning) {
    if (Time::monotonicTime() - m_fullUpdateTimer > m_root.assets()->json("/interface.config:fullUpdateInterval").toFloat()) {
      m_fullUpdateTimer = Time::monotonicTime();
      pullFullUpdate();
    }
  }
  if (!m_statusUpdateRunning) {
    if (Time::monotonicTime() - m_statusUpdateTimer > m_root.assets()->json("/interface.config:statusUpdateInterval").toFloat()) {
      m_statusUpdateTimer = Time::monotonicTime();
      statusUpdate();
    }
  }
}

void TeamClient::pullFullUpdate() {
  if (m_fullUpdateRunning)
    return;
  m_fullUpdateRunning = true;
  JsonObject request;
  request["playerUuid"] = m_clientContext->playerUuid().hex();

  invokeRemote("team.fetchTeamStatus", request, [this](Json response) {
      m_fullUpdateRunning = false;

      m_teamManagerVersion = response.optInt("version").value_or(0);
      
      m_teamUuid = response.optString("teamUuid").transform(construct<Uuid>());

      if (m_teamUuid) {
        m_teamLeader = Uuid(response.getString("leader"));
        m_members.clear();

        for (auto m : response.getArray("members")) {
          Member member{
              .name = m.getString("name"),
              .uuid = Uuid(m.getString("uuid")),
              .entity = static_cast<EntityId>(m.getInt("entity")),
              .healthPercentage = m.getFloat("health"),
              .energyPercentage = m.getFloat("energy"),
              .world = parseWorldId(m.getString("world")),
              .position = Vec2F(m.getFloat("x"), m.getFloat("y")),
              .warpMode = WarpModeNames.getLeft(m.getString("warpMode")),
              .portrait = jsonToList<Drawable>(m.get("portrait"), [this](Json const& value) {
                return Drawable(*m_root.imageMetadataDatabase(), value);
              })};
          m_members.push_back(member);
        }
        std::sort(m_members.begin(), m_members.end(), [](Member const& a, Member const& b) { return a.name < b.name; });
      } else {
        clearTeam();
      }
    });
}

void TeamClient::statusUpdate() {
  if (m_statusUpdateRunning)
    return;
  if (!m_teamUuid)
    return;
  m_statusUpdateRunning = true;
  JsonObject request;
  auto player = m_mainPlayer;

  // TODO: write full player data less often?
  writePlayerData(request, player, true);

  invokeRemote("team.updateStatus", request, [this](Json) {
      m_statusUpdateRunning = false;
    });
}

List<TeamClient::Member> TeamClient::members() {
  return m_members;
}

void TeamClient::forceUpdate() {
  m_statusUpdateTimer = 0;
  m_fullUpdateTimer = 0;
  m_pollInvitationsTimer = 0;
}

void TeamClient::invokeRemote(String const& method, Json const& args, std::function<void(Json const&)> responseFunction) {
  auto promise = m_clientContext->rpcInterface()->invokeRemote(method, args);
  m_pendingResponses.append({std::move(promise), std::move(responseFunction)});
}

void TeamClient::handleRpcResponses() {
  List<RpcResponseHandler> stillPendingResponses;
  while (m_pendingResponses.size() > 0) {
    auto handler = m_pendingResponses.takeLast();
    if (handler.first.finished()) {
      if (auto const& res = handler.first.result()) {
        if (handler.second)
          handler.second(*res);
      }
    } else {
      stillPendingResponses.append(std::move(handler));
    }
  }
  m_pendingResponses = stillPendingResponses;
}

void TeamClient::writePlayerData(JsonObject& request, std::shared_ptr<Player> player, bool fullWrite) const {
  request["version"] = TeamClientVersion;
  request["playerUuid"] = m_clientContext->playerUuid().hex();
  request["entity"] = player->entityId();
  request["health"] = player->health() / player->maxHealth();
  request["energy"] = player->energy() / player->maxEnergy();
  request["x"] = player->position()[0];
  request["y"] = player->position()[1];
  request["world"] = printWorldId(m_clientContext->playerWorldId());

  WarpMode mode = WarpMode::None;
  if (player->log()->introComplete()) {
    if (std::holds_alternative<CelestialWorldId>(m_clientContext->playerWorldId()))
      mode = WarpMode::BeamOnly;
    else
      mode = player->isDeployed() ? WarpMode::DeployOnly : WarpMode::BeamOnly;
  }
  request["warpMode"] = WarpModeNames.getRight(mode);

  if (fullWrite) {
    request["name"] = player->name();
    request["portrait"] = jsonFromList(player->portrait(PortraitMode::Head), std::mem_fn(&Drawable::toJson));
  }
}

void TeamClient::clearTeam() {
  m_teamLeader = Uuid(m_root.random());
  m_teamUuid = {};
  m_members.clear();
  forceUpdate();
}

}
