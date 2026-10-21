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
import star.file;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import star.format;
import star.version;
import star.id_map;
import star.rpc_promise;
import star.lua;
import star.audio;
import star.map;
import star.a_star;
import star.periodic_function;
import star.periodic;
import star.ordered_set;
import star.either;
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
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.mobile_entity;
import star.scripted_entity;
import star.game_timers;
import star.item_drop;
import star.mixer;
import star.entity_rendering_types;
import star.animation;
import star.particle;
import star.entity_rendering;
import star.platformer_astar_types;
import star.anchorable_entity;
import star.actor_movement_controller;
import star.animated_part_set;
import star.networked_animator;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.aggressive_entity;
import star.chat_action;
import star.chatty_entity;
import star.lua_animation_component;
import star.actor_entity;
import star.monster_database;
import star.effect_emitter;
import star.monster;
import star.humanoid;
import star.emote_entity;
import star.lounging_entities;
import star.tool_user_entity;
import star.npc_database;
import star.npc;
import star.status_effect_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.object;
import star.quests;
import star.quest_manager;
import star.inventory_types;
import star.ai_types;
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

import star.player_tech;

import star.player_lua_bindings;
import star.entity_lua_bindings;
import star.config_lua_bindings;
import star.celestial_lua_bindings;
import star.status_controller_lua_bindings;
import star.quest_template_database;
import star.versioning_database;
import star.object_database;


import star.player_inventory;
import star.client_context;

