#pragma once

#include "StarConfig.hpp"
#include "StarQuests.hpp"

import std;

namespace Star {

struct QuestIndicator {
  String indicatorImage;
  Vec2F worldPosition;
};

class QuestManager {
public:
  QuestManager(Player* player);

  void diskLoad(Json const& quests);
  auto diskStore() -> Json;

  void setUniverseClient(UniverseClient* client);

  void init(World* world);
  void uninit();

  [[nodiscard]] auto canStart(QuestArcDescriptor const& questArc) const -> bool;

  // Show a dialog offering the player a quest, and later start it if they
  // accept it.
  void offer(Ptr<Quest> const& quest);
  [[nodiscard]] auto quests() const -> StringMap<Ptr<Quest>>;
  // Only returns quests that are exclusive to the current server.
  [[nodiscard]] auto serverQuests() const -> StringMap<Ptr<Quest>>;
  [[nodiscard]] auto getQuest(String const& questId) const -> Ptr<Quest>;

  [[nodiscard]] auto hasQuest(String const& questId) const -> bool;
  [[nodiscard]] auto hasAcceptedQuest(String const& questId) const -> bool;
  [[nodiscard]] auto isActive(String const& questId) const -> bool;
  [[nodiscard]] auto isCurrent(String const& questId) const -> bool;
  [[nodiscard]] auto isTracked(String const& questId) const -> bool;
  void setAsTracked(std::optional<String> const& questId);
  void markAsRead(String const& questId);
  [[nodiscard]] auto hasCompleted(String const& questId) const -> bool;
  [[nodiscard]] auto canTurnIn(String const& questId) const -> bool;

  auto getFirstNewQuest() -> std::optional<Ptr<Quest>>;
  auto getFirstCompletableQuest() -> std::optional<Ptr<Quest>>;
  auto getFirstFailableQuest() -> std::optional<Ptr<Quest>>;
  auto getFirstMainQuest() -> std::optional<Ptr<Quest>>;

  [[nodiscard]] auto listActiveQuests() const -> List<Ptr<Quest>>;
  [[nodiscard]] auto listCompletedQuests() const -> List<Ptr<Quest>>;
  [[nodiscard]] auto listFailedQuests() const -> List<Ptr<Quest>>;

  [[nodiscard]] auto currentQuestId() const -> std::optional<String>;
  [[nodiscard]] auto currentQuest() const -> std::optional<Ptr<Quest>>;
  [[nodiscard]] auto trackedQuestId() const -> std::optional<String>;
  [[nodiscard]] auto trackedQuest() const -> std::optional<Ptr<Quest>>;
  [[nodiscard]] auto getQuestIndicator(Ptr<Entity> const& entity) const -> std::optional<QuestIndicator>;

  // Handled at this level to allow multiple active quests to specify interestingObjects
  auto interestingObjects() -> StringSet;

  auto receiveMessage(String const& message, bool localMessage, JsonArray const& args = {}) -> std::optional<Json>;
  void update(float dt);

private:
  void startInitialQuests();
  void setMostRecentQuestCurrent();
  [[nodiscard]] auto questValidOnServer(Ptr<Quest> quest) const -> bool;

  Player* m_player;
  World* m_world;
  UniverseClient* m_client;

  StringMap<Ptr<Quest>> m_quests;

  std::optional<String> m_trackedQuestId;
  bool m_trackOnWorldQuests;
  std::optional<String> m_onWorldQuestId;
};

}
