module;
#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetElementSystem.hpp"
#include "StarRpcPromise.hpp"
#include "StarOrderedMap.hpp"
#include "StarArray.hpp"


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
import star.inventory_types;
import star.item_descriptor;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.tile_damage;

import star.interaction_types;

import star.celestial_coordinate;
import star.quest_descriptor;

import star.interactive_entity;
import star.collision_block;

import star.tile_entity;

import star.container_entity;

import star.container_interactor;

import star.game_timers;

import star.inventory;

import star.animation;

import star.interface_cursor;

import star.pane_manager;
import star.registered_pane_manager;

import star.main_interface_types;
import star.uuid;
import star.warping;

namespace Star {
enum class P2PJoinRequestReply;
STAR_CLASS(InventoryPane);
STAR_CLASS(ItemSlotWidget);
}

namespace Star {

STAR_CLASS(UniverseClient);
STAR_CLASS(WorldPainter);
STAR_CLASS(Item);
STAR_CLASS(Chat);
STAR_CLASS(ClientCommandProcessor);
STAR_CLASS(OptionsMenu);
STAR_CLASS(WirePane);
STAR_CLASS(ActionBar);
STAR_CLASS(TeamBar);
STAR_CLASS(StatusPane);
STAR_CLASS(ContainerPane);
STAR_CLASS(CraftingPane);
STAR_CLASS(MerchantPane);
STAR_CLASS(CodexInterface);
STAR_CLASS(SongbookInterface);
STAR_CLASS(QuestLogInterface);
STAR_CLASS(AiInterface);
STAR_CLASS(PopupInterface);
STAR_CLASS(ConfirmationDialog);
STAR_CLASS(JoinRequestDialog);
STAR_CLASS(TeleportDialog);
STAR_CLASS(LabelWidget);
STAR_CLASS(Cinematic);
STAR_CLASS(NameplatePainter);
STAR_CLASS(QuestIndicatorPainter);
STAR_CLASS(RadioMessagePopup);
STAR_CLASS(Quest);
STAR_CLASS(QuestTrackerPane);
STAR_CLASS(ContainerInteractor);
STAR_CLASS(ScriptPane);
STAR_CLASS(ChatBubbleManager);
STAR_CLASS(CanvasWidget);

STAR_STRUCT(GuiMessage);
STAR_CLASS(MainInterface);

struct GuiMessage {
  GuiMessage();
  GuiMessage(String const& message, float cooldown, float spring = 0);

  String message;
  float cooldown;
  float springState;
};

class MainInterface {
public:
  enum RunningState {
    Running,
    ReturnToTitle
  };

  MainInterface(UniverseClientPtr client, WorldPainterPtr painter, CinematicPtr cinematicOverlay);

  ~MainInterface();

  RunningState currentState() const;

  MainInterfacePaneManager* paneManager();

  bool escapeDialogOpen() const;

  void openCraftingWindow(Json const& config, EntityId sourceEntityId = NullEntityId);
  void openMerchantWindow(Json const& config, EntityId sourceEntityId = NullEntityId);
  void togglePlainCraftingWindow();

  bool windowsOpen() const;

  MerchantPanePtr activeMerchantPane() const;

  // Return true if this event was consumed or should be handled elsewhere.
  bool handleInputEvent(InputEvent const& event);
  // Return true if mouse / keyboard events are currently locked here
  bool inputFocus() const;
  // If input is focused, should MainInterface also accept text input events?
  bool textInputActive() const;
  void handleInteractAction(InteractAction interactAction);

  void preUpdate(float dt);
  // Handles incoming client messages, aims main player, etc.
  void update(float dt);

  // Render things e.g. quest indicators that should be drawn in the world
  // behind interface e.g. chat bubbles
  void renderInWorldElements();
  void render();

  Vec2F cursorWorldPosition() const;

  bool isDebugDisplayed();

  void doChat(String const& chat, bool addToHistory);

  void queueMessage(String const& message, Maybe<float> cooldown, float spring);
  void queueMessage(String const& message);