import star.algorithm;
namespace Star {

extern EnumMap<QuestState> const QuestStateNames {
  {QuestState::New, "New"},
  {QuestState::Offer, "Offer"},
  {QuestState::Active, "Active"},
  {QuestState::Complete, "Complete"},
  {QuestState::Failed, "Failed"}
};

Quest::Quest(Root& root, QuestArcDescriptor const& questArc, std::size_t arcPos, Player* player) : m_root(root), m_scriptComponent(root) {
  m_trackedIndicator = m_root.assets()->json("/quests/quests.config:trackedCustomIndicator").toString();
  m_untrackedIndicator = m_root.assets()->json("/quests/quests.config:untrackedCustomIndicator").toString();

  m_money = 0;
  m_unread = true;
  m_canTurnIn = false;
  m_lastUpdatedOn = Time::monotonicMilliseconds();

  m_arc = questArc;
  m_arcPos = arcPos;

  auto itemDatabase = m_root.itemDatabase();
  auto templateDatabase = m_root.questTemplateDatabase();
  auto questTemplate = templateDatabase->questTemplate(templateId());

  m_parameters = questDescriptor().parameters;
  m_displayParameters = DisplayParameters {
     questTemplate->ephemeral,
     questTemplate->showInLog,
     questTemplate->showAcceptDialog,
     questTemplate->showCompleteDialog,
     questTemplate->showFailDialog,
     questTemplate->mainQuest,
     questTemplate->hideCrossServer
  };
  setEntityParameter("player", player);

  m_money = m_root.random().randUInt(questTemplate->moneyRange[0], questTemplate->moneyRange[1]);
  m_rewards = m_root.random().randValueFrom(questTemplate->rewards, {}).transformed(
      [&itemDatabase](ItemDescriptor const& item) -> std::shared_ptr<Item const> { return itemDatabase->item(item); });

  for (String const& rewardParamName : questTemplate->rewardParameters) {
    if (!m_parameters.contains(rewardParamName))
      continue;
    QuestParam const& rewardParam = m_parameters[rewardParamName];
    if (std::holds_alternative<QuestItem>(rewardParam.detail)) {
      addReward(std::get<QuestItem>(rewardParam.detail).descriptor());
    } else {
      if (!std::holds_alternative<QuestItemList>(rewardParam.detail))
        throw StarException::format("Quest parameter {} cannot be used as a reward parameter", rewardParamName);
      for (auto item : std::get<QuestItemList>(rewardParam.detail))
        addReward(item);
    }
  }

  m_title = questTemplate->title;
  m_text = questTemplate->text;
  m_completionText = questTemplate->completionText;
  m_failureText = questTemplate->failureText;

  m_state = QuestState::New;
  m_showDialog = false;

  m_player = nullptr;
  m_world = nullptr;
  m_inited = false;
}

Quest::Quest(Root& root, Json const& spec) : m_root(root), m_scriptComponent(root) {
  m_trackedIndicator = m_root.assets()->json("/quests/quests.config:trackedCustomIndicator").toString();
  m_untrackedIndicator = m_root.assets()->json("/quests/quests.config:untrackedCustomIndicator").toString();

  auto versioningDatabase = m_root.versioningDatabase();
  Json diskStore = versioningDatabase->loadVersionedJson(VersionedJson::fromJson(spec), "Quest");

  m_state = QuestStateNames.getLeft(diskStore.getString("state"));

  m_arc = QuestArcDescriptor::diskLoad(m_root, diskStore.get("arc"));
  m_arcPos = diskStore.getUInt("arcPos");
  m_parameters = questParamsDiskLoad(m_root, diskStore.get("parameters"));
  m_worldId = diskStore.optString("worldId").transform(parseWorldId);
  m_location = diskStore.opt("location").transform([](Json const& json) {
    return std::make_pair(jsonToVec3I(json.get("system")), jsonToSystemLocation(json.get("location")));
  });
  m_serverUuid = diskStore.optString("serverUuid").transform(construct<Uuid>());
  m_money = diskStore.getUInt("money");

  auto itemDatabase = m_root.itemDatabase();
  m_rewards = diskStore.getArray("rewards").transformed(
      [&itemDatabase](Json const& json) -> std::shared_ptr<Item const> { return itemDatabase->diskLoad(json); });

  m_lastUpdatedOn = diskStore.getInt("lastUpdatedOn");
  m_unread = diskStore.getBool("unread", true);
  m_canTurnIn = diskStore.getBool("canTurnIn", false);
  m_indicators = jsonToStringSet(diskStore.get("indicators", JsonArray{}));
  m_scriptComponent.setScriptStorage(diskStore.getObject("scriptStorage", JsonObject{}));

  auto templateDatabase = m_root.questTemplateDatabase();
  auto questTemplate = templateDatabase->questTemplate(templateId());
  m_displayParameters = DisplayParameters{
    questTemplate->ephemeral,
    questTemplate->showInLog,
    questTemplate->showAcceptDialog,
    questTemplate->showCompleteDialog,
    questTemplate->showFailDialog,
    questTemplate->mainQuest,
    questTemplate->hideCrossServer
  };

  m_title = diskStore.getString("title", questTemplate->title);
  m_text = diskStore.getString("text", questTemplate->text);
  m_completionText = diskStore.getString("completionText", questTemplate->completionText);
  m_failureText = diskStore.getString("failureText", questTemplate->failureText);

  m_portraits = jsonToMapV<StringMap<List<Drawable>>>(diskStore.get("portraits", JsonObject{}), [this](Json const& portrait) {
      return portrait.toArray().transformed([this](Json const& drawable) { return Drawable(*m_root.imageMetadataDatabase(), drawable); });
    });
  m_portraitTitles = jsonToMapV<StringMap<String>>(diskStore.get("portraitTitles", JsonObject{}), std::mem_fn(&Json::toString));
  m_showDialog = diskStore.getBool("showDialog", false);

  m_player = nullptr;
  m_world = nullptr;
  m_inited = false;
}

Json Quest::diskStore() const {
  auto versioningDatabase = m_root.versioningDatabase();

  JsonObject result;
  result["state"] = QuestStateNames.getRight(m_state);
  result["arc"] = m_arc.diskStore(m_root);
  result["arcPos"] = m_arcPos;
  result["parameters"] = questParamsDiskStore(m_root, m_parameters);
  result["money"] = m_money;

  result["worldId"] = jsonFromOptional(m_worldId.transform(printWorldId));
  result["location"] = jsonFromOptional(m_location.transform([](std::pair<Vec3I, SystemLocation> const& location) {
      return JsonObject{{"system", jsonFromVec3I(location.first)}, {"location", jsonFromSystemLocation(location.second)}};
    }));
  result["serverUuid"] = jsonFromOptional(m_serverUuid.transform(std::mem_fn(&Uuid::hex)));

  auto itemDatabase = m_root.itemDatabase();
  result["rewards"] =
      m_rewards.transformed([&itemDatabase](std::shared_ptr<Item const> const& item) { return itemDatabase->diskStore(item); });

  result["lastUpdatedOn"] = m_lastUpdatedOn;
  result["unread"] = m_unread;
  result["canTurnIn"] = m_canTurnIn;
  result["indicators"] = jsonFromStringSet(m_indicators);
  result["scriptStorage"] = m_scriptComponent.getScriptStorage();

  result["title"] = m_title;
  result["text"] = m_text;
  result["completionText"] = m_completionText;
  result["failureText"] = m_failureText;

  result["portraits"] = jsonFromMapV(m_portraits, [](List<Drawable> const& portrait) {
      return portrait.transformed(std::mem_fn(&Drawable::toJson));
    });
  result["portraitTitles"] = jsonFromMap(m_portraitTitles);
  result["showDialog"] = m_showDialog;

  return versioningDatabase->makeCurrentVersionedJson("Quest", result).toJson();
}

std::shared_ptr<QuestTemplate> Quest::getTemplate() const {
  return m_root.questTemplateDatabase()->questTemplate(templateId());
}

void Quest::init(Player* player, World* world, UniverseClient* client) {
  m_player = player;
  m_world = world;
  m_client = client;

  if (m_state == QuestState::Offer || m_state == QuestState::Active)
    initScript();
}

void Quest::uninit() {
  if (m_inited)
    uninitScript();

  m_player = nullptr;
  m_world = nullptr;
}

std::optional<ChainableJsonMessageResponse> Quest::receiveMessage(String const& message, bool localMessage, JsonArray const& args) {
  if (!m_inited)
    return {};
  return m_scriptComponent.handleMessage(message, localMessage, args);
}


std::optional<LuaValue> Quest::callScript(String const& func, LuaVariadic<LuaValue> const& args) {
  if (!m_inited)
    return {};
  return m_scriptComponent.invoke(func, args);
}

void Quest::update(float dt) {
  if (!m_inited)
    return;
  m_scriptComponent.update(m_scriptComponent.updateDt(dt));
}

void Quest::offer() {
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((m_player && m_world));
  }

