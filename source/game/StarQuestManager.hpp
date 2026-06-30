#pragma once

#include "StarAssets.hpp"
#include "StarQuests.hpp"

namespace Star {

class QuestManager;
using QuestManagerPtr = SharedPtr<QuestManager>;

struct QuestIndicator {
  String indicatorImage;
  Vec2F worldPosition;
};

class QuestManager {
public:
  QuestManager(AssetsConstPtr assets, Player& player, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase);

  QuestManager(AssetsConstPtr assets, Player& player, World& world, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase);

  void diskLoad(Json const& quests);
  [[nodiscard]] Json diskStore();

  void setUniverseClient(UniverseClient* client);
  [[nodiscard]] AssetsConstPtr assets() const;
  [[nodiscard]] ItemDatabaseConstPtr itemDatabase() const;
  [[nodiscard]] ObjectDatabaseConstPtr objectDatabase() const;
  [[nodiscard]] QuestTemplateDatabaseConstPtr questTemplateDatabase() const;
  [[nodiscard]] VersioningDatabaseConstPtr versioningDatabase() const;

  void init(World& world);
  void uninit();

  [[nodiscard]] bool canStart(QuestArcDescriptor const& questArc) const;

  // Show a dialog offering the player a quest, and later start it if they
  // accept it.
  void offer(QuestPtr const& quest);
  [[nodiscard]] StringMap<QuestPtr> quests() const;
  // Only returns quests that are exclusive to the current server.
  [[nodiscard]] StringMap<QuestPtr> serverQuests() const;
  [[nodiscard]] QuestPtr getQuest(String const& questId) const;

  [[nodiscard]] bool hasQuest(String const& questId) const;
  [[nodiscard]] bool hasAcceptedQuest(String const& questId) const;
  [[nodiscard]] bool isActive(String const& questId) const;
  [[nodiscard]] bool isCurrent(String const& questId) const;
  [[nodiscard]] bool isTracked(String const& questId) const;
  void setAsTracked(Maybe<String> const& questId);
  void markAsRead(String const& questId);
  [[nodiscard]] bool hasCompleted(String const& questId) const;
  [[nodiscard]] bool canTurnIn(String const& questId) const;

  [[nodiscard]] Maybe<QuestPtr> getFirstNewQuest();
  [[nodiscard]] Maybe<QuestPtr> getFirstCompletableQuest();
  [[nodiscard]] Maybe<QuestPtr> getFirstFailableQuest();
  [[nodiscard]] Maybe<QuestPtr> getFirstMainQuest();

  [[nodiscard]] List<QuestPtr> listActiveQuests() const;
  [[nodiscard]] List<QuestPtr> listCompletedQuests() const;
  [[nodiscard]] List<QuestPtr> listFailedQuests() const;

  [[nodiscard]] Maybe<String> currentQuestId() const;
  [[nodiscard]] Maybe<QuestPtr> currentQuest() const;
  [[nodiscard]] Maybe<String> trackedQuestId() const;
  [[nodiscard]] Maybe<QuestPtr> trackedQuest() const;
  [[nodiscard]] Maybe<QuestIndicator> getQuestIndicator(EntityPtr const& entity) const;

  // Handled at this level to allow multiple active quests to specify interestingObjects
  [[nodiscard]] StringSet interestingObjects();

  [[nodiscard]] Maybe<Json> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});
  void update(float dt);

private:
  void startInitialQuests();
  void setMostRecentQuestCurrent();
  [[nodiscard]] bool questValidOnServer(QuestPtr quest) const;

  Player* m_player;
  World* m_world;
  UniverseClient* m_client;
  AssetsConstPtr m_assets;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  QuestTemplateDatabaseConstPtr m_questTemplateDatabase;
  VersioningDatabaseConstPtr m_versioningDatabase;

  StringMap<QuestPtr> m_quests;

  Maybe<String> m_trackedQuestId;
  bool m_trackOnWorldQuests;
  Maybe<String> m_onWorldQuestId;
};

}
