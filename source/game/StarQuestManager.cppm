module;

#include "StarSet.hpp"
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarJson.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
import star.item_descriptor;
#include "StarVector.hpp"
import star.celestial_coordinate;
import star.quest_descriptor;

namespace Star {

STAR_CLASS(Quest);
STAR_CLASS(Player);
STAR_CLASS(World);
STAR_CLASS(UniverseClient);
STAR_CLASS(Entity);

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
