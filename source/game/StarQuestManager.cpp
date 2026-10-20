import star.json;
import star.xx_hash;
import star.json_extra;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.data_stream;
import star.bi_map;
import star.game_types;
import star.set;
import star.json_rpc;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.vector;
import star.strong_typedef;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import star.format;
import star.version;
import star.id_map;
import star.variant;
import star.rpc_promise;
import star.a_star;
import star.ordered_set;
import star.audio;
import star.map;
import star.either;
import star.periodic_function;
import star.weighted_pool;
import star.byte_array;
import star.data_stream_devices;

import std;
typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.btree_database;
import star.image;

import star.lua_root;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.item;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;
import star.drawable;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.lua_components;
import star.uuid;
import star.warping;
import star.quests;
import star.quest_manager;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.humanoid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
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
import star.inspectable_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.game_timers;
import star.actor_movement_controller;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
import star.radio_message_database;
import star.player;
import star.host_address;
import star.sky_types;
import star.weather_types;
import star.world_parameters;
import star.celestial_parameters;
import star.chat_types;
import star.socket;
import star.tcp;
import star.p2p_networking_service;
import star.liquid_types;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.wiring;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.world_storage;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.universe_connection;
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
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;


import star.quest_template_database;
import star.versioning_database;


import star.player_inventory;
import star.client_context;