  void queueItemPickupText(ItemPtr const& item);
  void queueJoinRequest(pair<String, RpcPromiseKeeper<P2PJoinRequestReply>> request);

  bool fixedCamera() const;
  bool hudVisible() const;
  void setHudVisible(bool visible = true);

  void warpToOrbitedWorld(bool deploy = false);
  void warpToOwnShip();
  void warpTo(WarpAction const& warpAction);

  CanvasWidgetPtr fetchCanvas(String const& canvasName, bool ignoreInterfaceScale = false);

  ClientCommandProcessorPtr commandProcessor() const;

  struct ScriptPaneInfo {
    ScriptPanePtr scriptPane;
    Json config;
    EntityId sourceEntityId;
    bool visible;
    Vec2I position;
  };

  void takeScriptPanes(List<ScriptPaneInfo>& out);
  void reviveScriptPanes(List<ScriptPaneInfo>& panes);
  void displayDefaultPanes();
private:
  PanePtr createEscapeDialog();
  void initHttpTrustDialog();

  float interfaceScale() const;
  unsigned windowHeight() const;
  unsigned windowWidth() const;
  Vec2F mainBarPosition() const;

  void renderBreath();
  void renderMessages();
  void renderMonsterHealthBar();
  void renderSpecialDamageBar();
  void renderMainBar();
  void renderWindows();
  void renderDebug();

  void updateCursor();
  void renderCursor();

  bool overButton(PolyI const& buttonPoly, Vec2F const& mousePos) const;

  bool overlayClick(Vec2F const& mousePos, MouseButton mouseButton);

  void displayScriptPane(ScriptPanePtr& scriptPane, EntityId sourceEntity);

  GuiContext* m_guiContext{nullptr};
  MainInterfaceConfigConstPtr m_config;
  InterfaceCursor m_cursor;

  RunningState m_state{Running};

  UniverseClientPtr m_client;
  WorldPainterPtr m_worldPainter;
  CinematicPtr m_cinematicOverlay;

  MainInterfacePaneManager m_paneManager;

  QuestLogInterfacePtr m_questLogInterface;

  InventoryPanePtr m_inventoryWindow;
  CraftingPanePtr m_plainCraftingWindow;
  CraftingPanePtr m_craftingWindow;
  MerchantPanePtr m_merchantWindow;
  CodexInterfacePtr m_codexInterface;
  OptionsMenuPtr m_optionsMenu;
  ContainerPanePtr m_containerPane;
  PopupInterfacePtr m_popupInterface;
  ConfirmationDialogPtr m_confirmationDialog;
  JoinRequestDialogPtr m_joinRequestDialog;
  TeleportDialogPtr m_teleportDialog;
  QuestTrackerPanePtr m_questTracker;
  ScriptPanePtr m_mmUpgrade;
  ScriptPanePtr m_collections;
  Map<EntityId, PanePtr> m_interactionScriptPanes;

  StringMap<CanvasWidgetPtr> m_canvases;

  ChatPtr m_chat;
  ClientCommandProcessorPtr m_clientCommandProcessor;
  RadioMessagePopupPtr m_radioMessagePopup;
  WirePanePtr m_wireInterface;

  ActionBarPtr m_actionBar;
  Vec2F m_cursorScreenPos{};
  Vec2I m_cursorScreenIPos{};
  ItemSlotWidgetPtr m_cursorItem;
  Maybe<String> m_cursorTooltip;

  LabelWidgetPtr m_planetText;
  GameTimer m_planetNameTimer;

  GameTimer m_debugSpatialClearTimer;
  GameTimer m_debugMapClearTimer;
  RectF m_debugTextRect{RectF::null()};

  NameplatePainterPtr m_nameplatePainter;
  QuestIndicatorPainterPtr m_questIndicatorPainter;
  ChatBubbleManagerPtr m_chatBubbleManager;

  bool m_disableHud = false;

  String m_lastCommand;

