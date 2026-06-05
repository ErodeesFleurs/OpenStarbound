#pragma once

#include "StarAiTypes.hpp"
#include "StarGameTimers.hpp"
#include "StarWarping.hpp"
#include "StarAnimation.hpp"
#include "StarItemDescriptor.hpp"
#include "StarPane.hpp"
#include "StarMainInterfaceTypes.hpp"
#include "StarTechDatabase.hpp"

namespace Star {

class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;
class AiDatabase;
using AiDatabaseConstPtr = SharedPtr<AiDatabase const>;
class Cinematic;
using CinematicPtr = SharedPtr<Cinematic>;
class LabelWidget;
using LabelWidgetPtr = SharedPtr<LabelWidget>;
class ImageWidget;
using ImageWidgetPtr = SharedPtr<ImageWidget>;
class ImageStretchWidget;
using ImageStretchWidgetPtr = SharedPtr<ImageStretchWidget>;
class CanvasWidget;
using CanvasWidgetPtr = SharedPtr<CanvasWidget>;
class ListWidget;
using ListWidgetPtr = SharedPtr<ListWidget>;
class ButtonWidget;
using ButtonWidgetPtr = SharedPtr<ButtonWidget>;
class QuestManager;
using QuestManagerPtr = SharedPtr<QuestManager>;
class StackWidget;
using StackWidgetPtr = SharedPtr<StackWidget>;
class Companion;
using CompanionPtr = SharedPtr<Companion>;

struct AiInterfaceExceptionTag { static constexpr char const* typeName = "AiInterfaceException"; };
using AiInterfaceException = TypedException<StarException, AiInterfaceExceptionTag>;

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
