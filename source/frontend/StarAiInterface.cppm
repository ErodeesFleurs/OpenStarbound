module;

#include "StarGameTypes.hpp"
#include "StarJson.hpp"

#include "StarOrderedSet.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarVector.hpp"
#include "StarArray.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"

#include "StarOrderedMap.hpp"





namespace Star {
STAR_CLASS(UniverseClient);
STAR_CLASS(AiDatabase);
STAR_CLASS(Cinematic);
STAR_CLASS(LabelWidget);
STAR_CLASS(ImageWidget);
STAR_CLASS(ImageStretchWidget);
STAR_CLASS(CanvasWidget);
STAR_CLASS(ListWidget);
STAR_CLASS(ButtonWidget);
STAR_CLASS(QuestManager);
STAR_CLASS(StackWidget);
STAR_CLASS(TabSetWidget);
STAR_CLASS(Companion);
}

import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;
import star.uuid;
import star.warping;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.pane;
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;
import star.main_interface_types;
import star.animation;
import star.tech_database;

namespace Star {


STAR_CLASS(AiInterface);

struct AiInterfaceExceptionTag {
  static constexpr char const* name() { return "AiInterfaceException"; }
};
using AiInterfaceException = StarError<AiInterfaceExceptionTag, StarException>;

class AiInterface : public Pane {
public:
  AiInterface(UniverseClientPtr client, CinematicPtr cinematic, MainInterfacePaneManager* paneManager);

  void update(float dt) override;

  void displayed() override;
  void dismissed() override;

  void setSourceEntityId(EntityId sourceEntityId);

private:
  enum class AiPages : uint8_t {
    StatusPage,
    MissionList,
    MissionPage,
    CrewList,
    CrewPage
  };

  void updateBreadcrumbs();
  void showStatus();

  void populateMissions();
  void showMissions();
  void selectMission();
  void startMission();

  void populateCrew();
  void showCrew();
  void selectRecruit();
  void dismissRecruit();

  void goBack();

  void setFaceAnimation(String const& name);
  void setCurrentSpeech(String const& textWidget, AiSpeech speech);

  void giveBlueprint(String const& blueprintName);

  AiPages m_currentPage;

  UniverseClientPtr m_client;
  CinematicPtr m_cinematic;
  MainInterfacePaneManager* m_paneManager;
  QuestManagerPtr m_questManager;

  EntityId m_sourceEntityId;

  AiDatabaseConstPtr m_aiDatabase;

  Animation m_staticAnimation;
  Animation m_scanlineAnimation;
  pair<String, Animation> m_faceAnimation;

  AudioInstancePtr m_chatterSound;

  StackWidgetPtr m_mainStack;
  StackWidgetPtr m_missionStack;
  StackWidgetPtr m_crewStack;

  ButtonWidgetPtr m_showMissionsButton;
  ButtonWidgetPtr m_showCrewButton;
  ButtonWidgetPtr m_backButton;

  int m_breadcrumbLeftPadding;
  int m_breadcrumbRightPadding;
  ImageStretchWidgetPtr m_homeBreadcrumbBackground;
  ImageStretchWidgetPtr m_pageBreadcrumbBackground;
  ImageStretchWidgetPtr m_itemBreadcrumbBackground;
  LabelWidgetPtr m_homeBreadcrumbWidget;
  LabelWidgetPtr m_pageBreadcrumbWidget;
  LabelWidgetPtr m_itemBreadcrumbWidget;

  LabelWidgetPtr m_currentTextWidget;

  CanvasWidgetPtr m_aiFaceCanvasWidget;
  LabelWidgetPtr m_shipStatusTextWidget;

  ListWidgetPtr m_missionListWidget;
  LabelWidgetPtr m_missionNameLabel;
  ImageWidgetPtr m_missionIcon;

  ListWidgetPtr m_crewListWidget;
  LabelWidgetPtr m_recruitNameLabel;
  ImageWidgetPtr m_recruitIcon;

  String m_species;

  String m_missionBreadcrumbText;
  String m_missionDeployText;
  String m_crewBreadcrumbText;
  String m_defaultRecruitName;
  String m_defaultRecruitDescription;

  StringList m_availableMissions;
  StringList m_completedMissions;
  Maybe<String> m_selectedMission;

  List<CompanionPtr> m_crew;
  CompanionPtr m_selectedRecruit;

  Maybe<AiSpeech> m_currentSpeech;
  float m_textLength;
  float m_textMaxLength;