  if (!showAcceptDialog()) {
    start();
  } else {
    setState(QuestState::Offer);
    initScript();
    m_scriptComponent.invoke("questOffer");
  }
}

void Quest::declineOffer() {
  setState(QuestState::New);
  m_scriptComponent.invoke("questDecline");
  uninitScript();
}

void Quest::cancelOffer() {
  setState(QuestState::New);
  m_scriptComponent.invoke("questCancel");
  uninitScript();
}

void Quest::start() {
  setState(QuestState::Active);
  initScript();

  auto current = m_player->questManager()->trackedQuest();
  if (mainQuest() || !current)
    m_player->questManager()->setAsTracked(questId());

  m_scriptComponent.invoke("questStart");
}

void Quest::complete(std::optional<std::size_t> followupIndex) {
  setState(QuestState::Complete);
  m_showDialog = showCompleteDialog();
  m_scriptComponent.invoke("questComplete");
  uninitScript();

  // Grant reward items and money
  for (auto const& item : rewards())
    m_player->giveItem(item->clone());
  m_player->inventory()->addCurrency("money", money());

  // Offer follow-up quests
  bool trackNewQuest = m_player->questManager()->isTracked(questId());
  std::size_t nextArcPos = followupIndex.value_or(questArcPosition() + 1);
  if (nextArcPos < m_arc.quests.size()) {
    auto followUp = std::make_shared<Quest>(m_root, m_arc, nextArcPos, m_player);
    followUp->setWorldId(worldId());
    followUp->setLocation(location());
    followUp->setServerUuid(serverUuid());
    m_player->questManager()->offer(followUp);
    if (trackNewQuest)
      m_player->questManager()->setAsTracked(followUp->questId());
  } else if (trackNewQuest) {
    // no followup, track another main quest or clear quest tracker
    if (auto main = m_player->questManager()->getFirstMainQuest())
      m_player->questManager()->setAsTracked(main.value()->questId());
    else
      m_player->questManager()->setAsTracked({});
  }
}