  LinkedList<GuiMessagePtr> m_messages;
  HashMap<ItemDescriptor, std::pair<size_t, GuiMessagePtr>> m_itemDropMessages;
  unsigned m_messageOverflow{};
  GuiMessagePtr m_overflowMessage;

  List<pair<String, RpcPromiseKeeper<P2PJoinRequestReply>>> m_queuedJoinRequests;

  EntityId m_lastMouseoverTarget{NullEntityId};
  GameTimer m_stickyTargetingTimer;
  int m_portraitScale{};

  HashMap<EntityId, float> m_specialDamageBars;

  ContainerInteractorPtr m_containerInteractor;
};

}

export module star.main_interface;

export namespace Star {
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::WorldPainter;
  using ::Star::WorldPainterPtr;
  using ::Star::WorldPainterConstPtr;
  using ::Star::WorldPainterWeakPtr;
  using ::Star::WorldPainterConstWeakPtr;
  using ::Star::WorldPainterUPtr;
  using ::Star::WorldPainterConstUPtr;
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::Chat;
  using ::Star::ChatPtr;
  using ::Star::ChatConstPtr;
  using ::Star::ChatWeakPtr;
  using ::Star::ChatConstWeakPtr;
  using ::Star::ChatUPtr;
  using ::Star::ChatConstUPtr;
  using ::Star::ClientCommandProcessor;
  using ::Star::ClientCommandProcessorPtr;
  using ::Star::ClientCommandProcessorConstPtr;
  using ::Star::ClientCommandProcessorWeakPtr;
  using ::Star::ClientCommandProcessorConstWeakPtr;
  using ::Star::ClientCommandProcessorUPtr;
  using ::Star::ClientCommandProcessorConstUPtr;
  using ::Star::OptionsMenu;
  using ::Star::OptionsMenuPtr;
  using ::Star::OptionsMenuConstPtr;
  using ::Star::OptionsMenuWeakPtr;
  using ::Star::OptionsMenuConstWeakPtr;
  using ::Star::OptionsMenuUPtr;
  using ::Star::OptionsMenuConstUPtr;
  using ::Star::WirePane;
  using ::Star::WirePanePtr;
  using ::Star::WirePaneConstPtr;
  using ::Star::WirePaneWeakPtr;
  using ::Star::WirePaneConstWeakPtr;
  using ::Star::WirePaneUPtr;
  using ::Star::WirePaneConstUPtr;
  using ::Star::ActionBar;
  using ::Star::ActionBarPtr;
  using ::Star::ActionBarConstPtr;
  using ::Star::ActionBarWeakPtr;
  using ::Star::ActionBarConstWeakPtr;
  using ::Star::ActionBarUPtr;
  using ::Star::ActionBarConstUPtr;
  using ::Star::TeamBar;
  using ::Star::TeamBarPtr;
  using ::Star::TeamBarConstPtr;
  using ::Star::TeamBarWeakPtr;
  using ::Star::TeamBarConstWeakPtr;
  using ::Star::TeamBarUPtr;
  using ::Star::TeamBarConstUPtr;
  using ::Star::StatusPane;
  using ::Star::StatusPanePtr;
  using ::Star::StatusPaneConstPtr;
  using ::Star::StatusPaneWeakPtr;
  using ::Star::StatusPaneConstWeakPtr;
  using ::Star::StatusPaneUPtr;
  using ::Star::StatusPaneConstUPtr;
  using ::Star::ContainerPane;
  using ::Star::ContainerPanePtr;
  using ::Star::ContainerPaneConstPtr;
  using ::Star::ContainerPaneWeakPtr;
  using ::Star::ContainerPaneConstWeakPtr;
  using ::Star::ContainerPaneUPtr;
  using ::Star::ContainerPaneConstUPtr;
  using ::Star::CraftingPane;
  using ::Star::CraftingPanePtr;
  using ::Star::CraftingPaneConstPtr;
  using ::Star::CraftingPaneWeakPtr;
  using ::Star::CraftingPaneConstWeakPtr;
  using ::Star::CraftingPaneUPtr;
  using ::Star::CraftingPaneConstUPtr;
  using ::Star::MerchantPane;
  using ::Star::MerchantPanePtr;
  using ::Star::MerchantPaneConstPtr;
  using ::Star::MerchantPaneWeakPtr;
  using ::Star::MerchantPaneConstWeakPtr;
  using ::Star::MerchantPaneUPtr;
  using ::Star::MerchantPaneConstUPtr;
  using ::Star::CodexInterface;
  using ::Star::CodexInterfacePtr;
  using ::Star::CodexInterfaceConstPtr;
  using ::Star::CodexInterfaceWeakPtr;
  using ::Star::CodexInterfaceConstWeakPtr;
  using ::Star::CodexInterfaceUPtr;
  using ::Star::CodexInterfaceConstUPtr;
  using ::Star::SongbookInterface;
  using ::Star::SongbookInterfacePtr;
  using ::Star::SongbookInterfaceConstPtr;
  using ::Star::SongbookInterfaceWeakPtr;
  using ::Star::SongbookInterfaceConstWeakPtr;
  using ::Star::SongbookInterfaceUPtr;
  using ::Star::SongbookInterfaceConstUPtr;
  using ::Star::QuestLogInterface;
  using ::Star::QuestLogInterfacePtr;
  using ::Star::QuestLogInterfaceConstPtr;
  using ::Star::QuestLogInterfaceWeakPtr;
  using ::Star::QuestLogInterfaceConstWeakPtr;
  using ::Star::QuestLogInterfaceUPtr;
  using ::Star::QuestLogInterfaceConstUPtr;
  using ::Star::AiInterface;
  using ::Star::AiInterfacePtr;
  using ::Star::AiInterfaceConstPtr;
  using ::Star::AiInterfaceWeakPtr;
  using ::Star::AiInterfaceConstWeakPtr;
  using ::Star::AiInterfaceUPtr;
  using ::Star::AiInterfaceConstUPtr;
  using ::Star::PopupInterface;
  using ::Star::PopupInterfacePtr;
  using ::Star::PopupInterfaceConstPtr;
  using ::Star::PopupInterfaceWeakPtr;
  using ::Star::PopupInterfaceConstWeakPtr;
  using ::Star::PopupInterfaceUPtr;
  using ::Star::PopupInterfaceConstUPtr;
  using ::Star::ConfirmationDialog;
  using ::Star::ConfirmationDialogPtr;
  using ::Star::ConfirmationDialogConstPtr;
  using ::Star::ConfirmationDialogWeakPtr;
  using ::Star::ConfirmationDialogConstWeakPtr;
  using ::Star::ConfirmationDialogUPtr;
  using ::Star::ConfirmationDialogConstUPtr;
  using ::Star::JoinRequestDialog;
  using ::Star::JoinRequestDialogPtr;
  using ::Star::JoinRequestDialogConstPtr;
  using ::Star::JoinRequestDialogWeakPtr;
  using ::Star::JoinRequestDialogConstWeakPtr;
  using ::Star::JoinRequestDialogUPtr;
  using ::Star::JoinRequestDialogConstUPtr;
  using ::Star::TeleportDialog;
  using ::Star::TeleportDialogPtr;
  using ::Star::TeleportDialogConstPtr;
  using ::Star::TeleportDialogWeakPtr;
  using ::Star::TeleportDialogConstWeakPtr;
  using ::Star::TeleportDialogUPtr;
  using ::Star::TeleportDialogConstUPtr;
  using ::Star::LabelWidget;
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
  using ::Star::Cinematic;
  using ::Star::CinematicPtr;
  using ::Star::CinematicConstPtr;
  using ::Star::CinematicWeakPtr;
  using ::Star::CinematicConstWeakPtr;
  using ::Star::CinematicUPtr;
  using ::Star::CinematicConstUPtr;
  using ::Star::NameplatePainter;
  using ::Star::NameplatePainterPtr;
  using ::Star::NameplatePainterConstPtr;
  using ::Star::NameplatePainterWeakPtr;
  using ::Star::NameplatePainterConstWeakPtr;
  using ::Star::NameplatePainterUPtr;
  using ::Star::NameplatePainterConstUPtr;
  using ::Star::QuestIndicatorPainter;
  using ::Star::QuestIndicatorPainterPtr;
  using ::Star::QuestIndicatorPainterConstPtr;
  using ::Star::QuestIndicatorPainterWeakPtr;
  using ::Star::QuestIndicatorPainterConstWeakPtr;
  using ::Star::QuestIndicatorPainterUPtr;
  using ::Star::QuestIndicatorPainterConstUPtr;
  using ::Star::RadioMessagePopup;
  using ::Star::RadioMessagePopupPtr;
  using ::Star::RadioMessagePopupConstPtr;
  using ::Star::RadioMessagePopupWeakPtr;
  using ::Star::RadioMessagePopupConstWeakPtr;
  using ::Star::RadioMessagePopupUPtr;
  using ::Star::RadioMessagePopupConstUPtr;
  using ::Star::Quest;
  using ::Star::QuestPtr;
  using ::Star::QuestConstPtr;
  using ::Star::QuestWeakPtr;
  using ::Star::QuestConstWeakPtr;
  using ::Star::QuestUPtr;
  using ::Star::QuestConstUPtr;
  using ::Star::QuestTrackerPane;
  using ::Star::QuestTrackerPanePtr;
  using ::Star::QuestTrackerPaneConstPtr;
  using ::Star::QuestTrackerPaneWeakPtr;
  using ::Star::QuestTrackerPaneConstWeakPtr;
  using ::Star::QuestTrackerPaneUPtr;
  using ::Star::QuestTrackerPaneConstUPtr;
  using ::Star::ContainerInteractor;
  using ::Star::ContainerInteractorPtr;
  using ::Star::ContainerInteractorConstPtr;
  using ::Star::ContainerInteractorWeakPtr;
  using ::Star::ContainerInteractorConstWeakPtr;
  using ::Star::ContainerInteractorUPtr;
  using ::Star::ContainerInteractorConstUPtr;
  using ::Star::ScriptPane;
  using ::Star::ScriptPanePtr;
  using ::Star::ScriptPaneConstPtr;
  using ::Star::ScriptPaneWeakPtr;
  using ::Star::ScriptPaneConstWeakPtr;
  using ::Star::ScriptPaneUPtr;
  using ::Star::ScriptPaneConstUPtr;
  using ::Star::ChatBubbleManager;
  using ::Star::ChatBubbleManagerPtr;
  using ::Star::ChatBubbleManagerConstPtr;
  using ::Star::ChatBubbleManagerWeakPtr;
  using ::Star::ChatBubbleManagerConstWeakPtr;
  using ::Star::ChatBubbleManagerUPtr;
  using ::Star::ChatBubbleManagerConstUPtr;
  using ::Star::CanvasWidget;
  using ::Star::CanvasWidgetPtr;
  using ::Star::CanvasWidgetConstPtr;
  using ::Star::CanvasWidgetWeakPtr;
  using ::Star::CanvasWidgetConstWeakPtr;
  using ::Star::CanvasWidgetUPtr;
  using ::Star::CanvasWidgetConstUPtr;
  using ::Star::GuiMessage;
  using ::Star::GuiMessagePtr;
  using ::Star::GuiMessageConstPtr;
  using ::Star::GuiMessageWeakPtr;
  using ::Star::GuiMessageConstWeakPtr;
  using ::Star::GuiMessageUPtr;
  using ::Star::GuiMessageConstUPtr;
  using ::Star::MainInterface;
  using ::Star::MainInterfacePtr;
  using ::Star::MainInterfaceConstPtr;
  using ::Star::MainInterfaceWeakPtr;
  using ::Star::MainInterfaceConstWeakPtr;
  using ::Star::MainInterfaceUPtr;
  using ::Star::MainInterfaceConstUPtr;
}
