module;
import std;
import star.json;
import star.id_map;
import star.game_types;
import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.poly;
import star.asset_path;
import star.maybe;
import star.listener;
import star.set;
import star.audio;
import star.map;
#include "StarStrongTypedef.hpp"
import star.rpc_promise;
import star.ordered_map;


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
class InventoryPane;
using InventoryPanePtr = std::shared_ptr<InventoryPane>;
using InventoryPaneConstPtr = std::shared_ptr<InventoryPane const>;
using InventoryPaneWeakPtr = std::weak_ptr<InventoryPane>;
using InventoryPaneConstWeakPtr = std::weak_ptr<InventoryPane const>;
using InventoryPaneUPtr = std::unique_ptr<InventoryPane>;
using InventoryPaneConstUPtr = std::unique_ptr<InventoryPane const>;
class ItemSlotWidget;
using ItemSlotWidgetPtr = std::shared_ptr<ItemSlotWidget>;
using ItemSlotWidgetConstPtr = std::shared_ptr<ItemSlotWidget const>;
using ItemSlotWidgetWeakPtr = std::weak_ptr<ItemSlotWidget>;
using ItemSlotWidgetConstWeakPtr = std::weak_ptr<ItemSlotWidget const>;
using ItemSlotWidgetUPtr = std::unique_ptr<ItemSlotWidget>;
using ItemSlotWidgetConstUPtr = std::unique_ptr<ItemSlotWidget const>;
}