void Quest::fail() {
  setState(QuestState::Failed);
  m_showDialog = showFailDialog();
  m_scriptComponent.invoke("questFail", false);
  uninitScript();
}

void Quest::abandon() {
  setState(QuestState::Failed);
  m_showDialog = false;
  m_scriptComponent.invoke("questFail", true);
  uninitScript();
}

bool Quest::interactWithEntity(EntityId entity) {
  std::optional<bool> result = m_scriptComponent.invoke<bool>("questInteract", entity);
  return result && result.value();
}

String Quest::questId() const {
  return questDescriptor().questId;
}

String Quest::templateId() const {
  return questDescriptor().templateId;
}

StringMap<QuestParam> const& Quest::parameters() const {
  return m_parameters;
}

QuestState Quest::state() const {
  return m_state;
}

bool Quest::showDialog() const {
  return m_showDialog;
}

void Quest::setDialogShown() {
  m_showDialog = false;
}

void Quest::setEntityParameter(String const& paramName, std::shared_ptr<Entity const> const& entity) {
  setEntityParameter(paramName, entity.get());
}

void Quest::setParameter(String const& paramName, QuestParam const& paramValue) {
  m_parameters[paramName] = paramValue;
}

std::optional<List<Drawable>> Quest::portrait(String const& portraitName) const {
  return m_portraits.maybe(portraitName);
}

std::optional<String> Quest::portraitTitle(String const& portraitName) const {
  return m_portraitTitles.maybe(portraitName);
}

QuestDescriptor Quest::questDescriptor() const {
  return m_arc.quests[m_arcPos];
}

QuestArcDescriptor Quest::questArcDescriptor() const {
  return m_arc;
}

std::size_t Quest::questArcPosition() const {
  return m_arcPos;
}

std::optional<WorldId> Quest::worldId() const {
  return m_worldId;
}

std::optional<std::pair<Vec3I, SystemLocation>> Quest::location() const {
  return m_location;
}

std::optional<Uuid> Quest::serverUuid() const {
  return m_serverUuid;
}

void Quest::setWorldId(std::optional<WorldId> worldId) {
  m_worldId = worldId;
}

void Quest::setLocation(std::optional<std::pair<Vec3I, SystemLocation>> location) {
  m_location = std::move(location);
}

void Quest::setServerUuid(std::optional<Uuid> serverUuid) {
  m_serverUuid = serverUuid;
}

String Quest::title() const {
  return m_title;
}

String Quest::text() const {
  return m_text;
}

String Quest::completionText() const {
  return m_completionText;
}

String Quest::failureText() const {
  return m_failureText;
}

std::size_t Quest::money() const {
  return m_money;
}

List<std::shared_ptr<Item const>> Quest::rewards() const {
  return m_rewards;
}

std::int64_t Quest::lastUpdatedOn() const {
  return m_lastUpdatedOn;
}

bool Quest::unread() const {
  return m_unread;
}

void Quest::markAsRead() {
  m_unread = false;
}

bool Quest::canTurnIn() const {
  return m_canTurnIn;
}

String Quest::questGiverIndicator() const {
  return getTemplate()->questGiverIndicator;
}

String Quest::questReceiverIndicator() const {
  return getTemplate()->questReceiverIndicator;
}

bool hasItemIndicator(Root& root, std::shared_ptr<Entity> const& entity, List<ItemDescriptor> indicatedItems) {
  if (auto itemDrop = as<ItemDrop>(entity)) {
    for (auto const& itemDesc : indicatedItems) {
      if (itemDrop->item()->matches(itemDesc, true)) {
        return true;
      }
    }
  } else if (auto object = as<Object>(entity)) {
    std::shared_ptr<ObjectConfig> objectConfig = root.objectDatabase()->getConfig(object->name());
    if (!objectConfig->hasObjectItem)
      return false;
    for (auto const& itemDesc : indicatedItems) {
      if (object->name() == itemDesc.name())
        return true;
    }
  }
  return false;
}

