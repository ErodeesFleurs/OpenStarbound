#pragma once

#include "StarAiTypes.hpp"
#include "StarGameTimers.hpp"
#include "StarWarping.hpp"
#include "StarAnimation.hpp"
#include "StarItemDescriptor.hpp"
#include "StarPane.hpp"
#include "StarMainInterfaceTypes.hpp"
#include "StarAssets.hpp"
#include "StarTechDatabase.hpp"
#include "StarQuestManager.hpp"

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
class StackWidget;
using StackWidgetPtr = SharedPtr<StackWidget>;
class Companion;
using CompanionPtr = SharedPtr<Companion>;

struct AiInterfaceExceptionTag { static constexpr char const* typeName = "AiInterfaceException"; };
using AiInterfaceException = TypedException<StarException, AiInterfaceExceptionTag>;

struct AiInterfaceServices {
  AssetsConstPtr assets;
  AiDatabaseConstPtr aiDatabase;
  GuiContext& guiContext;
};

class AiInterface : public Pane {
public:
  AiInterface(UniverseClientPtr client,
      CinematicPtr cinematic,
      MainInterfacePaneManager& paneManager,
      AiInterfaceServices services);

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

  struct FaceAnimation {
    String name;
    Animation animation;
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
  MainInterfacePaneManager& m_paneManager;
  QuestManagerPtr m_questManager;
  AssetsConstPtr m_assets;

  EntityId m_sourceEntityId;

  AiDatabaseConstPtr m_aiDatabase;

  Animation m_staticAnimation;
  Animation m_scanlineAnimation;
  FaceAnimation m_faceAnimation;

  AudioInstancePtr m_chatterSound;

  WidgetRef<StackWidget> m_mainStack;
  WidgetRef<StackWidget> m_missionStack;
  WidgetRef<StackWidget> m_crewStack;

  WidgetRef<ButtonWidget> m_showMissionsButton;
  WidgetRef<ButtonWidget> m_showCrewButton;
  WidgetRef<ButtonWidget> m_backButton;

  int m_breadcrumbLeftPadding;
  int m_breadcrumbRightPadding;
  WidgetRef<ImageStretchWidget> m_homeBreadcrumbBackground;
  WidgetRef<ImageStretchWidget> m_pageBreadcrumbBackground;
  WidgetRef<ImageStretchWidget> m_itemBreadcrumbBackground;
  WidgetRef<LabelWidget> m_homeBreadcrumbWidget;
  WidgetRef<LabelWidget> m_pageBreadcrumbWidget;
  WidgetRef<LabelWidget> m_itemBreadcrumbWidget;

  WidgetRef<LabelWidget> m_currentTextWidget;

  WidgetRef<CanvasWidget> m_aiFaceCanvasWidget;
  LabelWidgetPtr m_shipStatusTextWidget;

  WidgetRef<ListWidget> m_missionListWidget;
  WidgetRef<LabelWidget> m_missionNameLabel;
  WidgetRef<ImageWidget> m_missionIcon;

  WidgetRef<ListWidget> m_crewListWidget;
  WidgetRef<LabelWidget> m_recruitNameLabel;
  WidgetRef<ImageWidget> m_recruitIcon;

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
  float m_textLength = 0.0f;
  float m_textMaxLength = 0.0f;

  WidgetRef<ButtonWidget> m_startMissionButton;
  WidgetRef<ButtonWidget> m_dismissRecruitButton;
};

}
