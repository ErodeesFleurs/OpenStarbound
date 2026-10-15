module;
import std;
import star.format;
import star.json;

import star.game_types;

import star.ordered_set;
import star.data_stream;
import star.bi_map;
#include "StarStrongTypedef.hpp"
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.set;
import star.audio;
import star.map;

import star.ordered_map;





namespace Star {
class UniverseClient;
class AiDatabase;
class Cinematic;
class LabelWidget;
class ImageWidget;
class ImageStretchWidget;
class CanvasWidget;
class ListWidget;
class ButtonWidget;
class QuestManager;
class StackWidget;
class TabSetWidget;
class Companion;
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


struct AiInterfaceExceptionTag {
  static constexpr char const* name() { return "AiInterfaceException"; }
};
using AiInterfaceException = StarError<AiInterfaceExceptionTag, StarException>;

class AiInterface : public Pane {
public:
  AiInterface(std::shared_ptr<UniverseClient> client, std::shared_ptr<Cinematic> cinematic, MainInterfacePaneManager* paneManager);

  void update(float dt) override;

  void displayed() override;
  void dismissed() override;

  void setSourceEntityId(EntityId sourceEntityId);

private:
  enum class AiPages : std::uint8_t {
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

  std::shared_ptr<UniverseClient> m_client;
  std::shared_ptr<Cinematic> m_cinematic;
  MainInterfacePaneManager* m_paneManager;
  std::shared_ptr<QuestManager> m_questManager;

  EntityId m_sourceEntityId;

  std::shared_ptr<AiDatabase const> m_aiDatabase;

  Animation m_staticAnimation;
  Animation m_scanlineAnimation;
  std::pair<String, Animation> m_faceAnimation;

  std::shared_ptr<AudioInstance> m_chatterSound;

  std::shared_ptr<StackWidget> m_mainStack;
  std::shared_ptr<StackWidget> m_missionStack;
  std::shared_ptr<StackWidget> m_crewStack;

  std::shared_ptr<ButtonWidget> m_showMissionsButton;
  std::shared_ptr<ButtonWidget> m_showCrewButton;
  std::shared_ptr<ButtonWidget> m_backButton;

  int m_breadcrumbLeftPadding;
  int m_breadcrumbRightPadding;
  std::shared_ptr<ImageStretchWidget> m_homeBreadcrumbBackground;
  std::shared_ptr<ImageStretchWidget> m_pageBreadcrumbBackground;
  std::shared_ptr<ImageStretchWidget> m_itemBreadcrumbBackground;
  std::shared_ptr<LabelWidget> m_homeBreadcrumbWidget;
  std::shared_ptr<LabelWidget> m_pageBreadcrumbWidget;
  std::shared_ptr<LabelWidget> m_itemBreadcrumbWidget;

  std::shared_ptr<LabelWidget> m_currentTextWidget;

  std::shared_ptr<CanvasWidget> m_aiFaceCanvasWidget;
  std::shared_ptr<LabelWidget> m_shipStatusTextWidget;

  std::shared_ptr<ListWidget> m_missionListWidget;
  std::shared_ptr<LabelWidget> m_missionNameLabel;
  std::shared_ptr<ImageWidget> m_missionIcon;

  std::shared_ptr<ListWidget> m_crewListWidget;
  std::shared_ptr<LabelWidget> m_recruitNameLabel;
  std::shared_ptr<ImageWidget> m_recruitIcon;

  String m_species;

  String m_missionBreadcrumbText;
  String m_missionDeployText;
  String m_crewBreadcrumbText;
  String m_defaultRecruitName;
  String m_defaultRecruitDescription;

  StringList m_availableMissions;
  StringList m_completedMissions;
  Maybe<String> m_selectedMission;

  List<std::shared_ptr<Companion>> m_crew;
  std::shared_ptr<Companion> m_selectedRecruit;

  Maybe<AiSpeech> m_currentSpeech;
  float m_textLength;
  float m_textMaxLength;

  std::shared_ptr<ButtonWidget> m_startMissionButton;
  std::shared_ptr<ButtonWidget> m_dismissRecruitButton;
};

}

export module star.ai_interface;

export namespace Star {
  using ::Star::UniverseClient;
  using ::Star::AiDatabase;
  using ::Star::Cinematic;
  using ::Star::LabelWidget;
  using ::Star::ImageWidget;
  using ::Star::ImageStretchWidget;
  using ::Star::CanvasWidget;
  using ::Star::ListWidget;
  using ::Star::ButtonWidget;
  using ::Star::QuestManager;
  using ::Star::StackWidget;
  using ::Star::TabSetWidget;
  using ::Star::Companion;
  using ::Star::AiInterface;
  using ::Star::AiInterfaceExceptionTag;
  using ::Star::AiInterfaceException;
}