  ButtonWidgetPtr m_startMissionButton;
  ButtonWidgetPtr m_dismissRecruitButton;
};

}

export module star.ai_interface;

export namespace Star {
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::AiDatabase;
  using ::Star::AiDatabasePtr;
  using ::Star::AiDatabaseConstPtr;
  using ::Star::AiDatabaseWeakPtr;
  using ::Star::AiDatabaseConstWeakPtr;
  using ::Star::AiDatabaseUPtr;
  using ::Star::AiDatabaseConstUPtr;
  using ::Star::Cinematic;
  using ::Star::CinematicPtr;
  using ::Star::CinematicConstPtr;
  using ::Star::CinematicWeakPtr;
  using ::Star::CinematicConstWeakPtr;
  using ::Star::CinematicUPtr;
  using ::Star::CinematicConstUPtr;
  using ::Star::LabelWidget;
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
  using ::Star::ImageWidget;
  using ::Star::ImageWidgetPtr;
  using ::Star::ImageWidgetConstPtr;
  using ::Star::ImageWidgetWeakPtr;
  using ::Star::ImageWidgetConstWeakPtr;
  using ::Star::ImageWidgetUPtr;
  using ::Star::ImageWidgetConstUPtr;
  using ::Star::ImageStretchWidget;
  using ::Star::ImageStretchWidgetPtr;
  using ::Star::ImageStretchWidgetConstPtr;
  using ::Star::ImageStretchWidgetWeakPtr;
  using ::Star::ImageStretchWidgetConstWeakPtr;
  using ::Star::ImageStretchWidgetUPtr;
  using ::Star::ImageStretchWidgetConstUPtr;
  using ::Star::CanvasWidget;
  using ::Star::CanvasWidgetPtr;
  using ::Star::CanvasWidgetConstPtr;
  using ::Star::CanvasWidgetWeakPtr;
  using ::Star::CanvasWidgetConstWeakPtr;
  using ::Star::CanvasWidgetUPtr;
  using ::Star::CanvasWidgetConstUPtr;
  using ::Star::ListWidget;
  using ::Star::ListWidgetPtr;
  using ::Star::ListWidgetConstPtr;
  using ::Star::ListWidgetWeakPtr;
  using ::Star::ListWidgetConstWeakPtr;
  using ::Star::ListWidgetUPtr;
  using ::Star::ListWidgetConstUPtr;
  using ::Star::ButtonWidget;
  using ::Star::ButtonWidgetPtr;
  using ::Star::ButtonWidgetConstPtr;
  using ::Star::ButtonWidgetWeakPtr;
  using ::Star::ButtonWidgetConstWeakPtr;
  using ::Star::ButtonWidgetUPtr;
  using ::Star::ButtonWidgetConstUPtr;
  using ::Star::QuestManager;
  using ::Star::QuestManagerPtr;
  using ::Star::QuestManagerConstPtr;
  using ::Star::QuestManagerWeakPtr;
  using ::Star::QuestManagerConstWeakPtr;
  using ::Star::QuestManagerUPtr;
  using ::Star::QuestManagerConstUPtr;
  using ::Star::StackWidget;
  using ::Star::StackWidgetPtr;
  using ::Star::StackWidgetConstPtr;
  using ::Star::StackWidgetWeakPtr;
  using ::Star::StackWidgetConstWeakPtr;
  using ::Star::StackWidgetUPtr;
  using ::Star::StackWidgetConstUPtr;
  using ::Star::TabSetWidget;
  using ::Star::TabSetWidgetPtr;
  using ::Star::TabSetWidgetConstPtr;
  using ::Star::TabSetWidgetWeakPtr;
  using ::Star::TabSetWidgetConstWeakPtr;
  using ::Star::TabSetWidgetUPtr;
  using ::Star::TabSetWidgetConstUPtr;
  using ::Star::Companion;
  using ::Star::CompanionPtr;
  using ::Star::CompanionConstPtr;
  using ::Star::CompanionWeakPtr;
  using ::Star::CompanionConstWeakPtr;
  using ::Star::CompanionUPtr;
  using ::Star::CompanionConstUPtr;
  using ::Star::AiInterface;
  using ::Star::AiInterfacePtr;
  using ::Star::AiInterfaceConstPtr;
  using ::Star::AiInterfaceWeakPtr;
  using ::Star::AiInterfaceConstWeakPtr;
  using ::Star::AiInterfaceUPtr;
  using ::Star::AiInterfaceConstUPtr;
  using ::Star::AiInterfaceExceptionTag;
  using ::Star::AiInterfaceException;
}
