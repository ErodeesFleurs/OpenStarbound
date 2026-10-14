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
using QuestPtr = std::shared_ptr<Quest>;
using QuestConstPtr = std::shared_ptr<const Quest>;
using QuestWeakPtr = std::weak_ptr<Quest>;
using QuestConstWeakPtr = std::weak_ptr<const Quest>;
using QuestUPtr = std::unique_ptr<Quest>;
using QuestConstUPtr = std::unique_ptr<const Quest>;
class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<const Player>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<const Player>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<const Player>;
class World;
using WorldPtr = std::shared_ptr<World>;
using WorldConstPtr = std::shared_ptr<const World>;
using WorldWeakPtr = std::weak_ptr<World>;
using WorldConstWeakPtr = std::weak_ptr<const World>;
using WorldUPtr = std::unique_ptr<World>;
using WorldConstUPtr = std::unique_ptr<const World>;
class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<const UniverseClient>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<const UniverseClient>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<const UniverseClient>;
class Entity;
using EntityPtr = std::shared_ptr<Entity>;
using EntityConstPtr = std::shared_ptr<const Entity>;
using EntityWeakPtr = std::weak_ptr<Entity>;
using EntityConstWeakPtr = std::weak_ptr<const Entity>;
using EntityUPtr = std::unique_ptr<Entity>;
using EntityConstUPtr = std::unique_ptr<const Entity>;

class QuestManager;
using QuestManagerPtr = std::shared_ptr<QuestManager>;
using QuestManagerConstPtr = std::shared_ptr<const QuestManager>;
using QuestManagerWeakPtr = std::weak_ptr<QuestManager>;
using QuestManagerConstWeakPtr = std::weak_ptr<const QuestManager>;
using QuestManagerUPtr = std::unique_ptr<QuestManager>;
using QuestManagerConstUPtr = std::unique_ptr<const QuestManager>;

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
  void setAsTracked(Maybe<String> const& questId);
  void markAsRead(String const& questId);
  bool hasCompleted(String const& questId) const;
  bool canTurnIn(String const& questId) const;

  Maybe<QuestPtr> getFirstNewQuest();
  Maybe<QuestPtr> getFirstCompletableQuest();
  Maybe<QuestPtr> getFirstFailableQuest();
  Maybe<QuestPtr> getFirstMainQuest();

  List<QuestPtr> listActiveQuests() const;
  List<QuestPtr> listCompletedQuests() const;
  List<QuestPtr> listFailedQuests() const;

  Maybe<String> currentQuestId() const;
  Maybe<QuestPtr> currentQuest() const;
  Maybe<String> trackedQuestId() const;
  Maybe<QuestPtr> trackedQuest() const;
  Maybe<QuestIndicator> getQuestIndicator(EntityPtr const& entity) const;

  // Handled at this level to allow multiple active quests to specify interestingObjects
  StringSet interestingObjects();

  Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});
  void update(float dt);

private:
  void startInitialQuests();
  void setMostRecentQuestCurrent();
  bool questValidOnServer(QuestPtr quest) const;

  Player* m_player;
  World* m_world;
  UniverseClient* m_client;

  StringMap<QuestPtr> m_quests;

  Maybe<String> m_trackedQuestId;
  bool m_trackOnWorldQuests;
  Maybe<String> m_onWorldQuestId;
};

}

export module star.quest_manager;

export namespace Star {
using ::Star::Quest;
using ::Star::QuestPtr;
using ::Star::QuestConstPtr;
using ::Star::QuestWeakPtr;
using ::Star::QuestConstWeakPtr;
using ::Star::QuestUPtr;
using ::Star::QuestConstUPtr;
using ::Star::Player;
using ::Star::PlayerPtr;
using ::Star::PlayerConstPtr;
using ::Star::PlayerWeakPtr;
using ::Star::PlayerConstWeakPtr;
using ::Star::PlayerUPtr;
using ::Star::PlayerConstUPtr;
using ::Star::World;
using ::Star::WorldPtr;
using ::Star::WorldConstPtr;
using ::Star::WorldWeakPtr;
using ::Star::WorldConstWeakPtr;
using ::Star::WorldUPtr;
using ::Star::WorldConstUPtr;
using ::Star::UniverseClient;
using ::Star::UniverseClientPtr;
using ::Star::UniverseClientConstPtr;
using ::Star::UniverseClientWeakPtr;
using ::Star::UniverseClientConstWeakPtr;
using ::Star::UniverseClientUPtr;
using ::Star::UniverseClientConstUPtr;
using ::Star::Entity;
using ::Star::EntityPtr;
using ::Star::EntityConstPtr;
using ::Star::EntityWeakPtr;
using ::Star::EntityConstWeakPtr;
using ::Star::EntityUPtr;
using ::Star::EntityConstUPtr;
using ::Star::QuestManager;
using ::Star::QuestManagerPtr;
using ::Star::QuestManagerConstPtr;
using ::Star::QuestManagerWeakPtr;
using ::Star::QuestManagerConstWeakPtr;
using ::Star::QuestManagerUPtr;
using ::Star::QuestManagerConstUPtr;
using ::Star::QuestIndicator;
}
