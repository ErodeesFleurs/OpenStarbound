module;
import std;
import star.format;
import star.json;
import star.id_map;
import star.game_types;
import star.vector;
import star.casting;
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
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.strong_typedef;
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
class ItemSlotWidget;
}

namespace Star {

class UniverseClient;
class WorldPainter;
class Item;
class Chat;
class ClientCommandProcessor;
class OptionsMenu;
class WirePane;
class ActionBar;
class TeamBar;
class StatusPane;
class ContainerPane;
class CraftingPane;
class MerchantPane;
class CodexInterface;
class SongbookInterface;
class QuestLogInterface;
class AiInterface;
class PopupInterface;
class ConfirmationDialog;
class JoinRequestDialog;
class TeleportDialog;
class LabelWidget;
class Cinematic;
class NameplatePainter;
class QuestIndicatorPainter;
class RadioMessagePopup;
class Quest;
class QuestTrackerPane;
class ContainerInteractor;
class ScriptPane;
class ChatBubbleManager;
class CanvasWidget;

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

  [[nodiscard]] static std::expected<std::shared_ptr<MainInterface>, StarException> create(GuiContext& guiContext, std::shared_ptr<UniverseClient> client, std::shared_ptr<WorldPainter> painter, std::shared_ptr<Cinematic> cinematicOverlay);

  ~MainInterface();
  [[nodiscard]] std::expected<void, StarException> close();

  RunningState currentState() const;

  MainInterfacePaneManager* paneManager();
  GuiContext& context() const;

  [[nodiscard]] std::expected<bool, StarException> escapeDialogOpen() const;

  [[nodiscard]] std::expected<void, StarException> openCraftingWindow(Json const& config, EntityId sourceEntityId = NullEntityId);
  [[nodiscard]] std::expected<void, StarException> openMerchantWindow(Json const& config, EntityId sourceEntityId = NullEntityId);
  [[nodiscard]] std::expected<void, StarException> togglePlainCraftingWindow();

  bool windowsOpen() const;

  std::shared_ptr<MerchantPane> activeMerchantPane() const;

  // Return true if this event was consumed or should be handled elsewhere.
  [[nodiscard]] std::expected<bool, StarException> handleInputEvent(InputEvent const& event);
  // Return true if mouse / keyboard events are currently locked here
  [[nodiscard]] std::expected<bool, StarException> inputFocus() const;
  // If input is focused, should MainInterface also accept text input events?
  [[nodiscard]] std::expected<bool, StarException> textInputActive() const;
  [[nodiscard]] std::expected<void, StarException> handleInteractAction(InteractAction interactAction);

  [[nodiscard]] std::expected<void, StarException> preUpdate(float dt);
  // Handles incoming client messages, aims main player, etc.
  [[nodiscard]] std::expected<void, StarException> update(float dt);

  // Render things e.g. quest indicators that should be drawn in the world
  // behind interface e.g. chat bubbles
  [[nodiscard]] std::expected<void, StarException> renderInWorldElements();
  [[nodiscard]] std::expected<void, StarException> render();

  Vec2F cursorWorldPosition() const;

  bool isDebugDisplayed();

  [[nodiscard]] std::expected<void, StarException> doChat(String const& chat, bool addToHistory);

  [[nodiscard]] std::expected<void, StarException> queueMessage(String const& message, std::optional<float> cooldown, float spring);
  [[nodiscard]] std::expected<void, StarException> queueMessage(String const& message);

  [[nodiscard]] std::expected<void, StarException> queueItemPickupText(std::shared_ptr<Item> const& item);
  [[nodiscard]] std::expected<void, StarException> queueJoinRequest(std::pair<String, RpcPromiseKeeper<P2PJoinRequestReply>> request);

  bool fixedCamera() const;
  bool hudVisible() const;
  void setHudVisible(bool visible = true);

