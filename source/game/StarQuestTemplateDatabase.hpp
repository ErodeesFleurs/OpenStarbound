#pragma once

#include "StarConfig.hpp"
#include "StarItemDescriptor.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

// A Quest Template
// Used to check prerequisites for quest availability and by the QuestManager to
// instantiate quests
class QuestTemplate {
public:
  QuestTemplate(Json const& config);

  Json config;
  String templateId;
  String title;
  String text;
  String completionText;
  String failureText;
  StringMap<String> parameterTypes;
  JsonObject parameterExamples;
  Vec2U moneyRange;
  List<List<ItemDescriptor>> rewards;
  List<String> rewardParameters;
  std::optional<String> completionCinema;
  bool canBeAbandoned;
  // Whether the quest is cleared from the quest log when it is
  // completed/failed.
  bool ephemeral;
  // determine whether to show the quest in the quest log
  bool showInLog;
  // whether to show the quest accept, quest complete, and/or quest fail popups
  bool showAcceptDialog;
  bool showCompleteDialog;
  bool showFailDialog;
  // main quests are listed separately in the quest log
  bool mainQuest;
  // hide from log when the quest server uuid doesn't match the current client context server uuid
  bool hideCrossServer;
  String questGiverIndicator;
  String questReceiverIndicator;

  List<String> prerequisiteQuests;
  std::optional<unsigned> requiredShipLevel;
  List<ItemDescriptor> requiredItems;

  unsigned updateDelta;
  std::optional<String> script;
  JsonObject scriptConfig;

  std::optional<String> newQuestGuiConfig;
  std::optional<String> questCompleteGuiConfig;
  std::optional<String> questFailedGuiConfig;
};

// Quest Template Database
// Stores and returns from the list of known quest templates
class QuestTemplateDatabase {
public:
  QuestTemplateDatabase();

  // Return a list of all known template id values
  [[nodiscard]] auto allQuestTemplateIds() const -> List<String>;

  // Return the template for the given template id
  [[nodiscard]] auto questTemplate(String const& templateId) const -> Ptr<QuestTemplate>;

private:
  StringMap<Ptr<QuestTemplate>> m_templates;
};

}// namespace Star