namespace Star {

QuestManager::QuestManager(Root& root, Player* player) : m_root(root) {
  m_player = player;
  m_world = nullptr;
  m_trackOnWorldQuests = false;
}

std::shared_ptr<QuestTemplate> getTemplate(Root& root, String const& templateId) {
  return root.questTemplateDatabase()->questTemplate(templateId);
}

StringMap<std::shared_ptr<Quest>> readQuests(Root& root, Json const& json) {
  auto versioningDatabase = root.versioningDatabase();
  auto questTemplateDatabase = root.questTemplateDatabase();

  auto validateArc = [questTemplateDatabase](QuestArcDescriptor const& arc) {
      for (auto quest : arc.quests) {
        if (!questTemplateDatabase->questTemplate(quest.templateId))
          return false;
      }
      return true;
    };

  StringMap<std::shared_ptr<Quest>> result;
  for (auto const& questPair : json.iterateObject()) {
    // don't load the quest unless all quests in the arc exist
    Json diskStore = versioningDatabase->loadVersionedJson(VersionedJson::fromJson(questPair.second), "Quest");
    auto questArc = QuestArcDescriptor::diskLoad(root, diskStore.get("arc"));
    if (validateArc(questArc))
      result[questPair.first] = std::make_shared<Quest>(root, questPair.second);
  }
  return result;
}

std::function<bool (std::shared_ptr<Quest> const&)> questFilter(QuestState state) {
  return [state](std::shared_ptr<Quest> const& quest) {
      return quest->state() == state;
    };
}

void QuestManager::diskLoad(Json const& quests) {
  m_quests = readQuests(m_root, quests.get("quests", JsonObject{}));
  m_trackedQuestId = quests.optString("currentQuest");
}

Json QuestManager::diskStore() {
  auto questPtrToJson = [](std::shared_ptr<Quest> const& quest) { return quest->diskStore(); };

  auto quests = m_quests.values();

  return JsonObject{
      {"quests", jsonFromMapV<StringMap<std::shared_ptr<Quest>>>(m_quests, questPtrToJson)},
      {"currentQuest", jsonFromOptional(m_trackedQuestId)}
    };
}

void QuestManager::setUniverseClient(UniverseClient* client) {
  m_client = client;
}

void QuestManager::init(World* world) {
  m_world = world;
  for (auto& quest : m_quests.values()) {
    if (!questValidOnServer(quest))
      continue;
    quest->init(m_player, world, m_client);
  }
  m_trackOnWorldQuests = true;

  // untrack tracked quest if it's not cross-server, and we're on a different server
  if (auto tracked = m_trackedQuestId) {
    if (auto maybeQuest = m_quests.maybe(*tracked)) {
      auto quest = *maybeQuest;
      if (quest->hideCrossServer() && quest->serverUuid() && *quest->serverUuid() != m_player->clientContext()->serverUuid())
        m_trackedQuestId = {};
    }
  }
}

void QuestManager::uninit() {
  m_quests.values().exec([](std::shared_ptr<Quest> const& quest) { quest->uninit(); });
  m_world = nullptr;
}

bool QuestManager::canStart(QuestArcDescriptor const& questArc) const {
  if (questArc.quests.size() == 0)
    return false;

  for (auto questDesc : questArc.quests) {
    auto questTemplate = getTemplate(m_root, questDesc.templateId);
    if (!questTemplate)
      return false;
    if (auto quest = m_quests.maybe(questDesc.questId))
      if ((*quest)->state() != QuestState::Failed)
        return false;
    for (auto const& prerequisiteQuestId : questTemplate->prerequisiteQuests) {
      if (!hasCompleted(prerequisiteQuestId))
        return false;
    }
    for (auto item : questTemplate->requiredItems)
      if (!m_player->inventory()->hasItem(item))
        return false;
    if (questTemplate->requiredShipLevel) {
      if (m_player->shipUpgrades().shipLevel < *questTemplate->requiredShipLevel)
        return false;
    }
  }

  return true;
}

void QuestManager::offer(std::shared_ptr<Quest> const& quest) {
  m_quests[quest->questId()] = quest;
  quest->init(m_player, m_world, m_client);
  quest->offer();
}

StringMap<std::shared_ptr<Quest>> QuestManager::quests() const {
  return m_quests;
}

StringMap<std::shared_ptr<Quest>> QuestManager::serverQuests() const {
  StringMap<std::shared_ptr<Quest>> filtered;
  for (auto& pair : m_quests) {
    std::shared_ptr<Quest> q = pair.second;
    if (!questValidOnServer(q))
      continue;
    filtered.insert(pair.first, q);
  }
  return filtered;
}

std::shared_ptr<Quest> QuestManager::getQuest(String const& questId) const {
  return m_quests.get(questId);
}

bool QuestManager::hasQuest(String const& questId) const {
  return m_quests.contains(questId);
}

bool QuestManager::hasAcceptedQuest(String const& questId) const {
  return m_quests.contains(questId) && m_quests.get(questId)->state() != QuestState::New && m_quests.get(questId)->state() != QuestState::Offer;
}

bool QuestManager::isActive(String const& questId) const {
  return m_quests.contains(questId) && m_quests.get(questId)->state() == QuestState::Active;
}

bool QuestManager::isCurrent(String const& questId) const {
  return (m_onWorldQuestId ? m_onWorldQuestId : m_trackedQuestId) == questId;
}

bool QuestManager::isTracked(String const& questId) const {
  return m_trackedQuestId == questId;
}

void QuestManager::setAsTracked(std::optional<String> const& questId) {
  if (questId && isActive(*questId)) {
    m_trackedQuestId = questId;
    if (m_onWorldQuestId) {
      // stop auto tracking quests on this world
      m_onWorldQuestId = {};
      m_trackOnWorldQuests = false;
    }
  } else {
    m_trackedQuestId = {};
    m_trackOnWorldQuests = true;
  }
}

void QuestManager::markAsRead(String const& questId) {
  if (!m_quests.contains(questId))
    return;

  getQuest(questId)->markAsRead();
}

bool QuestManager::hasCompleted(String const& questId) const {
  if (auto quest = m_quests.maybe(questId))
    if ((*quest)->state() == QuestState::Complete)
      return true;
  return false;
}

bool QuestManager::canTurnIn(String const& questId) const {
  if (auto quest = m_quests.maybe(questId))
    if ((*quest)->state() == QuestState::Active && (*quest)->canTurnIn())
      return true;
  return false;
}

std::optional<std::shared_ptr<Quest>> QuestManager::getFirstNewQuest() {
  for (auto& q : m_quests) {
    if (questValidOnServer(q.second) && q.second->state() == QuestState::Offer)
      return q.second;
  }
  return {};
}

std::optional<std::shared_ptr<Quest>> QuestManager::getFirstCompletableQuest() {
  for (auto& q : m_quests) {
    if (questValidOnServer(q.second) && q.second->state() == QuestState::Complete && q.second->showDialog())
      return q.second;
  }
  return {};
}

std::optional<std::shared_ptr<Quest>> QuestManager::getFirstFailableQuest() {
  for (auto& q : m_quests) {
    if (questValidOnServer(q.second) && q.second->state() == QuestState::Failed && q.second->showDialog())
      return q.second;
  }
  return {};
}

std::optional<std::shared_ptr<Quest>> QuestManager::getFirstMainQuest() {
  for (auto& q : m_quests) {
    if (questValidOnServer(q.second) && q.second->state() == QuestState::Active && q.second->mainQuest())
      return q.second;
  }
  return {};
}

void sortQuests(List<std::shared_ptr<Quest>>& quests) {
  std::sort(quests.begin(),
      quests.end(),
      [](std::shared_ptr<Quest> const& left, std::shared_ptr<Quest> const& right) -> bool {
        std::int64_t leftUpdated = left->lastUpdatedOn();
        std::int64_t rightUpdated = right->lastUpdatedOn();
        String leftQuestId = left->templateId();
        String rightQuestId = right->templateId();
        return std::tie(leftUpdated, leftQuestId) < std::tie(rightUpdated, rightQuestId);
      });
}

List<std::shared_ptr<Quest>> QuestManager::listActiveQuests() const {
  List<std::shared_ptr<Quest>> result = serverQuests().values();
  result.filter([&](std::shared_ptr<Quest> quest) {
      return quest->state() == QuestState::Active && quest->showInLog();
    });
  sortQuests(result);
  return result;
}

List<std::shared_ptr<Quest>> QuestManager::listCompletedQuests() const {
  List<std::shared_ptr<Quest>> result = serverQuests().values();
  result.filter([](std::shared_ptr<Quest> quest) {
      return quest->state() == QuestState::Complete && quest->showInLog();
    });
  sortQuests(result);
  return result;
}

List<std::shared_ptr<Quest>> QuestManager::listFailedQuests() const {
  List<std::shared_ptr<Quest>> result = serverQuests().values();
  result.filter([](std::shared_ptr<Quest> quest) {
      return quest->state() == QuestState::Failed && quest->showInLog();
    });
  sortQuests(result);
  return result;
}

std::optional<String> QuestManager::currentQuestId() const {
  return m_trackedQuestId;
}

std::optional<std::shared_ptr<Quest>> QuestManager::currentQuest() const {
  auto questId = (m_onWorldQuestId ? m_onWorldQuestId : m_trackedQuestId);
  if (questId && isActive(*questId)) {
    auto current = getQuest(*questId);
    if (current->showInLog())
      return current;
  }
  return {};
}

std::optional<String> QuestManager::trackedQuestId() const {
  return m_trackedQuestId;
}

std::optional<std::shared_ptr<Quest>> QuestManager::trackedQuest() const {
  if (m_trackedQuestId && isActive(*m_trackedQuestId)) {
    auto current = getQuest(*m_trackedQuestId);
    if (current->showInLog())
      return current;
  }
  return {};
}

std::optional<QuestIndicator> QuestManager::getQuestIndicator(std::shared_ptr<Entity> const& entity) const {
  std::optional<String> indicatorType;
  Vec2F indicatorPos = entity->position() + Vec2F(0, 2.75);
  auto questGiver = as<InteractiveEntity>(entity);

  if (questGiver) {
    if (!indicatorType) {
      for (auto& questId : questGiver->turnInQuests()) {
        if (!isActive(questId))
          continue;
        auto quest = getQuest(questId);
        if (quest->canTurnIn()) {
          indicatorType = quest->questReceiverIndicator();
          break;
        }
      }
    }

    if (!indicatorType) {
      auto questTemplateDatabase = m_root.questTemplateDatabase();
      for (auto& questArc : questGiver->offeredQuests()) {
        if (canStart(questArc) && questArc.quests.size() > 0) {
          auto& questDesc = questArc.quests[0];
          auto questTemplate = questTemplateDatabase->questTemplate(questDesc.templateId);
          indicatorType = questTemplate->questGiverIndicator;
          break;
        }
      }
    }
  }

  if (indicatorType) {
    Json indicators = m_root.assets()->json("/quests/quests.config:indicators");
    String indicatorImage = indicators.get(*indicatorType).getString("image");
    if (questGiver)
      indicatorPos = questGiver->questIndicatorPosition();
    return QuestIndicator{indicatorImage, indicatorPos};
  }

  for (auto& pair : m_quests) {
    if (pair.second->state() == QuestState::Active) {
      if (auto indicatorImage = pair.second->customIndicator(entity)) {
        if (questGiver)
          indicatorPos = questGiver->questIndicatorPosition();
        return QuestIndicator{ *indicatorImage, indicatorPos };
      }
    }
  }

  return {};
}

StringSet QuestManager::interestingObjects() {
  StringSet result;
  m_quests.values().exec([&result](std::shared_ptr<Quest> const& quest) {
    if (auto resp = quest->receiveMessage("interestingObjects", true, JsonArray()))
      if (resp->is<Json>())
        result.addAll(jsonToStringSet(resp->get<Json>()));
  });
  return result;
}

std::optional<ChainableJsonMessageResponse> QuestManager::receiveMessage(String const& message, bool localMessage, JsonArray const& args) {
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((m_world));
  }
  std::optional<ChainableJsonMessageResponse> result;
  m_quests.values().exec([&result, message, localMessage, args](
      std::shared_ptr<Quest> const& quest) {
        auto response = quest->receiveMessage(message, localMessage, args);
        if (!result)
          result = std::move(response);
      });
  return result;
}