  [[nodiscard]] std::expected<void, StarException> warpToOrbitedWorld(bool deploy = false);
  [[nodiscard]] std::expected<void, StarException> warpToOwnShip();
  [[nodiscard]] std::expected<void, StarException> warpTo(WarpAction const& warpAction);

  [[nodiscard]] std::expected<std::shared_ptr<CanvasWidget>, StarException> fetchCanvas(String const& canvasName, bool ignoreInterfaceScale = false);

  std::shared_ptr<ClientCommandProcessor> commandProcessor() const;

  struct ScriptPaneInfo {
    std::shared_ptr<ScriptPane> scriptPane;
    Json config;
    EntityId sourceEntityId;
    bool visible;
    Vec2I position;
  };

  [[nodiscard]] std::expected<void, StarException> takeScriptPanes(List<ScriptPaneInfo>& out);
  [[nodiscard]] std::expected<void, StarException> reviveScriptPanes(List<ScriptPaneInfo>& panes);
  [[nodiscard]] std::expected<void, StarException> displayDefaultPanes();
private:
  MainInterface(GuiContext& guiContext, std::shared_ptr<UniverseClient> client, std::shared_ptr<WorldPainter> painter, std::shared_ptr<Cinematic> cinematicOverlay);
  [[nodiscard]] std::expected<void, StarException> initialize();
  [[nodiscard]] std::expected<std::shared_ptr<Pane>, StarException> createEscapeDialog();
  [[nodiscard]] std::expected<void, StarException> initHttpTrustDialog();

  float interfaceScale() const;
  [[nodiscard]] std::expected<unsigned, StarException> windowHeight() const;
  [[nodiscard]] std::expected<unsigned, StarException> windowWidth() const;
  [[nodiscard]] std::expected<Vec2F, StarException> mainBarPosition() const;

  [[nodiscard]] std::expected<void, StarException> renderBreath();
  [[nodiscard]] std::expected<void, StarException> renderMessages();
  [[nodiscard]] std::expected<void, StarException> renderMonsterHealthBar();
  [[nodiscard]] std::expected<void, StarException> renderSpecialDamageBar();
  [[nodiscard]] std::expected<void, StarException> renderMainBar();
  [[nodiscard]] std::expected<void, StarException> renderWindows();
  [[nodiscard]] std::expected<void, StarException> renderDebug();

  [[nodiscard]] std::expected<void, StarException> updateCursor();
  [[nodiscard]] std::expected<void, StarException> renderCursor();

  [[nodiscard]] std::expected<bool, StarException> overButton(PolyI const& buttonPoly, Vec2F const& mousePos) const;

  [[nodiscard]] std::expected<bool, StarException> overlayClick(Vec2F const& mousePos, MouseButton mouseButton);

  [[nodiscard]] std::expected<void, StarException> displayScriptPane(std::shared_ptr<ScriptPane>& scriptPane, EntityId sourceEntity);

  GuiContext* m_guiContext{nullptr};
  std::shared_ptr<MainInterfaceConfig const> m_config;
  std::shared_ptr<InterfaceCursor> m_cursor;

  RunningState m_state{Running};

  std::shared_ptr<UniverseClient> m_client;
  std::shared_ptr<WorldPainter> m_worldPainter;
  std::shared_ptr<Cinematic> m_cinematicOverlay;

  MainInterfacePaneManager m_paneManager;

  std::shared_ptr<QuestLogInterface> m_questLogInterface;

