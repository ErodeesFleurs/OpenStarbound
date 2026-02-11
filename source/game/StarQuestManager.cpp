#include "StarQuestManager.hpp"

#include "StarCasting.hpp"
#include "StarClientContext.hpp"// IWYU pragma: export
#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerInventory.hpp"// IWYU pragma: export
#include "StarRoot.hpp"
#include "StarUniverseClient.hpp"

import std;

namespace Star {

QuestManager::QuestManager(Player* player) {
  m_player = player;
  m_world = nullptr;
  m_trackOnWorldQuests = false;
}

auto getTemplate(String const& templateId) -> Ptr<QuestTemplate> {
  return Root::singleton().questTemplateDatabase()->questTemplate(templateId);
}

auto readQuests(Json const& json) -> StringMap<Ptr<Quest>> {
  auto versioningDatabase = Root::singleton().versioningDatabase();
  auto questTemplateDatabase = Root::singleton().questTemplateDatabase();

  auto validateArc = [questTemplateDatabase](QuestArcDescriptor const& arc) -> bool {
    for (auto quest : arc.quests) {
      if (!questTemplateDatabase->questTemplate(quest.templateId))
        return false;
    }
    return true;
  };

  StringMap<Ptr<Quest>> result;
  for (auto const& questPair : json.iterateObject()) {
    // don't load the quest unless all quests in the arc exist
    Json diskStore = versioningDatabase->loadVersionedJson(VersionedJson::fromJson(questPair.second), "Quest");
    auto questArc = QuestArcDescriptor::diskLoad(diskStore.get("arc"));
    if (validateArc(questArc))
      result[questPair.first] = std::make_shared<Quest>(questPair.second);
  }
  return result;
}

auto questFilter(QuestState state) -> std::function<bool(Ptr<Quest> const&)> {
  return [state](Ptr<Quest> const& quest) -> bool {
    return quest->state() == state;
  };
}

void QuestManager::diskLoad(Json const& quests) {
  m_quests = readQuests(quests.get("quests", JsonObject{}));
  m_trackedQuestId = quests.optString("currentQuest");
}

auto QuestManager::diskStore() -> Json {
  auto questPtrToJson = [](Ptr<Quest> const& quest) -> Json { return quest->diskStore(); };

  auto quests = m_quests.values();

  return JsonObject{
    {"quests", jsonFromMapV<StringMap<Ptr<Quest>>>(m_quests, questPtrToJson)},
    {"currentQuest", jsonFromMaybe(m_trackedQuestId)}};
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
  m_quests.values().exec([](Ptr<Quest> const& quest) -> void { quest->uninit(); });
  m_world = nullptr;
}

auto QuestManager::canStart(QuestArcDescriptor const& questArc) const -> bool {
  if (questArc.quests.size() == 0)
    return false;

  for (auto questDesc : questArc.quests) {
    auto questTemplate = getTemplate(questDesc.templateId);
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

void QuestManager::offer(Ptr<Quest> const& quest) {
  m_quests[quest->questId()] = quest;
  quest->init(m_player, m_world, m_client);
  quest->offer();
}

auto QuestManager::quests() const -> StringMap<Ptr<Quest>> {
  return m_quests;
}

auto QuestManager::serverQuests() const -> StringMap<Ptr<Quest>> {
  StringMap<Ptr<Quest>> filtered;
  for (auto& pair : m_quests) {
    Ptr<Quest> q = pair.second;
    if (!questValidOnServer(q))
      continue;
    filtered.insert(pair.first, q);
  }
  return filtered;
}

auto QuestManager::getQuest(String const& questId) const -> Ptr<Quest> {
  return m_quests.get(questId);
}

auto QuestManager::hasQuest(String const& questId) const -> bool {
  return m_quests.contains(questId);
}

auto QuestManager::hasAcceptedQuest(String const& questId) const -> bool {
  return m_quests.contains(questId) && m_quests.get(questId)->state() != QuestState::New && m_quests.get(questId)->state() != QuestState::Offer;
}

auto QuestManager::isActive(String const& questId) const -> bool {
  return m_quests.contains(questId) && m_quests.get(questId)->state() == QuestState::Active;
}

auto QuestManager::isCurrent(String const& questId) const -> bool {
  return (m_onWorldQuestId ? m_onWorldQuestId : m_trackedQuestId) == questId;
}

auto QuestManager::isTracked(String const& questId) const -> bool {
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

auto QuestManager::hasCompleted(String const& questId) const -> bool {
  if (auto quest = m_quests.maybe(questId))
    if ((*quest)->state() == QuestState::Complete)
      return true;
  return false;
}

auto QuestManager::canTurnIn(String const& questId) const -> bool {
  if (auto quest = m_quests.maybe(questId))
    if ((*quest)->state() == QuestState::Active && (*quest)->canTurnIn())
      return true;
  return false;
}

auto QuestManager::getFirstNewQuest() -> std::optional<Ptr<Quest>> {
  for (auto& q : m_quests) {
    if (questValidOnServer(q.second) && q.second->state() == QuestState::Offer)
      return q.second;
  }
  return {};
}

auto QuestManager::getFirstCompletableQuest() -> std::optional<Ptr<Quest>> {
  for (auto& q : m_quests) {
    if (questValidOnServer(q.second) && q.second->state() == QuestState::Complete && q.second->showDialog())
      return q.second;
  }
  return {};
}

auto QuestManager::getFirstFailableQuest() -> std::optional<Ptr<Quest>> {
  for (auto& q : m_quests) {
    if (questValidOnServer(q.second) && q.second->state() == QuestState::Failed && q.second->showDialog())
      return q.second;
  }
  return {};
}

auto QuestManager::getFirstMainQuest() -> std::optional<Ptr<Quest>> {
  for (auto& q : m_quests) {
    if (questValidOnServer(q.second) && q.second->state() == QuestState::Active && q.second->mainQuest())
      return q.second;
  }
  return {};
}

void sortQuests(List<Ptr<Quest>>& quests) {
  std::ranges::sort(quests,
                    [](Ptr<Quest> const& left, Ptr<Quest> const& right) -> bool {
                      std::int64_t leftUpdated = left->lastUpdatedOn();
                      std::int64_t rightUpdated = right->lastUpdatedOn();
                      String leftQuestId = left->templateId();
                      String rightQuestId = right->templateId();
                      return std::tie(leftUpdated, leftQuestId) < std::tie(rightUpdated, rightQuestId);
                    });
}

auto QuestManager::listActiveQuests() const -> List<Ptr<Quest>> {
  List<Ptr<Quest>> result = serverQuests().values();
  result.filter([&](Ptr<Quest> quest) -> bool {
    return quest->state() == QuestState::Active && quest->showInLog();
  });
  sortQuests(result);
  return result;
}

auto QuestManager::listCompletedQuests() const -> List<Ptr<Quest>> {
  List<Ptr<Quest>> result = serverQuests().values();
  result.filter([](Ptr<Quest> quest) -> bool {
    return quest->state() == QuestState::Complete && quest->showInLog();
  });
  sortQuests(result);
  return result;
}

auto QuestManager::listFailedQuests() const -> List<Ptr<Quest>> {
  List<Ptr<Quest>> result = serverQuests().values();
  result.filter([](Ptr<Quest> quest) -> bool {
    return quest->state() == QuestState::Failed && quest->showInLog();
  });
  sortQuests(result);
  return result;
}

auto QuestManager::currentQuestId() const -> std::optional<String> {
  return m_trackedQuestId;
}

auto QuestManager::currentQuest() const -> std::optional<Ptr<Quest>> {
  auto questId = m_onWorldQuestId ? m_onWorldQuestId : m_trackedQuestId;
  if (questId && isActive(*questId)) {
    auto current = getQuest(*questId);
    if (current->showInLog())
      return current;
  }
  return {};
}

auto QuestManager::trackedQuestId() const -> std::optional<String> {
  return m_trackedQuestId;
}

auto QuestManager::trackedQuest() const -> std::optional<Ptr<Quest>> {
  if (m_trackedQuestId && isActive(*m_trackedQuestId)) {
    auto current = getQuest(*m_trackedQuestId);
    if (current->showInLog())
      return current;
  }
  return {};
}

auto QuestManager::getQuestIndicator(Ptr<Entity> const& entity) const -> std::optional<QuestIndicator> {
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
      auto questTemplateDatabase = Root::singleton().questTemplateDatabase();
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
    Json indicators = Root::singleton().assets()->json("/quests/quests.config:indicators");
    String indicatorImage = indicators.get(*indicatorType).getString("image");
    if (questGiver)
      indicatorPos = questGiver->questIndicatorPosition();
    return QuestIndicator{.indicatorImage = indicatorImage, .worldPosition = indicatorPos};
  }

  for (auto& pair : m_quests) {
    if (pair.second->state() == QuestState::Active) {
      if (auto indicatorImage = pair.second->customIndicator(entity)) {
        if (questGiver)
          indicatorPos = questGiver->questIndicatorPosition();
        return QuestIndicator{.indicatorImage = *indicatorImage, .worldPosition = indicatorPos};
      }
    }
  }

  return {};
}

auto QuestManager::interestingObjects() -> StringSet {
  StringSet result;
  m_quests.values().exec([&result](Ptr<Quest> const& quest) -> void {
    if (auto questObjects = quest->receiveMessage("interestingObjects", true, JsonArray()))
      result.addAll(jsonToStringSet(*questObjects));
  });
  return result;
}

auto QuestManager::receiveMessage(String const& message, bool localMessage, JsonArray const& args) -> std::optional<Json> {
  std::optional<Json> result;
  m_quests.values().exec([&result, message, localMessage, args](Ptr<Quest> const& quest) -> void {
    auto r = quest->receiveMessage(message, localMessage, args);
    if (!result)
      result = std::move(r);
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
    auto trackedWorld = currentQuest().and_then([](Ptr<Quest> q) -> std::optional<WorldId> { return q->worldId(); });
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
    Root::singleton().assets()->json(strf("/quests/quests.config:initialquests.{}", m_player->species())).toArray();
  for (auto const& questArcJson : startingQuests) {
    QuestArcDescriptor quest = QuestArcDescriptor::fromJson(questArcJson);
    if (canStart(quest))
      offer(std::make_shared<Quest>(quest, 0, m_player));
  }
}

void QuestManager::setMostRecentQuestCurrent() {
  List<Ptr<Quest>> sortedActiveQuests = listActiveQuests();
  if (sortedActiveQuests.size() > 0)
    setAsTracked(sortedActiveQuests.last()->questId());
}

auto QuestManager::questValidOnServer(Ptr<Quest> q) const -> bool {
  return !(q->hideCrossServer() && q->serverUuid().has_value() && *q->serverUuid() != m_player->clientContext()->serverUuid());
}

}// namespace Star
