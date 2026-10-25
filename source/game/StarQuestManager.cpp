import std;
import star.list;
import star.vector;
import star.game_types;
import star.json;
import star.json_extra;
import star.casting;
import star.string;
import star.format;
import star.quests;
import star.quest_descriptor;
import star.quest_manager;
import star.quest_template_database;
import star.versioning_database;
import star.root;
import star.assets;
import star.player;
import star.player_inventory;
import star.client_context;
import star.entity;
import star.interactive_entity;
import star.universe_client;
import star.world;
namespace Star {
QuestManager::QuestManager(Root& root, Player& player)
  : m_root(root), m_player(player), m_trackOnWorldQuests(false) {}
std::expected<std::shared_ptr<QuestManager>, StarException> QuestManager::create(Root& root, Player& player) {
  return std::shared_ptr<QuestManager>(new QuestManager(root, player));
}
[[nodiscard]] std::expected<StringMap<std::shared_ptr<Quest>>, StarException> readQuests(Root& root, Json const& json) {
  auto versions = root.versioningDatabase();
  if (!versions) return std::unexpected(std::move(versions.error()));
  auto templates = root.questTemplateDatabase();
  if (!templates) return std::unexpected(std::move(templates.error()));
  auto entries = json.iterateObject();
  if (!entries) return std::unexpected(std::move(entries.error()));
  StringMap<std::shared_ptr<Quest>> result;
  for (auto const& entry : *entries) {
    auto versioned = VersionedJson::fromJson(entry.second);
    if (!versioned) return std::unexpected(std::move(versioned.error()));
    auto store = (*versions)->loadVersionedJson(*versioned, "Quest");
    if (!store) return std::unexpected(std::move(store.error()));
    auto arcJson = store->get("arc");
    if (!arcJson) return std::unexpected(std::move(arcJson.error()));
    auto arc = QuestArcDescriptor::diskLoad(root, *arcJson);
    if (!arc) return std::unexpected(std::move(arc.error()));
    bool valid = true;
    for (auto const& descriptor : arc->quests) {
      auto questTemplate = (*templates)->questTemplate(descriptor.templateId);
      if (!questTemplate) return std::unexpected(std::move(questTemplate.error()));
      if (!*questTemplate) { valid = false; break; }
    }
    if (valid) {
      auto quest = Quest::create(root, entry.second);
      if (!quest) return std::unexpected(std::move(quest.error()));
      result[entry.first] = *quest;
    }
  }
  return result;
}
std::expected<void, StarException> QuestManager::diskLoad(Json const& quests) {
  auto json = quests.get("quests", JsonObject{});
  if (!json) return std::unexpected(std::move(json.error()));
  auto loaded = readQuests(m_root, *json);
  if (!loaded) return std::unexpected(std::move(loaded.error()));
  m_quests = std::move(*loaded);
  auto current = quests.optString("currentQuest");
  if (!current) return std::unexpected(std::move(current.error()));
  m_trackedQuestId = std::move(*current);
  return {};
}
std::expected<Json, StarException> QuestManager::diskStore() {
  auto quests = jsonFromMapV(m_quests, [](std::shared_ptr<Quest> const& quest) { return quest->diskStore(); });
  if (!quests) return std::unexpected(std::move(quests.error()));
  return JsonObject{{"quests", *quests}, {"currentQuest", jsonFromOptional(m_trackedQuestId)}};
}
void QuestManager::setUniverseClient(std::optional<std::reference_wrapper<UniverseClient>> client) { m_client = client; }
std::expected<void, StarException> QuestManager::init(Player& player, World& world) {
  m_player = player;
  m_world = world;
  m_client = player.universeClient();
  for (auto const& quest : m_quests.values()) {
    if (!questValidOnServer(quest)) continue;
    auto initialized = quest->init(player, world, m_client);
    if (!initialized) {
      auto error = std::move(initialized.error());
      auto shutdown = uninit();
      if (!shutdown) error = error.withContext(strf("Error rolling back quest manager: {}", outputException(shutdown.error(), true)));
      return std::unexpected(std::move(error));
    }
  }
  m_trackOnWorldQuests = true;
  if (m_trackedQuestId) {
    if (auto quest = m_quests.maybeRef(*m_trackedQuestId)) {
      if (quest->get()->hideCrossServer() && quest->get()->serverUuid() && *quest->get()->serverUuid() != m_player->get().clientContext()->serverUuid()) m_trackedQuestId.reset();
    }
  }
  return {};
}
std::expected<void, StarException> QuestManager::uninit() {
  std::optional<StarException> error;
  for (auto const& quest : m_quests.values()) {
    auto result = quest->uninit();
    if (!result && !error) error = std::move(result.error());
  }
  detachPlayer();
  if (error) return std::unexpected(std::move(*error));
  return {};
}
void QuestManager::detachPlayer() {
  for (auto const& quest : m_quests.values())
    quest->detachPlayer();
  m_world.reset();
  m_player.reset();
  m_client.reset();
  m_trackOnWorldQuests = false;
}
std::expected<bool, StarException> QuestManager::canStart(QuestArcDescriptor const& arc) const {
  if (!m_player || arc.quests.empty()) return false;
  for (auto const& descriptor : arc.quests) {
    auto database = m_root.questTemplateDatabase();
    if (!database) return std::unexpected(std::move(database.error()));
    auto config = (*database)->questTemplate(descriptor.templateId);
    if (!config) return std::unexpected(std::move(config.error()));
    if (!*config) return false;
    if (auto quest = m_quests.maybeRef(descriptor.questId)) if (quest->get()->state() != QuestState::Failed) return false;
    for (auto const& prerequisite : (*config)->prerequisiteQuests) if (!hasCompleted(prerequisite)) return false;
    for (auto const& item : (*config)->requiredItems) if (!m_player->get().inventory()->hasItem(item)) return false;
    if ((*config)->requiredShipLevel && m_player->get().shipUpgrades().shipLevel < *(*config)->requiredShipLevel) return false;
  }
  return true;
}
std::expected<void, StarException> QuestManager::offer(std::shared_ptr<Quest> const& quest) {
  if (!m_player || !m_world)
    return std::unexpected(StarException("Cannot offer a quest without an attached player world"));
  m_quests[quest->questId()] = quest;
  auto initialized = quest->init(m_player->get(), m_world->get(), m_client);
  if (!initialized) return std::unexpected(std::move(initialized.error()));
  return quest->offer();
}
StringMap<std::shared_ptr<Quest>> QuestManager::quests() const { return m_quests; }
StringMap<std::shared_ptr<Quest>> QuestManager::serverQuests() const {
  StringMap<std::shared_ptr<Quest>> result;
  for (auto const& p : m_quests) if (questValidOnServer(p.second)) result.insert(p.first, p.second);
  return result;
}
std::expected<std::shared_ptr<Quest>, StarException> QuestManager::getQuest(String const& id) const {
  auto result = m_quests.get(id);
  if (!result) return std::unexpected(std::move(result.error()));
  return result->get();
}
bool QuestManager::hasQuest(String const& id) const { return m_quests.contains(id); }
bool QuestManager::hasAcceptedQuest(String const& id) const {
  auto quest = m_quests.maybeRef(id);
  return quest && quest->get()->state() != QuestState::New && quest->get()->state() != QuestState::Offer;
}
bool QuestManager::isActive(String const& id) const {
  auto quest = m_quests.maybeRef(id);
  return quest && quest->get()->state() == QuestState::Active;
}
bool QuestManager::isCurrent(String const& id) const { return (m_onWorldQuestId ? m_onWorldQuestId : m_trackedQuestId) == id; }
bool QuestManager::isTracked(String const& id) const { return m_trackedQuestId == id; }
void QuestManager::setAsTracked(std::optional<String> const& id) {
  if (id && isActive(*id)) {
    m_trackedQuestId = id;
    if (m_onWorldQuestId) { m_onWorldQuestId.reset(); m_trackOnWorldQuests = false; }
  } else { m_trackedQuestId.reset(); m_trackOnWorldQuests = true; }
}
void QuestManager::markAsRead(String const& id) { if (auto quest = m_quests.maybeRef(id)) quest->get()->markAsRead(); }
bool QuestManager::hasCompleted(String const& id) const {
  auto quest = m_quests.maybeRef(id);
  return quest && quest->get()->state() == QuestState::Complete;
}
bool QuestManager::canTurnIn(String const& id) const {
  auto quest = m_quests.maybeRef(id);
  return quest && quest->get()->state() == QuestState::Active && quest->get()->canTurnIn();
}
std::optional<std::shared_ptr<Quest>> QuestManager::getFirstNewQuest() {
  for (auto const& q : m_quests) if (questValidOnServer(q.second) && q.second->state() == QuestState::Offer) return q.second;
  return {};
}
std::optional<std::shared_ptr<Quest>> QuestManager::getFirstCompletableQuest() {
  for (auto const& q : m_quests) if (questValidOnServer(q.second) && q.second->state() == QuestState::Complete && q.second->showDialog()) return q.second;
  return {};
}
std::optional<std::shared_ptr<Quest>> QuestManager::getFirstFailableQuest() {
  for (auto const& q : m_quests) if (questValidOnServer(q.second) && q.second->state() == QuestState::Failed && q.second->showDialog()) return q.second;
  return {};
}
std::optional<std::shared_ptr<Quest>> QuestManager::getFirstMainQuest() {
  for (auto const& q : m_quests) if (questValidOnServer(q.second) && q.second->state() == QuestState::Active && q.second->mainQuest()) return q.second;
  return {};
}
void sortQuests(List<std::shared_ptr<Quest>>& quests) {
  std::sort(quests.begin(), quests.end(), [](auto const& left, auto const& right) {
    return std::tuple(left->lastUpdatedOn(), left->templateId()) < std::tuple(right->lastUpdatedOn(), right->templateId());
  });
}
List<std::shared_ptr<Quest>> QuestManager::listActiveQuests() const {
  auto result = serverQuests().values();
  result.filter([](auto const& quest) { return quest->state() == QuestState::Active && quest->showInLog(); });
  sortQuests(result);
  return result;
}
List<std::shared_ptr<Quest>> QuestManager::listCompletedQuests() const {
  auto result = serverQuests().values();
  result.filter([](auto const& quest) { return quest->state() == QuestState::Complete && quest->showInLog(); });
  sortQuests(result);
  return result;
}
List<std::shared_ptr<Quest>> QuestManager::listFailedQuests() const {
  auto result = serverQuests().values();
  result.filter([](auto const& quest) { return quest->state() == QuestState::Failed && quest->showInLog(); });
  sortQuests(result);
  return result;
}
std::optional<String> QuestManager::currentQuestId() const { return m_trackedQuestId; }
std::optional<std::shared_ptr<Quest>> QuestManager::currentQuest() const {
  auto id = m_onWorldQuestId ? m_onWorldQuestId : m_trackedQuestId;
  if (id && isActive(*id)) {
    if (auto quest = m_quests.maybeRef(*id)) if (quest->get()->showInLog()) return quest->get();
  }
  return {};
}
std::optional<String> QuestManager::trackedQuestId() const { return m_trackedQuestId; }
std::optional<std::shared_ptr<Quest>> QuestManager::trackedQuest() const {
  if (m_trackedQuestId && isActive(*m_trackedQuestId)) {
    if (auto quest = m_quests.maybeRef(*m_trackedQuestId)) if (quest->get()->showInLog()) return quest->get();
  }
  return {};
}
std::expected<std::optional<QuestIndicator>, StarException> QuestManager::getQuestIndicator(std::shared_ptr<Entity> const& entity) const {
  std::optional<String> type;
  Vec2F position = entity->position() + Vec2F(0, 2.75);
  auto giver = as<InteractiveEntity>(entity);
  if (giver) {
    auto turnIns = giver->turnInQuests();
    if (!turnIns) return std::unexpected(std::move(turnIns.error()));
    for (auto const& id : *turnIns) {
      if (!isActive(id)) continue;
      auto quest = getQuest(id);
      if (!quest) return std::unexpected(std::move(quest.error()));
      if ((*quest)->canTurnIn()) {
        auto indicator = (*quest)->questReceiverIndicator();
        if (!indicator) return std::unexpected(std::move(indicator.error()));
        type = *indicator;
        break;
      }
    }
    if (!type) {
      auto templates = m_root.questTemplateDatabase();
      if (!templates) return std::unexpected(std::move(templates.error()));
      auto arcs = giver->offeredQuests();
      if (!arcs) return std::unexpected(std::move(arcs.error()));
      for (auto const& arc : *arcs) {
        auto start = canStart(arc);
        if (!start) return std::unexpected(std::move(start.error()));
        if (*start && !arc.quests.empty()) {
          auto config = (*templates)->questTemplate(arc.quests[0].templateId);
          if (!config) return std::unexpected(std::move(config.error()));
          if (!*config) return std::unexpected(StarException(strf("No quest template found for id '{}'", arc.quests[0].templateId)));
          type = (*config)->questGiverIndicator;
          break;
        }
      }
    }
  }
  if (type) {
    auto assets = m_root.assets();
    if (!assets) return std::unexpected(std::move(assets.error()));
    auto indicators = (*assets)->json("/quests/quests.config:indicators");
    if (!indicators) return std::unexpected(std::move(indicators.error()));
    auto indicator = indicators->get(*type);
    if (!indicator) return std::unexpected(std::move(indicator.error()));
    auto image = indicator->getString("image");
    if (!image) return std::unexpected(std::move(image.error()));
    if (giver) {
      auto indicatorPosition = giver->questIndicatorPosition();
      if (!indicatorPosition) return std::unexpected(std::move(indicatorPosition.error()));
      position = *indicatorPosition;
    }
    return std::optional<QuestIndicator>(QuestIndicator{*image, position});
  }
  for (auto const& pair : m_quests) {
    if (pair.second->state() != QuestState::Active) continue;
    auto image = pair.second->customIndicator(entity);
    if (!image) return std::unexpected(std::move(image.error()));
    if (*image) {
      if (giver) {
        auto indicatorPosition = giver->questIndicatorPosition();
        if (!indicatorPosition) return std::unexpected(std::move(indicatorPosition.error()));
        position = *indicatorPosition;
      }
      return std::optional<QuestIndicator>(QuestIndicator{**image, position});
    }
  }
  return std::optional<QuestIndicator>();
}
std::expected<StringSet, StarException> QuestManager::interestingObjects() {
  StringSet result;
  for (auto const& quest : m_quests.values()) {
    auto response = quest->receiveMessage("interestingObjects", true, JsonArray());
    if (!response) return std::unexpected(std::move(response.error()));
    if (*response) {
      if (std::holds_alternative<Json>(**response)) {
        auto objects = jsonToStringSet(std::get<Json>(**response));
        if (!objects) return std::unexpected(std::move(objects.error()));
        result.addAll(*objects);
      }
    }
  }
  return result;
}
std::expected<std::optional<ChainableJsonMessageResponse>, StarException> QuestManager::receiveMessage(String const& message, bool local, JsonArray const& args) {
  std::optional<ChainableJsonMessageResponse> result;
  for (auto const& quest : m_quests.values()) {
    auto response = quest->receiveMessage(message, local, args);
    if (!response) return std::unexpected(std::move(response.error()));
    if (!result) result = std::move(*response);
  }
  return result;
}
std::expected<void, StarException> QuestManager::update(float dt) {
  if (!m_player || !m_world) return {};
  auto initial = startInitialQuests();
  if (!initial) return std::unexpected(std::move(initial.error()));
  if (m_trackedQuestId && !isActive(*m_trackedQuestId)) m_trackedQuestId.reset();
  if (m_onWorldQuestId) {
    auto active = isActive(*m_onWorldQuestId);
    if (active) {
      auto quest = getQuest(*m_onWorldQuestId);
      if (!quest) return std::unexpected(std::move(quest.error()));
      auto world = (*quest)->worldId();
      if (!world || m_player->get().clientContext()->playerWorldId() != *world) active = false;
    }
    if (!active) m_onWorldQuestId.reset();
  } else if (m_trackOnWorldQuests && m_client) {
    auto playerWorld = m_client->get().clientContext()->playerWorldId();
    auto trackedWorld = currentQuest().and_then([](std::shared_ptr<Quest> const& quest) { return quest->worldId(); });
    if (!trackedWorld || playerWorld != *trackedWorld) {
      for (auto const& quest : listActiveQuests()) {
        if (auto world = quest->worldId()) if (playerWorld == *world) m_onWorldQuestId = quest->questId();
      }
    }
  }
  List<String> expired;
  for (auto const& entry : m_quests) {
    auto const& quest = entry.second;
    auto state = quest->state();
    bool finished = state == QuestState::Complete || state == QuestState::Failed;
    if (state == QuestState::New || (finished && quest->ephemeral() && !quest->showDialog())) {
      auto uninitialized = quest->uninit();
      if (!uninitialized) return std::unexpected(std::move(uninitialized.error()));
      expired.append(entry.first);
    }
  }
  for (auto const& id : expired) m_quests.remove(id);
  for (auto const& quest : m_quests.values()) {
    if (!questValidOnServer(quest)) continue;
    auto updated = quest->update(dt);
    if (!updated) return std::unexpected(std::move(updated.error()));
  }
  return {};
}
std::expected<void, StarException> QuestManager::startInitialQuests() {
  auto assets = m_root.assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto json = (*assets)->json(strf("/quests/quests.config:initialquests.{}", m_player->get().species()));
  if (!json) return std::unexpected(std::move(json.error()));
  auto initial = json->toArray();
  if (!initial) return std::unexpected(std::move(initial.error()));
  for (auto const& entry : *initial) {
    auto arc = QuestArcDescriptor::fromJson(m_root, entry);
    if (!arc) return std::unexpected(std::move(arc.error()));
    auto start = canStart(*arc);
    if (!start) return std::unexpected(std::move(start.error()));
    if (*start) {
      auto quest = Quest::create(m_root, *arc, 0, m_player->get());
      if (!quest) return std::unexpected(std::move(quest.error()));
      auto offered = offer(*quest);
      if (!offered) return std::unexpected(std::move(offered.error()));
    }
  }
  return {};
}
void QuestManager::setMostRecentQuestCurrent() {
  auto active = listActiveQuests();
  if (!active.empty()) setAsTracked(active[active.size() - 1]->questId());
}
bool QuestManager::questValidOnServer(std::shared_ptr<Quest> q) const {
  return !(q->hideCrossServer() && q->serverUuid()
      && (!m_player || *q->serverUuid() != m_player->get().clientContext()->serverUuid()));
}
}