std::optional<String> Quest::customIndicator(std::shared_ptr<Entity> const& entity) const {
  if (!m_inited)
    return {};

  for (String const& indicator : m_indicators) {
    auto param = parameters().get(indicator);
    if (std::holds_alternative<QuestEntity>(param.detail)) {
      QuestEntity questEntity = std::get<QuestEntity>(param.detail);
      if (questEntity.uniqueId && entity->uniqueId() == questEntity.uniqueId) {
        return param.indicator.value_or(defaultCustomIndicator());
      }

    } else if (std::holds_alternative<QuestItem>(param.detail)) {
      QuestItem questItem = std::get<QuestItem>(param.detail);
      if (hasItemIndicator(m_root, entity, {questItem.descriptor()}))
        return param.indicator.value_or(defaultCustomIndicator());

    } else if (std::holds_alternative<QuestItemTag>(param.detail)) {
      String questItemTag = std::get<QuestItemTag>(param.detail);
      if (auto itemDrop = as<ItemDrop>(entity)) {
        if (itemDrop->item()->itemTags().contains(questItemTag))
          return param.indicator.value_or(defaultCustomIndicator());
      }

    } else if (std::holds_alternative<QuestItemList>(param.detail)) {
      QuestItemList questItemList = std::get<QuestItemList>(param.detail);
      if (hasItemIndicator(m_root, entity, questItemList))
        return param.indicator.value_or(defaultCustomIndicator());

    } else if (std::holds_alternative<QuestMonsterType>(param.detail)) {
      QuestMonsterType questMonsterType = std::get<QuestMonsterType>(param.detail);
      if (auto monster = as<Monster>(entity)) {
        if (monster->typeName() == questMonsterType.typeName) {
          TeamType team = monster->getTeam().type;
          if (team == TeamType::Enemy || team == TeamType::Passive)
            return param.indicator.value_or(defaultCustomIndicator());
        }
      }

    }
  }
  return {};
}

std::optional<JsonArray> Quest::objectiveList() const {
  return m_objectiveList;
}

std::optional<float> Quest::progress() const {
  return m_progress;
}

std::optional<float> Quest::compassDirection() const {
  return m_compassDirection;
}

void Quest::setObjectiveList(std::optional<JsonArray> const& objectiveList) {
  m_objectiveList = objectiveList;
}

void Quest::setProgress(std::optional<float> const& progress) {
  m_progress = progress;
}

void Quest::setCompassDirection(std::optional<float> const& compassDirection) {
  m_compassDirection = compassDirection;
}

std::optional<String> Quest::completionCinema() const {
  return getTemplate()->completionCinema;
}

bool Quest::canBeAbandoned() const {
  return getTemplate()->canBeAbandoned;
}

bool Quest::ephemeral() const {
  return m_displayParameters.ephemeral;
}

bool Quest::showInLog() const {
  return m_displayParameters.showInLog;
}

bool Quest::showAcceptDialog() const {
  return m_displayParameters.showAcceptDialog;
}

bool Quest::showCompleteDialog() const {
  return m_displayParameters.showCompleteDialog;
}

bool Quest::showFailDialog() const {
  return m_displayParameters.showFailDialog;
}

bool Quest::mainQuest() const {
  return m_displayParameters.mainQuest;
}

bool Quest::hideCrossServer() const {
  return m_displayParameters.hideCrossServer;
}

void Quest::setState(QuestState state) {
  m_state = state;
  m_lastUpdatedOn = Time::monotonicMilliseconds();
}

void Quest::initScript() {
  if (!m_player || !m_world || m_inited)
    return;

  auto questTemplate = getTemplate();
  if (questTemplate->script.has_value())
    m_scriptComponent.setScript(*questTemplate->script);
  else
    m_scriptComponent.setScripts(StringList{});
  m_scriptComponent.setUpdateDelta(questTemplate->updateDelta);

  m_scriptComponent.addCallbacks("quest", makeQuestCallbacks(m_player));
  m_scriptComponent.addCallbacks("celestial", LuaBindings::makeCelestialCallbacks(m_root, m_client));
  m_scriptComponent.addCallbacks("player", LuaBindings::makePlayerCallbacks(m_root, m_player));
  m_scriptComponent.addCallbacks("config", LuaBindings::makeConfigCallbacks([this](String const& name, Json const& def) {
      return Json(getTemplate()->scriptConfig).query(name, def);
    }));
  m_scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(m_player));
  m_scriptComponent.addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(m_player->statusController()));
  m_scriptComponent.addActorMovementCallbacks(m_player->movementController());
  m_inited = true;

  m_scriptComponent.init(m_world);
}

