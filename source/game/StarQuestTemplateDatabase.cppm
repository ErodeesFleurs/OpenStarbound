module;
import std;
import star.string;
import star.list;
import star.maybe;
import star.json;

import star.data_stream;
import star.bi_map;
import star.item_descriptor;
import star.vector;

namespace Star {

class SpeciesTextVariants;
using SpeciesTextVariantsPtr = std::shared_ptr<SpeciesTextVariants>;
using SpeciesTextVariantsConstPtr = std::shared_ptr<const SpeciesTextVariants>;
using SpeciesTextVariantsWeakPtr = std::weak_ptr<SpeciesTextVariants>;
using SpeciesTextVariantsConstWeakPtr = std::weak_ptr<const SpeciesTextVariants>;
using SpeciesTextVariantsUPtr = std::unique_ptr<SpeciesTextVariants>;
using SpeciesTextVariantsConstUPtr = std::unique_ptr<const SpeciesTextVariants>;
class PositionalTextVariants;
using PositionalTextVariantsPtr = std::shared_ptr<PositionalTextVariants>;
using PositionalTextVariantsConstPtr = std::shared_ptr<const PositionalTextVariants>;
using PositionalTextVariantsWeakPtr = std::weak_ptr<PositionalTextVariants>;
using PositionalTextVariantsConstWeakPtr = std::weak_ptr<const PositionalTextVariants>;
using PositionalTextVariantsUPtr = std::unique_ptr<PositionalTextVariants>;
using PositionalTextVariantsConstUPtr = std::unique_ptr<const PositionalTextVariants>;
class QuestTemplate;
using QuestTemplatePtr = std::shared_ptr<QuestTemplate>;
using QuestTemplateConstPtr = std::shared_ptr<const QuestTemplate>;
using QuestTemplateWeakPtr = std::weak_ptr<QuestTemplate>;
using QuestTemplateConstWeakPtr = std::weak_ptr<const QuestTemplate>;
using QuestTemplateUPtr = std::unique_ptr<QuestTemplate>;
using QuestTemplateConstUPtr = std::unique_ptr<const QuestTemplate>;
class QuestTemplateDatabase;
using QuestTemplateDatabasePtr = std::shared_ptr<QuestTemplateDatabase>;
using QuestTemplateDatabaseConstPtr = std::shared_ptr<const QuestTemplateDatabase>;
using QuestTemplateDatabaseWeakPtr = std::weak_ptr<QuestTemplateDatabase>;
using QuestTemplateDatabaseConstWeakPtr = std::weak_ptr<const QuestTemplateDatabase>;
using QuestTemplateDatabaseUPtr = std::unique_ptr<QuestTemplateDatabase>;
using QuestTemplateDatabaseConstUPtr = std::unique_ptr<const QuestTemplateDatabase>;

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
  Maybe<String> completionCinema;
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
  Maybe<unsigned> requiredShipLevel;
  List<ItemDescriptor> requiredItems;

  unsigned updateDelta;
  Maybe<String> script;
  JsonObject scriptConfig;

  Maybe<String> newQuestGuiConfig;
  Maybe<String> questCompleteGuiConfig;
  Maybe<String> questFailedGuiConfig;
};

// Quest Template Database
// Stores and returns from the list of known quest templates
class QuestTemplateDatabase {
public:
  QuestTemplateDatabase();

  // Return a list of all known template id values
  List<String> allQuestTemplateIds() const;

  // Return the template for the given template id
  QuestTemplatePtr questTemplate(String const& templateId) const;

private:
  StringMap<QuestTemplatePtr> m_templates;
};

}

export module star.quest_template_database;

export namespace Star {
  using ::Star::SpeciesTextVariants;
  using ::Star::SpeciesTextVariantsPtr;
  using ::Star::SpeciesTextVariantsConstPtr;
  using ::Star::PositionalTextVariants;
  using ::Star::PositionalTextVariantsPtr;
  using ::Star::PositionalTextVariantsConstPtr;
  using ::Star::QuestTemplate;
  using ::Star::QuestTemplatePtr;
  using ::Star::QuestTemplateConstPtr;
  using ::Star::QuestTemplateDatabase;
  using ::Star::QuestTemplateDatabasePtr;
  using ::Star::QuestTemplateDatabaseConstPtr;
}
