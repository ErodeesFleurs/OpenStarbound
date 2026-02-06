#pragma once

#include <optional>

#include "StarQuests.hpp"

namespace Star {

STAR_CLASS(QuestManager);

struct QuestIndicator {
  String indicatorImage;
  Vec2F worldPosition;
};

class QuestManager {
public:
  QuestManager(Player* player);

  void diskLoad(Json const& quests);
  Json diskStore();

  void setUniverseClient(UniverseClient* client);

  void init(World* world);
  void uninit();

  bool canStart(QuestArcDescriptor const& questArc) const;

  // Show a dialog offering the player a quest, and later start it if they
  // accept it.
  void offer(QuestPtr const& quest);
  StringMap<QuestPtr> quests() const;
  // Only returns quests that are exclusive to the current server.
  StringMap<QuestPtr> serverQuests() const;
  QuestPtr getQuest(String const& questId) const;

  bool hasQuest(String const& questId) const;
  bool hasAcceptedQuest(String const& questId) const;
  bool isActive(String const& questId) const;
  bool isCurrent(String const& questId) const;
  bool isTracked(String const& questId) const;
  void setAsTracked(std::optional<String> const& questId);
  void markAsRead(String const& questId);
  bool hasCompleted(String const& questId) const;
  bool canTurnIn(String const& questId) const;

  std::optional<QuestPtr> getFirstNewQuest();
  std::optional<QuestPtr> getFirstCompletableQuest();
  std::optional<QuestPtr> getFirstFailableQuest();
  std::optional<QuestPtr> getFirstMainQuest();

  List<QuestPtr> listActiveQuests() const;
  List<QuestPtr> listCompletedQuests() const;
  List<QuestPtr> listFailedQuests() const;

  std::optional<String> currentQuestId() const;
  std::optional<QuestPtr> currentQuest() const;
  std::optional<String> trackedQuestId() const;
  std::optional<QuestPtr> trackedQuest() const;
  std::optional<QuestIndicator> getQuestIndicator(EntityPtr const& entity) const;

  // Handled at this level to allow multiple active quests to specify interestingObjects
  StringSet interestingObjects();

  std::optional<Json> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});
  void update(float dt);

private:
  void startInitialQuests();
  void setMostRecentQuestCurrent();
  bool questValidOnServer(QuestPtr quest) const;

  Player* m_player;
  World* m_world;
  UniverseClient* m_client;

  StringMap<QuestPtr> m_quests;

  std::optional<String> m_trackedQuestId;
  bool m_trackOnWorldQuests;
  std::optional<String> m_onWorldQuestId;
};

}