void Quest::uninitScript() {
  m_scriptComponent.uninit();
  m_scriptComponent.removeCallbacks("quest");
  m_scriptComponent.removeCallbacks("celestial");
  m_scriptComponent.removeCallbacks("player");
  m_scriptComponent.removeCallbacks("config");
  m_scriptComponent.removeCallbacks("entity");
  m_scriptComponent.removeCallbacks("status");
  m_scriptComponent.removeActorMovementCallbacks();
  m_inited = false;
}

LuaCallbacks Quest::makeQuestCallbacks(Player* player) {
  LuaCallbacks callbacks;

  callbacks.registerCallback("context", [this]() { m_scriptComponent.context(); });

  callbacks.registerCallback("state", [this]() { return QuestStateNames.getRight(state()); });

  callbacks.registerCallback("complete", [this](std::optional<std::size_t> followup) { complete(followup); });

  callbacks.registerCallback("fail", [this]() { fail(); });

  callbacks.registerCallback("abandon", [this]() { abandon(); });

  callbacks.registerCallback("decline", [this]() { declineOffer(); });

  callbacks.registerCallback("cancel", [this]() { cancelOffer(); });

  callbacks.registerCallback("setCanTurnIn", [this](bool value) { this->m_canTurnIn = value; });

  callbacks.registerCallback("questId", [this]() { return questId(); });

  callbacks.registerCallback("templateId", [this]() { return templateId(); });

  callbacks.registerCallback("seed", [this]() { return questDescriptor().seed; });

  callbacks.registerCallback("questDescriptor", [this]() { return questDescriptor().toJson(); });

  callbacks.registerCallback("questArcDescriptor", [this]() { return questArcDescriptor().toJson(); });

  callbacks.registerCallback("questArcPosition", [this]() { return m_arcPos; });

  callbacks.registerCallback("worldId", [this]() { return worldId().transform(printWorldId); });

  callbacks.registerCallback("setWorldId", [this](std::optional<String> const& worldId) { setWorldId(worldId.transform(parseWorldId)); });

  callbacks.registerCallback("serverUuid", [this]() { return serverUuid().transform(std::mem_fn(&Uuid::hex)); });

  callbacks.registerCallback("setServerUuid", [this](String const& serverUuid) { setServerUuid(Uuid(serverUuid)); });

  callbacks.registerCallback("isCurrent", [this, player]() -> bool { return player->questManager()->isCurrent(questId()); });

  callbacks.registerCallback("location", [this]() -> Json {
      if (auto loc = location()) {
        return JsonObject{
          {"system", jsonFromVec3I(loc.value().first)},
          {"location", jsonFromSystemLocation(loc.value().second)}
        };
      }
      return {};
    });

  callbacks.registerCallback("setLocation", [this](Json const& json) {
      if (json.isNull()) {
        setLocation({});
      } else {
        Vec3I system = jsonToVec3I(json.get("system"));
        SystemLocation location = jsonToSystemLocation(json.opt("location").value_or(Json{}));
        setLocation(std::make_pair(system, location));
      }
    });

  callbacks.registerCallback("parameters", [this]() { return questParamsToJson(parameters()); });

  callbacks.registerCallback("setParameter",
      [this](String const& name, Json paramJson) { m_parameters[name] = QuestParam::fromJson(paramJson); });

  callbacks.registerCallback(
      "setIndicators", [this](StringList indicators) { m_indicators = StringSet::from(indicators); });

  callbacks.registerCallbackWithSignature<void, std::optional<JsonArray>>("setObjectiveList", std::bind(&Quest::setObjectiveList, this, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, std::optional<float>>("setProgress", std::bind(&Quest::setProgress, this, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, std::optional<float>>("setCompassDirection", std::bind(&Quest::setCompassDirection, this, std::placeholders::_1));

  callbacks.registerCallbackWithSignature<void, String>("setTitle", [this](String const& title) {
      m_title = title;
    });
  callbacks.registerCallbackWithSignature<void, String>("setText", [this](String const& text) {
      m_text = text;
    });
  callbacks.registerCallbackWithSignature<void, String>("setCompletionText", [this](String const& completionText) {
      m_completionText = completionText;
    });
  callbacks.registerCallbackWithSignature<void, String>("setFailureText", [this](String const& failureText) {
      m_failureText = failureText;
    });

  callbacks.registerCallbackWithSignature<void, String, std::optional<JsonArray>>("setPortrait", [this](String const& portraitName, std::optional<JsonArray> const& portrait) {
      if (portrait) {
        m_portraits[portraitName] = portrait->transformed([this](Json const& drawable) { return Drawable(*m_root.imageMetadataDatabase(), drawable); });
      } else {
        m_portraits.remove(portraitName);
      }
    });
  callbacks.registerCallbackWithSignature<void, String, std::optional<String>>("setPortraitTitle", [this](String const& portraitName, std::optional<String> const& portrait) {
      if (portrait) {
        m_portraitTitles[portraitName] = *portrait;
      } else {
        m_portraitTitles.remove(portraitName);
      }
    });
  callbacks.registerCallbackWithSignature<void, Json>("addReward", [this](Json const& reward) {
      addReward(ItemDescriptor(reward));
    });

  return callbacks;
}

void Quest::setEntityParameter(String const& paramName, Entity const* entity) {
  std::optional<Json> portrait = {};
  std::optional<String> name = {};
  std::optional<String> species = {};
  std::optional<Gender> gender = {};

  if (auto portraitEntity = as<PortraitEntity>(entity)) {
    portrait = Json{portraitEntity->portrait(PortraitMode::Full).transformed(std::mem_fn(&Drawable::toJson))};
    name = portraitEntity->name();
  }

  if (auto npc = as<Npc>(entity)) {
    species = npc->species();
    gender = npc->gender();
  } else if (auto player = as<Player>(entity)) {
    species = player->species();
    gender = player->gender();
  }

  m_parameters[paramName] = QuestParam{QuestEntity{entity->uniqueId(), species, gender}, name, portrait, {}};
}

void Quest::addReward(ItemDescriptor const& reward) {
  if (reward.name() == "money") {
    m_money += reward.count();
    return;
  }
  auto itemDatabase = m_root.itemDatabase();
  m_rewards.append(itemDatabase->item(reward));
}

String const& Quest::defaultCustomIndicator() const {
  return m_player->questManager()->isCurrent(questId()) ? m_trackedIndicator : m_untrackedIndicator;
}

std::shared_ptr<Quest> createPreviewQuest(
    Root& root, String const& templateId, String const& position, String const& questGiverSpecies, Player* player) {
  auto questTemplates = root.questTemplateDatabase();
  auto questTemplate = questTemplates->questTemplate(templateId);
  if (!questTemplate)
    return {};

  Json portrait = player->portrait(PortraitMode::Full).transformed(std::mem_fn(&Drawable::toJson));

  JsonObject paramJson = questTemplate->parameterExamples;
  for (auto const& paramName : paramJson.keys()) {
    paramJson[paramName] = paramJson[paramName].set("type", questTemplate->parameterTypes[paramName]);
    paramJson[paramName] = paramJson[paramName].set("portrait", portrait);
  }
  StringMap<QuestParam> parameters = questParamsFromJson(paramJson);
  QuestDescriptor questDesc = QuestDescriptor{"preview", templateId, parameters, root.random().randu64()};

  List<QuestDescriptor> quests;
  if (!position.equalsIgnoreCase("next") && !position.equalsIgnoreCase("last") && !position.equalsIgnoreCase("first")) {
    quests = {questDesc};
  } else {
    quests = {questDesc, questDesc, questDesc};
  }
  QuestArcDescriptor arc = QuestArcDescriptor{quests, {}};

  std::size_t arcPos = 0;
  if (position.equalsIgnoreCase("next")) {
    arcPos = 1;
  } else if (position.equalsIgnoreCase("last")) {
    arcPos = 2;
  }

  auto quest = std::make_shared<Quest>(root, arc, arcPos, player);
  quest->setParameter("questGiver", QuestParam{QuestEntity{{}, questGiverSpecies, {}}, {"Quest Giver"}, portrait, {}});
  return quest;
}

}
