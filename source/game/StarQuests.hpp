#pragma once

#include "StarSet.hpp"
#include "StarJsonRpc.hpp"
#include "StarItemDescriptor.hpp"
#include "StarDrawable.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarThread.hpp"
#include "StarQuestDescriptor.hpp"
#include "StarQuestTemplateDatabase.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarWarping.hpp"
#include "StarAssets.hpp"

namespace Star {

class Quest;
using QuestPtr = SharedPtr<Quest>;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class QuestTemplateDatabase;
using QuestTemplateDatabaseConstPtr = SharedPtr<QuestTemplateDatabase const>;
class VersioningDatabase;
using VersioningDatabaseConstPtr = SharedPtr<VersioningDatabase const>;
class Player;
class UniverseClient;

enum class QuestState {
  // New - being set up and quest hasn't been offered yet (or was offered and declined)
  New,
  // Offer - waiting on the player to accept or decline the quest
  Offer,
  // Active - the quest was accepted and is in progress
  Active,
  // Complete - the quest finished successfully
  Complete,
  // Failed - the quest finished unsuccessfully or the player abandoned it
  Failed
};
extern EnumMap<QuestState> const QuestStateNames;

class Quest {
public:
  Quest(AssetsConstPtr assets, QuestArcDescriptor const& questArc, size_t arcPos, Player& player, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase);

  Quest(AssetsConstPtr assets, Json const& diskStore, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase);
  [[nodiscard]] Json diskStore() const;

  [[nodiscard]] QuestTemplatePtr getTemplate() const;

  void init(Player& player, World& world, observer_ptr<UniverseClient> client);
  void uninit();

  [[nodiscard]] Maybe<Json> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});
  [[nodiscard]] Maybe<LuaValue> callScript(String const& func, LuaVariadic<LuaValue> const& args);
  void update(float dt);

  void offer();
  void declineOffer();
  void cancelOffer();
  void start();
  void complete(Maybe<size_t> followupIndex = {});
  void fail();
  void abandon();

  [[nodiscard]] bool interactWithEntity(EntityId entity);

  // The generated ID for this instance of the quest with these specific
  // parameters. Multiple players iin a universe may have quests with the same
  // questId if the the source of the quest was the same.
  [[nodiscard]] String questId() const;
  // The ID of the template this quest was created from
  [[nodiscard]] String templateId() const;

  [[nodiscard]] StringMap<QuestParam> const& parameters() const;

  [[nodiscard]] QuestState state() const;

  // Whether to show the Complete / Failed dialog
  [[nodiscard]] bool showDialog() const;
  void setDialogShown();

  void setEntityParameter(String const& paramName, EntityConstPtr const& entity);
  void setParameter(String const& paramName, QuestParam const& paramValue);

  [[nodiscard]] Maybe<List<Drawable>> portrait(String const& portraitName) const;
  [[nodiscard]] Maybe<String> portraitTitle(String const& portraitName) const;

  [[nodiscard]] QuestDescriptor questDescriptor() const;
  [[nodiscard]] QuestArcDescriptor questArcDescriptor() const;
  [[nodiscard]] size_t questArcPosition() const;

  [[nodiscard]] Maybe<WorldId> worldId() const;
  [[nodiscard]] Maybe<pair<Vec3I, SystemLocation>> location() const;
  [[nodiscard]] Maybe<Uuid> serverUuid() const;
  void setWorldId(Maybe<WorldId> worldId);
  void setLocation(Maybe<pair<Vec3I, SystemLocation>> location);
  void setServerUuid(Maybe<Uuid> serverUuid);

  [[nodiscard]] String title() const;
  [[nodiscard]] String text() const;
  [[nodiscard]] String completionText() const;
  [[nodiscard]] String failureText() const;

  [[nodiscard]] size_t money() const;
  [[nodiscard]] List<ItemConstPtr> rewards() const;

  // The time when this quest last changed state (active/completed/failed)
  [[nodiscard]] int64_t lastUpdatedOn() const;
  [[nodiscard]] bool unread() const;
  void markAsRead();
  [[nodiscard]] bool canTurnIn() const;

  [[nodiscard]] String questGiverIndicator() const;
  [[nodiscard]] String questReceiverIndicator() const;

  // The String returned by this method is an image path, not a reference to a configured indicator
  [[nodiscard]] Maybe<String> customIndicator(EntityPtr const& entity) const;

  [[nodiscard]] Maybe<JsonArray> objectiveList() const;
  [[nodiscard]] Maybe<float> progress() const;
  [[nodiscard]] Maybe<float> compassDirection() const;

  void setObjectiveList(Maybe<JsonArray> const& objectiveList);
  void setProgress(Maybe<float> const& progress);
  void setCompassDirection(Maybe<float> const& compassDirection);

  [[nodiscard]] Maybe<String> completionCinema() const;
  [[nodiscard]] bool canBeAbandoned() const;
  [[nodiscard]] bool ephemeral() const;
  [[nodiscard]] bool showInLog() const;
  [[nodiscard]] bool showAcceptDialog() const;
  [[nodiscard]] bool showCompleteDialog() const;
  [[nodiscard]] bool showFailDialog() const;
  [[nodiscard]] bool mainQuest() const;
  [[nodiscard]] bool hideCrossServer() const;

private:
  struct DisplayParameters {
    bool ephemeral;
    bool showInLog;
    bool showAcceptDialog;
    bool showCompleteDialog;
    bool showFailDialog;
    bool mainQuest;
    bool hideCrossServer;
  };

  void setState(QuestState state);

  void initScript();
  void uninitScript();
  [[nodiscard]] LuaCallbacks makeQuestCallbacks(Player& player);

  void setEntityParameter(String const& paramName, Entity const* entity);
  void addReward(ItemDescriptor const& reward);

  [[nodiscard]] String const& defaultCustomIndicator() const;

  observer_ptr<Player> m_player;
  observer_ptr<World> m_world;
  observer_ptr<UniverseClient> m_client;

  QuestState m_state;
  bool m_inited;
  bool m_showDialog;

  QuestArcDescriptor m_arc;
  size_t m_arcPos;
  StringMap<QuestParam> m_parameters;
  DisplayParameters m_displayParameters;
  Maybe<WorldId> m_worldId;
  Maybe<pair<Vec3I, SystemLocation>> m_location;
  Maybe<Uuid> m_serverUuid;
  size_t m_money;
  List<ItemConstPtr> m_rewards;
  int64_t m_lastUpdatedOn;
  bool m_unread;
  bool m_canTurnIn;
  StringSet m_indicators;

  String m_trackedIndicator;
  String m_untrackedIndicator;
  AssetsConstPtr m_assets;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  QuestTemplateDatabaseConstPtr m_questTemplateDatabase;
  VersioningDatabaseConstPtr m_versioningDatabase;

  String m_title;
  String m_text;
  String m_completionText;
  String m_failureText;
  StringMap<List<Drawable>> m_portraits;
  StringMap<String> m_portraitTitles;

  Maybe<JsonArray> m_objectiveList;
  Maybe<float> m_progress;
  Maybe<float> m_compassDirection;

  LuaMessageHandlingComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaStorableComponent<LuaWorldComponent<LuaBaseComponent>>>>>
      m_scriptComponent;
};

// Create an instance of Quest for a specific template with all the parameters filled
// in with examples. Doesn't necessarily make a valid quest that can be completed, since
// its purpose is for previewing dialogs only.
QuestPtr createPreviewQuest(
    String const& templateId, String const& position, String const& questGiverSpecies, Player& player);
}
