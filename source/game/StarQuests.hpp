#pragma once

#include "StarConfig.hpp"
#include "StarDrawable.hpp"
#include "StarItemDescriptor.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaComponents.hpp"
#include "StarQuestDescriptor.hpp"
#include "StarQuestTemplateDatabase.hpp"
#include "StarSystemWorld.hpp"
#include "StarWarping.hpp"

import std;

namespace Star {

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
  Quest(QuestArcDescriptor const& questArc, size_t arcPos, Player* player);

  Quest(Json const& diskStore);
  auto diskStore() const -> Json;

  auto getTemplate() const -> Ptr<QuestTemplate>;

  void init(Player* player, World* world, UniverseClient* client);
  void uninit();

  auto receiveMessage(String const& message, bool localMessage, JsonArray const& args = {}) -> std::optional<Json>;
  auto callScript(String const& func, LuaVariadic<LuaValue> const& args) -> std::optional<LuaValue>;
  void update(float dt);

  void offer();
  void declineOffer();
  void cancelOffer();
  void start();
  void complete(std::optional<size_t> followupIndex = {});
  void fail();
  void abandon();

  auto interactWithEntity(EntityId entity) -> bool;

  // The generated ID for this instance of the quest with these specific
  // parameters. Multiple players iin a universe may have quests with the same
  // questId if the the source of the quest was the same.
  auto questId() const -> String;
  // The ID of the template this quest was created from
  auto templateId() const -> String;

  auto parameters() const -> StringMap<QuestParam> const&;

  auto state() const -> QuestState;

  // Whether to show the Complete / Failed dialog
  auto showDialog() const -> bool;
  void setDialogShown();

  void setEntityParameter(String const& paramName, ConstPtr<Entity> const& entity);
  void setParameter(String const& paramName, QuestParam const& paramValue);

  auto portrait(String const& portraitName) const -> std::optional<List<Drawable>>;
  auto portraitTitle(String const& portraitName) const -> std::optional<String>;

  auto questDescriptor() const -> QuestDescriptor;
  auto questArcDescriptor() const -> QuestArcDescriptor;
  auto questArcPosition() const -> size_t;

  auto worldId() const -> std::optional<WorldId>;
  auto location() const -> std::optional<std::pair<Vec3I, SystemLocation>>;
  auto serverUuid() const -> std::optional<Uuid>;
  void setWorldId(std::optional<WorldId> worldId);
  void setLocation(std::optional<std::pair<Vec3I, SystemLocation>> location);
  void setServerUuid(std::optional<Uuid> serverUuid);

  auto title() const -> String;
  auto text() const -> String;
  auto completionText() const -> String;
  auto failureText() const -> String;

  auto money() const -> size_t;
  auto rewards() const -> List<ConstPtr<Item>>;

  // The time when this quest last changed state (active/completed/failed)
  auto lastUpdatedOn() const -> std::int64_t;
  auto unread() const -> bool;
  void markAsRead();
  auto canTurnIn() const -> bool;

  auto questGiverIndicator() const -> String;
  auto questReceiverIndicator() const -> String;

  // The String returned by this method is an image path, not a reference to a configured indicator
  auto customIndicator(Ptr<Entity> const& entity) const -> std::optional<String>;

  auto objectiveList() const -> std::optional<JsonArray>;
  auto progress() const -> std::optional<float>;
  auto compassDirection() const -> std::optional<float>;

  void setObjectiveList(std::optional<JsonArray> const& objectiveList);
  void setProgress(std::optional<float> const& progress);
  void setCompassDirection(std::optional<float> const& compassDirection);

  auto completionCinema() const -> std::optional<String>;
  auto canBeAbandoned() const -> bool;
  auto ephemeral() const -> bool;
  auto showInLog() const -> bool;
  auto showAcceptDialog() const -> bool;
  auto showCompleteDialog() const -> bool;
  auto showFailDialog() const -> bool;
  auto mainQuest() const -> bool;
  auto hideCrossServer() const -> bool;

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
  auto makeQuestCallbacks(Player* player) -> LuaCallbacks;

  void setEntityParameter(String const& paramName, Entity const* entity);
  void addReward(ItemDescriptor const& reward);

  auto defaultCustomIndicator() const -> String const&;

  Player* m_player;
  World* m_world;
  UniverseClient* m_client;

  QuestState m_state;
  bool m_inited;
  bool m_showDialog;

  QuestArcDescriptor m_arc;
  size_t m_arcPos;
  StringMap<QuestParam> m_parameters;
  DisplayParameters m_displayParameters;
  std::optional<WorldId> m_worldId;
  std::optional<std::pair<Vec3I, SystemLocation>> m_location;
  std::optional<Uuid> m_serverUuid;
  size_t m_money;
  List<ConstPtr<Item>> m_rewards;
  std::int64_t m_lastUpdatedOn;
  bool m_unread;
  bool m_canTurnIn;
  StringSet m_indicators;

  String m_trackedIndicator;
  String m_untrackedIndicator;

  String m_title;
  String m_text;
  String m_completionText;
  String m_failureText;
  StringMap<List<Drawable>> m_portraits;
  StringMap<String> m_portraitTitles;

  std::optional<JsonArray> m_objectiveList;
  std::optional<float> m_progress;
  std::optional<float> m_compassDirection;

  LuaMessageHandlingComponent<LuaActorMovementComponent<LuaUpdatableComponent<LuaStorableComponent<LuaWorldComponent<LuaBaseComponent>>>>>
    m_scriptComponent;
};

// Create an instance of Quest for a specific template with all the parameters filled
// in with examples. Doesn't necessarily make a valid quest that can be completed, since
// its purpose is for previewing dialogs only.
auto createPreviewQuest(
  String const& templateId, String const& position, String const& questGiverSpecies, Player* player) -> Ptr<Quest>;
}// namespace Star
