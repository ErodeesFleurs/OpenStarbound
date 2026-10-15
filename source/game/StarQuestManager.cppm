module;
import std;
import star.string;
import star.list;
import star.maybe;
import star.json;

import star.set;
import star.game_types;
#include "StarStrongTypedef.hpp"
import star.data_stream;
import star.bi_map;
import star.item_descriptor;
import star.vector;
import star.celestial_coordinate;
import star.quest_descriptor;

namespace Star {

class Quest;
class Player;
class World;
class UniverseClient;
class Entity;

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
  void offer(std::shared_ptr<Quest> const& quest);
  StringMap<std::shared_ptr<Quest>> quests() const;
  // Only returns quests that are exclusive to the current server.
  StringMap<std::shared_ptr<Quest>> serverQuests() const;
  std::shared_ptr<Quest> getQuest(String const& questId) const;

  bool hasQuest(String const& questId) const;
  bool hasAcceptedQuest(String const& questId) const;
  bool isActive(String const& questId) const;
  bool isCurrent(String const& questId) const;
  bool isTracked(String const& questId) const;
  void setAsTracked(Maybe<String> const& questId);
  void markAsRead(String const& questId);
  bool hasCompleted(String const& questId) const;
  bool canTurnIn(String const& questId) const;

  Maybe<std::shared_ptr<Quest>> getFirstNewQuest();
  Maybe<std::shared_ptr<Quest>> getFirstCompletableQuest();
  Maybe<std::shared_ptr<Quest>> getFirstFailableQuest();
  Maybe<std::shared_ptr<Quest>> getFirstMainQuest();

  List<std::shared_ptr<Quest>> listActiveQuests() const;
  List<std::shared_ptr<Quest>> listCompletedQuests() const;
  List<std::shared_ptr<Quest>> listFailedQuests() const;

  Maybe<String> currentQuestId() const;
  Maybe<std::shared_ptr<Quest>> currentQuest() const;
  Maybe<String> trackedQuestId() const;
  Maybe<std::shared_ptr<Quest>> trackedQuest() const;
  Maybe<QuestIndicator> getQuestIndicator(std::shared_ptr<Entity> const& entity) const;

  // Handled at this level to allow multiple active quests to specify interestingObjects
  StringSet interestingObjects();

  Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});
  void update(float dt);

private:
  void startInitialQuests();
  void setMostRecentQuestCurrent();
  bool questValidOnServer(std::shared_ptr<Quest> quest) const;

  Player* m_player;
  World* m_world;
  UniverseClient* m_client;

  StringMap<std::shared_ptr<Quest>> m_quests;

  Maybe<String> m_trackedQuestId;
  bool m_trackOnWorldQuests;
  Maybe<String> m_onWorldQuestId;
};

}

export module star.quest_manager;

export namespace Star {
using ::Star::Quest;
using ::Star::Player;
using ::Star::World;
using ::Star::UniverseClient;
using ::Star::Entity;
using ::Star::QuestManager;
using ::Star::QuestIndicator;
}
