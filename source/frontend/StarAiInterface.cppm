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
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
class AiDatabase;
using AiDatabasePtr = std::shared_ptr<AiDatabase>;
using AiDatabaseConstPtr = std::shared_ptr<AiDatabase const>;
using AiDatabaseWeakPtr = std::weak_ptr<AiDatabase>;
using AiDatabaseConstWeakPtr = std::weak_ptr<AiDatabase const>;
using AiDatabaseUPtr = std::unique_ptr<AiDatabase>;
using AiDatabaseConstUPtr = std::unique_ptr<AiDatabase const>;
class Cinematic;
using CinematicPtr = std::shared_ptr<Cinematic>;
using CinematicConstPtr = std::shared_ptr<Cinematic const>;
using CinematicWeakPtr = std::weak_ptr<Cinematic>;
using CinematicConstWeakPtr = std::weak_ptr<Cinematic const>;
using CinematicUPtr = std::unique_ptr<Cinematic>;
using CinematicConstUPtr = std::unique_ptr<Cinematic const>;
class LabelWidget;
using LabelWidgetPtr = std::shared_ptr<LabelWidget>;
using LabelWidgetConstPtr = std::shared_ptr<LabelWidget const>;
using LabelWidgetWeakPtr = std::weak_ptr<LabelWidget>;
using LabelWidgetConstWeakPtr = std::weak_ptr<LabelWidget const>;
using LabelWidgetUPtr = std::unique_ptr<LabelWidget>;
using LabelWidgetConstUPtr = std::unique_ptr<LabelWidget const>;
class ImageWidget;
using ImageWidgetPtr = std::shared_ptr<ImageWidget>;
using ImageWidgetConstPtr = std::shared_ptr<ImageWidget const>;
using ImageWidgetWeakPtr = std::weak_ptr<ImageWidget>;
using ImageWidgetConstWeakPtr = std::weak_ptr<ImageWidget const>;
using ImageWidgetUPtr = std::unique_ptr<ImageWidget>;
using ImageWidgetConstUPtr = std::unique_ptr<ImageWidget const>;
class ImageStretchWidget;
using ImageStretchWidgetPtr = std::shared_ptr<ImageStretchWidget>;
using ImageStretchWidgetConstPtr = std::shared_ptr<ImageStretchWidget const>;
using ImageStretchWidgetWeakPtr = std::weak_ptr<ImageStretchWidget>;
using ImageStretchWidgetConstWeakPtr = std::weak_ptr<ImageStretchWidget const>;
using ImageStretchWidgetUPtr = std::unique_ptr<ImageStretchWidget>;
using ImageStretchWidgetConstUPtr = std::unique_ptr<ImageStretchWidget const>;
class CanvasWidget;
using CanvasWidgetPtr = std::shared_ptr<CanvasWidget>;
using CanvasWidgetConstPtr = std::shared_ptr<CanvasWidget const>;
using CanvasWidgetWeakPtr = std::weak_ptr<CanvasWidget>;
using CanvasWidgetConstWeakPtr = std::weak_ptr<CanvasWidget const>;
using CanvasWidgetUPtr = std::unique_ptr<CanvasWidget>;
using CanvasWidgetConstUPtr = std::unique_ptr<CanvasWidget const>;
class ListWidget;
using ListWidgetPtr = std::shared_ptr<ListWidget>;
using ListWidgetConstPtr = std::shared_ptr<ListWidget const>;
using ListWidgetWeakPtr = std::weak_ptr<ListWidget>;
using ListWidgetConstWeakPtr = std::weak_ptr<ListWidget const>;
using ListWidgetUPtr = std::unique_ptr<ListWidget>;
using ListWidgetConstUPtr = std::unique_ptr<ListWidget const>;
class ButtonWidget;
using ButtonWidgetPtr = std::shared_ptr<ButtonWidget>;
using ButtonWidgetConstPtr = std::shared_ptr<ButtonWidget const>;
using ButtonWidgetWeakPtr = std::weak_ptr<ButtonWidget>;
using ButtonWidgetConstWeakPtr = std::weak_ptr<ButtonWidget const>;
using ButtonWidgetUPtr = std::unique_ptr<ButtonWidget>;
using ButtonWidgetConstUPtr = std::unique_ptr<ButtonWidget const>;
class QuestManager;
using QuestManagerPtr = std::shared_ptr<QuestManager>;
using QuestManagerConstPtr = std::shared_ptr<QuestManager const>;
using QuestManagerWeakPtr = std::weak_ptr<QuestManager>;
using QuestManagerConstWeakPtr = std::weak_ptr<QuestManager const>;
using QuestManagerUPtr = std::unique_ptr<QuestManager>;
using QuestManagerConstUPtr = std::unique_ptr<QuestManager const>;
class StackWidget;
using StackWidgetPtr = std::shared_ptr<StackWidget>;
using StackWidgetConstPtr = std::shared_ptr<StackWidget const>;
using StackWidgetWeakPtr = std::weak_ptr<StackWidget>;
using StackWidgetConstWeakPtr = std::weak_ptr<StackWidget const>;
using StackWidgetUPtr = std::unique_ptr<StackWidget>;
using StackWidgetConstUPtr = std::unique_ptr<StackWidget const>;
class TabSetWidget;
using TabSetWidgetPtr = std::shared_ptr<TabSetWidget>;
using TabSetWidgetConstPtr = std::shared_ptr<TabSetWidget const>;
using TabSetWidgetWeakPtr = std::weak_ptr<TabSetWidget>;
using TabSetWidgetConstWeakPtr = std::weak_ptr<TabSetWidget const>;
using TabSetWidgetUPtr = std::unique_ptr<TabSetWidget>;
using TabSetWidgetConstUPtr = std::unique_ptr<TabSetWidget const>;
class Companion;
using CompanionPtr = std::shared_ptr<Companion>;
using CompanionConstPtr = std::shared_ptr<Companion const>;
using CompanionWeakPtr = std::weak_ptr<Companion>;
using CompanionConstWeakPtr = std::weak_ptr<Companion const>;
using CompanionUPtr = std::unique_ptr<Companion>;
using CompanionConstUPtr = std::unique_ptr<Companion const>;
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


class AiInterface;
using AiInterfacePtr = std::shared_ptr<AiInterface>;
using AiInterfaceConstPtr = std::shared_ptr<AiInterface const>;
using AiInterfaceWeakPtr = std::weak_ptr<AiInterface>;
using AiInterfaceConstWeakPtr = std::weak_ptr<AiInterface const>;
using AiInterfaceUPtr = std::unique_ptr<AiInterface>;
using AiInterfaceConstUPtr = std::unique_ptr<AiInterface const>;

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

  UniverseClientPtr m_client;
  CinematicPtr m_cinematic;
  MainInterfacePaneManager* m_paneManager;
  QuestManagerPtr m_questManager;

  EntityId m_sourceEntityId;

  AiDatabaseConstPtr m_aiDatabase;

  Animation m_staticAnimation;
  Animation m_scanlineAnimation;
  std::pair<String, Animation> m_faceAnimation;

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