namespace Star {

class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
class WorldPainter;
using WorldPainterPtr = std::shared_ptr<WorldPainter>;
using WorldPainterConstPtr = std::shared_ptr<WorldPainter const>;
using WorldPainterWeakPtr = std::weak_ptr<WorldPainter>;
using WorldPainterConstWeakPtr = std::weak_ptr<WorldPainter const>;
using WorldPainterUPtr = std::unique_ptr<WorldPainter>;
using WorldPainterConstUPtr = std::unique_ptr<WorldPainter const>;
class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<Item const>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<Item const>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<Item const>;
class Chat;
using ChatPtr = std::shared_ptr<Chat>;
using ChatConstPtr = std::shared_ptr<Chat const>;
using ChatWeakPtr = std::weak_ptr<Chat>;
using ChatConstWeakPtr = std::weak_ptr<Chat const>;
using ChatUPtr = std::unique_ptr<Chat>;
using ChatConstUPtr = std::unique_ptr<Chat const>;
class ClientCommandProcessor;
using ClientCommandProcessorPtr = std::shared_ptr<ClientCommandProcessor>;
using ClientCommandProcessorConstPtr = std::shared_ptr<ClientCommandProcessor const>;
using ClientCommandProcessorWeakPtr = std::weak_ptr<ClientCommandProcessor>;
using ClientCommandProcessorConstWeakPtr = std::weak_ptr<ClientCommandProcessor const>;
using ClientCommandProcessorUPtr = std::unique_ptr<ClientCommandProcessor>;
using ClientCommandProcessorConstUPtr = std::unique_ptr<ClientCommandProcessor const>;
class OptionsMenu;
using OptionsMenuPtr = std::shared_ptr<OptionsMenu>;
using OptionsMenuConstPtr = std::shared_ptr<OptionsMenu const>;
using OptionsMenuWeakPtr = std::weak_ptr<OptionsMenu>;
using OptionsMenuConstWeakPtr = std::weak_ptr<OptionsMenu const>;
using OptionsMenuUPtr = std::unique_ptr<OptionsMenu>;
using OptionsMenuConstUPtr = std::unique_ptr<OptionsMenu const>;
class WirePane;
using WirePanePtr = std::shared_ptr<WirePane>;
using WirePaneConstPtr = std::shared_ptr<WirePane const>;
using WirePaneWeakPtr = std::weak_ptr<WirePane>;
using WirePaneConstWeakPtr = std::weak_ptr<WirePane const>;
using WirePaneUPtr = std::unique_ptr<WirePane>;
using WirePaneConstUPtr = std::unique_ptr<WirePane const>;
class ActionBar;
using ActionBarPtr = std::shared_ptr<ActionBar>;
using ActionBarConstPtr = std::shared_ptr<ActionBar const>;
using ActionBarWeakPtr = std::weak_ptr<ActionBar>;
using ActionBarConstWeakPtr = std::weak_ptr<ActionBar const>;
using ActionBarUPtr = std::unique_ptr<ActionBar>;
using ActionBarConstUPtr = std::unique_ptr<ActionBar const>;
class TeamBar;
using TeamBarPtr = std::shared_ptr<TeamBar>;
using TeamBarConstPtr = std::shared_ptr<TeamBar const>;
using TeamBarWeakPtr = std::weak_ptr<TeamBar>;
using TeamBarConstWeakPtr = std::weak_ptr<TeamBar const>;
using TeamBarUPtr = std::unique_ptr<TeamBar>;
using TeamBarConstUPtr = std::unique_ptr<TeamBar const>;
class StatusPane;
using StatusPanePtr = std::shared_ptr<StatusPane>;
using StatusPaneConstPtr = std::shared_ptr<StatusPane const>;
using StatusPaneWeakPtr = std::weak_ptr<StatusPane>;
using StatusPaneConstWeakPtr = std::weak_ptr<StatusPane const>;
using StatusPaneUPtr = std::unique_ptr<StatusPane>;
using StatusPaneConstUPtr = std::unique_ptr<StatusPane const>;
class ContainerPane;
using ContainerPanePtr = std::shared_ptr<ContainerPane>;
using ContainerPaneConstPtr = std::shared_ptr<ContainerPane const>;
using ContainerPaneWeakPtr = std::weak_ptr<ContainerPane>;
using ContainerPaneConstWeakPtr = std::weak_ptr<ContainerPane const>;
using ContainerPaneUPtr = std::unique_ptr<ContainerPane>;
using ContainerPaneConstUPtr = std::unique_ptr<ContainerPane const>;
class CraftingPane;
using CraftingPanePtr = std::shared_ptr<CraftingPane>;
using CraftingPaneConstPtr = std::shared_ptr<CraftingPane const>;
using CraftingPaneWeakPtr = std::weak_ptr<CraftingPane>;
using CraftingPaneConstWeakPtr = std::weak_ptr<CraftingPane const>;
using CraftingPaneUPtr = std::unique_ptr<CraftingPane>;
using CraftingPaneConstUPtr = std::unique_ptr<CraftingPane const>;
class MerchantPane;
using MerchantPanePtr = std::shared_ptr<MerchantPane>;
using MerchantPaneConstPtr = std::shared_ptr<MerchantPane const>;
using MerchantPaneWeakPtr = std::weak_ptr<MerchantPane>;
using MerchantPaneConstWeakPtr = std::weak_ptr<MerchantPane const>;
using MerchantPaneUPtr = std::unique_ptr<MerchantPane>;
using MerchantPaneConstUPtr = std::unique_ptr<MerchantPane const>;
class CodexInterface;
using CodexInterfacePtr = std::shared_ptr<CodexInterface>;
using CodexInterfaceConstPtr = std::shared_ptr<CodexInterface const>;
using CodexInterfaceWeakPtr = std::weak_ptr<CodexInterface>;
using CodexInterfaceConstWeakPtr = std::weak_ptr<CodexInterface const>;
using CodexInterfaceUPtr = std::unique_ptr<CodexInterface>;
using CodexInterfaceConstUPtr = std::unique_ptr<CodexInterface const>;
class SongbookInterface;
using SongbookInterfacePtr = std::shared_ptr<SongbookInterface>;
using SongbookInterfaceConstPtr = std::shared_ptr<SongbookInterface const>;
using SongbookInterfaceWeakPtr = std::weak_ptr<SongbookInterface>;
using SongbookInterfaceConstWeakPtr = std::weak_ptr<SongbookInterface const>;
using SongbookInterfaceUPtr = std::unique_ptr<SongbookInterface>;
using SongbookInterfaceConstUPtr = std::unique_ptr<SongbookInterface const>;
class QuestLogInterface;
using QuestLogInterfacePtr = std::shared_ptr<QuestLogInterface>;
using QuestLogInterfaceConstPtr = std::shared_ptr<QuestLogInterface const>;
using QuestLogInterfaceWeakPtr = std::weak_ptr<QuestLogInterface>;
using QuestLogInterfaceConstWeakPtr = std::weak_ptr<QuestLogInterface const>;
using QuestLogInterfaceUPtr = std::unique_ptr<QuestLogInterface>;
using QuestLogInterfaceConstUPtr = std::unique_ptr<QuestLogInterface const>;
class AiInterface;
using AiInterfacePtr = std::shared_ptr<AiInterface>;
using AiInterfaceConstPtr = std::shared_ptr<AiInterface const>;
using AiInterfaceWeakPtr = std::weak_ptr<AiInterface>;
using AiInterfaceConstWeakPtr = std::weak_ptr<AiInterface const>;
using AiInterfaceUPtr = std::unique_ptr<AiInterface>;
using AiInterfaceConstUPtr = std::unique_ptr<AiInterface const>;
class PopupInterface;
using PopupInterfacePtr = std::shared_ptr<PopupInterface>;
using PopupInterfaceConstPtr = std::shared_ptr<PopupInterface const>;
using PopupInterfaceWeakPtr = std::weak_ptr<PopupInterface>;
using PopupInterfaceConstWeakPtr = std::weak_ptr<PopupInterface const>;
using PopupInterfaceUPtr = std::unique_ptr<PopupInterface>;
using PopupInterfaceConstUPtr = std::unique_ptr<PopupInterface const>;
class ConfirmationDialog;
using ConfirmationDialogPtr = std::shared_ptr<ConfirmationDialog>;
using ConfirmationDialogConstPtr = std::shared_ptr<ConfirmationDialog const>;
using ConfirmationDialogWeakPtr = std::weak_ptr<ConfirmationDialog>;
using ConfirmationDialogConstWeakPtr = std::weak_ptr<ConfirmationDialog const>;
using ConfirmationDialogUPtr = std::unique_ptr<ConfirmationDialog>;
using ConfirmationDialogConstUPtr = std::unique_ptr<ConfirmationDialog const>;
class JoinRequestDialog;
using JoinRequestDialogPtr = std::shared_ptr<JoinRequestDialog>;
using JoinRequestDialogConstPtr = std::shared_ptr<JoinRequestDialog const>;
using JoinRequestDialogWeakPtr = std::weak_ptr<JoinRequestDialog>;
using JoinRequestDialogConstWeakPtr = std::weak_ptr<JoinRequestDialog const>;
using JoinRequestDialogUPtr = std::unique_ptr<JoinRequestDialog>;
using JoinRequestDialogConstUPtr = std::unique_ptr<JoinRequestDialog const>;
class TeleportDialog;
using TeleportDialogPtr = std::shared_ptr<TeleportDialog>;
using TeleportDialogConstPtr = std::shared_ptr<TeleportDialog const>;
using TeleportDialogWeakPtr = std::weak_ptr<TeleportDialog>;
using TeleportDialogConstWeakPtr = std::weak_ptr<TeleportDialog const>;
using TeleportDialogUPtr = std::unique_ptr<TeleportDialog>;
using TeleportDialogConstUPtr = std::unique_ptr<TeleportDialog const>;
class LabelWidget;
using LabelWidgetPtr = std::shared_ptr<LabelWidget>;
using LabelWidgetConstPtr = std::shared_ptr<LabelWidget const>;
using LabelWidgetWeakPtr = std::weak_ptr<LabelWidget>;
using LabelWidgetConstWeakPtr = std::weak_ptr<LabelWidget const>;
using LabelWidgetUPtr = std::unique_ptr<LabelWidget>;
using LabelWidgetConstUPtr = std::unique_ptr<LabelWidget const>;
class Cinematic;
using CinematicPtr = std::shared_ptr<Cinematic>;
using CinematicConstPtr = std::shared_ptr<Cinematic const>;
using CinematicWeakPtr = std::weak_ptr<Cinematic>;
using CinematicConstWeakPtr = std::weak_ptr<Cinematic const>;
using CinematicUPtr = std::unique_ptr<Cinematic>;
using CinematicConstUPtr = std::unique_ptr<Cinematic const>;
class NameplatePainter;
using NameplatePainterPtr = std::shared_ptr<NameplatePainter>;
using NameplatePainterConstPtr = std::shared_ptr<NameplatePainter const>;
using NameplatePainterWeakPtr = std::weak_ptr<NameplatePainter>;
using NameplatePainterConstWeakPtr = std::weak_ptr<NameplatePainter const>;
using NameplatePainterUPtr = std::unique_ptr<NameplatePainter>;
using NameplatePainterConstUPtr = std::unique_ptr<NameplatePainter const>;
class QuestIndicatorPainter;
using QuestIndicatorPainterPtr = std::shared_ptr<QuestIndicatorPainter>;
using QuestIndicatorPainterConstPtr = std::shared_ptr<QuestIndicatorPainter const>;
using QuestIndicatorPainterWeakPtr = std::weak_ptr<QuestIndicatorPainter>;
using QuestIndicatorPainterConstWeakPtr = std::weak_ptr<QuestIndicatorPainter const>;
using QuestIndicatorPainterUPtr = std::unique_ptr<QuestIndicatorPainter>;
using QuestIndicatorPainterConstUPtr = std::unique_ptr<QuestIndicatorPainter const>;
class RadioMessagePopup;
using RadioMessagePopupPtr = std::shared_ptr<RadioMessagePopup>;
using RadioMessagePopupConstPtr = std::shared_ptr<RadioMessagePopup const>;
using RadioMessagePopupWeakPtr = std::weak_ptr<RadioMessagePopup>;
using RadioMessagePopupConstWeakPtr = std::weak_ptr<RadioMessagePopup const>;
using RadioMessagePopupUPtr = std::unique_ptr<RadioMessagePopup>;
using RadioMessagePopupConstUPtr = std::unique_ptr<RadioMessagePopup const>;
class Quest;
using QuestPtr = std::shared_ptr<Quest>;
using QuestConstPtr = std::shared_ptr<Quest const>;
using QuestWeakPtr = std::weak_ptr<Quest>;
using QuestConstWeakPtr = std::weak_ptr<Quest const>;
using QuestUPtr = std::unique_ptr<Quest>;
using QuestConstUPtr = std::unique_ptr<Quest const>;
class QuestTrackerPane;
using QuestTrackerPanePtr = std::shared_ptr<QuestTrackerPane>;
using QuestTrackerPaneConstPtr = std::shared_ptr<QuestTrackerPane const>;
using QuestTrackerPaneWeakPtr = std::weak_ptr<QuestTrackerPane>;
using QuestTrackerPaneConstWeakPtr = std::weak_ptr<QuestTrackerPane const>;
using QuestTrackerPaneUPtr = std::unique_ptr<QuestTrackerPane>;
using QuestTrackerPaneConstUPtr = std::unique_ptr<QuestTrackerPane const>;
class ContainerInteractor;
using ContainerInteractorPtr = std::shared_ptr<ContainerInteractor>;
using ContainerInteractorConstPtr = std::shared_ptr<ContainerInteractor const>;
using ContainerInteractorWeakPtr = std::weak_ptr<ContainerInteractor>;
using ContainerInteractorConstWeakPtr = std::weak_ptr<ContainerInteractor const>;
using ContainerInteractorUPtr = std::unique_ptr<ContainerInteractor>;
using ContainerInteractorConstUPtr = std::unique_ptr<ContainerInteractor const>;
class ScriptPane;
using ScriptPanePtr = std::shared_ptr<ScriptPane>;
using ScriptPaneConstPtr = std::shared_ptr<ScriptPane const>;
using ScriptPaneWeakPtr = std::weak_ptr<ScriptPane>;
using ScriptPaneConstWeakPtr = std::weak_ptr<ScriptPane const>;
using ScriptPaneUPtr = std::unique_ptr<ScriptPane>;
using ScriptPaneConstUPtr = std::unique_ptr<ScriptPane const>;
class ChatBubbleManager;
using ChatBubbleManagerPtr = std::shared_ptr<ChatBubbleManager>;
using ChatBubbleManagerConstPtr = std::shared_ptr<ChatBubbleManager const>;
using ChatBubbleManagerWeakPtr = std::weak_ptr<ChatBubbleManager>;
using ChatBubbleManagerConstWeakPtr = std::weak_ptr<ChatBubbleManager const>;
using ChatBubbleManagerUPtr = std::unique_ptr<ChatBubbleManager>;
using ChatBubbleManagerConstUPtr = std::unique_ptr<ChatBubbleManager const>;
class CanvasWidget;
using CanvasWidgetPtr = std::shared_ptr<CanvasWidget>;
using CanvasWidgetConstPtr = std::shared_ptr<CanvasWidget const>;
using CanvasWidgetWeakPtr = std::weak_ptr<CanvasWidget>;
using CanvasWidgetConstWeakPtr = std::weak_ptr<CanvasWidget const>;
using CanvasWidgetUPtr = std::unique_ptr<CanvasWidget>;
using CanvasWidgetConstUPtr = std::unique_ptr<CanvasWidget const>;

struct GuiMessage;
using GuiMessagePtr = std::shared_ptr<GuiMessage>;
using GuiMessageConstPtr = std::shared_ptr<GuiMessage const>;
using GuiMessageWeakPtr = std::weak_ptr<GuiMessage>;
using GuiMessageConstWeakPtr = std::weak_ptr<GuiMessage const>;
using GuiMessageUPtr = std::unique_ptr<GuiMessage>;
using GuiMessageConstUPtr = std::unique_ptr<GuiMessage const>;
class MainInterface;
using MainInterfacePtr = std::shared_ptr<MainInterface>;
using MainInterfaceConstPtr = std::shared_ptr<MainInterface const>;
using MainInterfaceWeakPtr = std::weak_ptr<MainInterface>;
using MainInterfaceConstWeakPtr = std::weak_ptr<MainInterface const>;
using MainInterfaceUPtr = std::unique_ptr<MainInterface>;
using MainInterfaceConstUPtr = std::unique_ptr<MainInterface const>;

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
  void queueJoinRequest(std::pair<String, RpcPromiseKeeper<P2PJoinRequestReply>> request);

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
  HashMap<ItemDescriptor, std::pair<std::size_t, GuiMessagePtr>> m_itemDropMessages;
  unsigned m_messageOverflow{};
  GuiMessagePtr m_overflowMessage;

  List<std::pair<String, RpcPromiseKeeper<P2PJoinRequestReply>>> m_queuedJoinRequests;

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