void QuestManager::update(float dt) {
  startInitialQuests();

  if (m_trackedQuestId && !isActive(*m_trackedQuestId))
    m_trackedQuestId = {};

  if (m_onWorldQuestId) {
    auto active = isActive(*m_onWorldQuestId);
    if (active) {
      auto worldId = getQuest(*m_onWorldQuestId)->worldId();
      if (!worldId || m_player->clientContext()->playerWorldId() != *worldId)
        active = false;
    }
    if (!active)
      m_onWorldQuestId = {};
  } else if (m_trackOnWorldQuests) {
    auto playerWorldId = m_client->clientContext()->playerWorldId();
    auto trackedWorld = currentQuest().and_then([](std::shared_ptr<Quest> q) { return q->worldId(); });
    if (!trackedWorld || playerWorldId != *trackedWorld) {
      // the currently tracked quest is not on this world, track another quest on this world
      for (auto quest : listActiveQuests()) {
        if (auto questWorld = quest->worldId()) {
          if (playerWorldId == *questWorld)
            m_onWorldQuestId = quest->questId();  
        }
      }
    }
  }

  List<String> expiredQuests;
  for (auto& entry : m_quests) {
    auto quest = entry.second;
    QuestState state = quest->state();
    bool finished = state == QuestState::Complete || state == QuestState::Failed;
    if (state == QuestState::New || (finished && quest->ephemeral() && !quest->showDialog())) {
      quest->uninit();
      expiredQuests.append(entry.first);
    }
  }

  for (auto& questId : expiredQuests)
    m_quests.remove(questId);

  for (auto& q : m_quests.values()) {
    if (questValidOnServer(q))
      q->update(dt);
  }
}

void QuestManager::startInitialQuests() {
  auto startingQuests =
      m_root.assets()->json(strf("/quests/quests.config:initialquests.{}", m_player->species())).toArray();
  for (auto const& questArcJson : startingQuests) {
    QuestArcDescriptor quest = QuestArcDescriptor::fromJson(m_root, questArcJson);
    if (canStart(quest))
      offer(std::make_shared<Quest>(m_root, quest, 0, m_player));
  }
}

void QuestManager::setMostRecentQuestCurrent() {
  List<std::shared_ptr<Quest>> sortedActiveQuests = listActiveQuests();
  if (sortedActiveQuests.size() > 0)
    setAsTracked(sortedActiveQuests.last()->questId());
}

bool QuestManager::questValidOnServer(std::shared_ptr<Quest> q) const {
  return !(q->hideCrossServer() && q->serverUuid().has_value() && *q->serverUuid() != m_player->clientContext()->serverUuid());
}

}