  std::shared_ptr<InventoryPane> m_inventoryWindow;
  std::shared_ptr<CraftingPane> m_plainCraftingWindow;
  std::shared_ptr<CraftingPane> m_craftingWindow;
  std::shared_ptr<MerchantPane> m_merchantWindow;
  std::shared_ptr<CodexInterface> m_codexInterface;
  std::shared_ptr<OptionsMenu> m_optionsMenu;
  std::shared_ptr<ContainerPane> m_containerPane;
  std::shared_ptr<PopupInterface> m_popupInterface;
  std::shared_ptr<ConfirmationDialog> m_confirmationDialog;
  std::shared_ptr<JoinRequestDialog> m_joinRequestDialog;
  std::shared_ptr<TeleportDialog> m_teleportDialog;
  std::shared_ptr<QuestTrackerPane> m_questTracker;
  std::shared_ptr<ScriptPane> m_mmUpgrade;
  std::shared_ptr<ScriptPane> m_collections;
  Map<EntityId, std::shared_ptr<Pane>> m_interactionScriptPanes;

  StringMap<std::shared_ptr<CanvasWidget>> m_canvases;

  std::shared_ptr<Chat> m_chat;
  std::shared_ptr<ClientCommandProcessor> m_clientCommandProcessor;
  std::shared_ptr<RadioMessagePopup> m_radioMessagePopup;
  std::shared_ptr<WirePane> m_wireInterface;

  std::shared_ptr<ActionBar> m_actionBar;
  Vec2F m_cursorScreenPos{};
  Vec2I m_cursorScreenIPos{};
  std::shared_ptr<ItemSlotWidget> m_cursorItem;
  std::optional<String> m_cursorTooltip;

  std::shared_ptr<LabelWidget> m_planetText;
  GameTimer m_planetNameTimer;

  GameTimer m_debugSpatialClearTimer;
  GameTimer m_debugMapClearTimer;
  RectF m_debugTextRect{RectF::null()};

  std::shared_ptr<NameplatePainter> m_nameplatePainter;
  std::shared_ptr<QuestIndicatorPainter> m_questIndicatorPainter;
  std::shared_ptr<ChatBubbleManager> m_chatBubbleManager;

  bool m_disableHud = false;

  String m_lastCommand;

  LinkedList<std::shared_ptr<GuiMessage>> m_messages;
  HashMap<ItemDescriptor, std::pair<std::size_t, std::shared_ptr<GuiMessage>>> m_itemDropMessages;
  unsigned m_messageOverflow{};
  std::shared_ptr<GuiMessage> m_overflowMessage;

  List<std::pair<String, RpcPromiseKeeper<P2PJoinRequestReply>>> m_queuedJoinRequests;

  EntityId m_lastMouseoverTarget{NullEntityId};
  GameTimer m_stickyTargetingTimer;
  int m_portraitScale{};

  HashMap<EntityId, float> m_specialDamageBars;

  std::shared_ptr<ContainerInteractor> m_containerInteractor;
};

}

export module star.main_interface;

export namespace Star {
  using ::Star::UniverseClient;
  using ::Star::WorldPainter;
  using ::Star::Item;
  using ::Star::Chat;
  using ::Star::ClientCommandProcessor;
  using ::Star::OptionsMenu;
  using ::Star::WirePane;
  using ::Star::ActionBar;
  using ::Star::TeamBar;
  using ::Star::StatusPane;
  using ::Star::ContainerPane;
  using ::Star::CraftingPane;
  using ::Star::MerchantPane;
  using ::Star::CodexInterface;
  using ::Star::SongbookInterface;
  using ::Star::QuestLogInterface;
  using ::Star::AiInterface;
  using ::Star::PopupInterface;
  using ::Star::ConfirmationDialog;
  using ::Star::JoinRequestDialog;
  using ::Star::TeleportDialog;
  using ::Star::LabelWidget;
  using ::Star::Cinematic;
  using ::Star::NameplatePainter;
  using ::Star::QuestIndicatorPainter;
  using ::Star::RadioMessagePopup;
  using ::Star::Quest;
  using ::Star::QuestTrackerPane;
  using ::Star::ContainerInteractor;
  using ::Star::ScriptPane;
  using ::Star::ChatBubbleManager;
  using ::Star::CanvasWidget;
  using ::Star::GuiMessage;
  using ::Star::MainInterface;
}
