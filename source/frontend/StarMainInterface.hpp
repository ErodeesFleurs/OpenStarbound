#pragma once

#include "StarInventory.hpp"
#include "StarInteractionTypes.hpp"
#include "StarItemDescriptor.hpp"
#include "StarGameTypes.hpp"
#include "StarInterfaceCursor.hpp"
#include "StarListener.hpp"
#include "StarMainInterfaceTypes.hpp"
#include "StarWarping.hpp"
#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarItemDatabase.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarEntityFactory.hpp"
#include "StarEntityFactory.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLiquidsDatabase.hpp"

namespace Star {

class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;
class WorldPainter;
using WorldPainterPtr = SharedPtr<WorldPainter>;
class Item;
using ItemPtr = SharedPtr<Item>;
class Chat;
using ChatPtr = SharedPtr<Chat>;
class ClientCommandProcessor;
using ClientCommandProcessorPtr = SharedPtr<ClientCommandProcessor>;
class OptionsMenu;
using OptionsMenuPtr = SharedPtr<OptionsMenu>;
class WirePane;
using WirePanePtr = SharedPtr<WirePane>;
class ActionBar;
using ActionBarPtr = SharedPtr<ActionBar>;
class ContainerPane;
using ContainerPanePtr = SharedPtr<ContainerPane>;
class CraftingPane;
using CraftingPanePtr = SharedPtr<CraftingPane>;
class MerchantPane;
using MerchantPanePtr = SharedPtr<MerchantPane>;
class CodexInterface;
using CodexInterfacePtr = SharedPtr<CodexInterface>;
class QuestLogInterface;
using QuestLogInterfacePtr = SharedPtr<QuestLogInterface>;
class PopupInterface;
using PopupInterfacePtr = SharedPtr<PopupInterface>;
class ConfirmationDialog;
using ConfirmationDialogPtr = SharedPtr<ConfirmationDialog>;
class JoinRequestDialog;
using JoinRequestDialogPtr = SharedPtr<JoinRequestDialog>;
class TeleportDialog;
using TeleportDialogPtr = SharedPtr<TeleportDialog>;
class LabelWidget;
using LabelWidgetPtr = SharedPtr<LabelWidget>;
class Cinematic;
using CinematicPtr = SharedPtr<Cinematic>;
class NameplatePainter;
using NameplatePainterPtr = SharedPtr<NameplatePainter>;
class QuestIndicatorPainter;
using QuestIndicatorPainterPtr = SharedPtr<QuestIndicatorPainter>;
class RadioMessagePopup;
using RadioMessagePopupPtr = SharedPtr<RadioMessagePopup>;
class QuestTrackerPane;
using QuestTrackerPanePtr = SharedPtr<QuestTrackerPane>;
class ContainerInteractor;
using ContainerInteractorPtr = SharedPtr<ContainerInteractor>;
class ScriptPane;
using ScriptPanePtr = SharedPtr<ScriptPane>;
class ChatBubbleManager;
using ChatBubbleManagerPtr = SharedPtr<ChatBubbleManager>;
class CanvasWidget;
class Voice;
using CanvasWidgetPtr = SharedPtr<CanvasWidget>;
class GuiContext;
class FunctionDatabase;
using FunctionDatabaseConstPtr = SharedPtr<FunctionDatabase const>;
class Input;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class AiDatabase;
using AiDatabaseConstPtr = SharedPtr<AiDatabase const>;
class TechDatabase;
using TechDatabaseConstPtr = SharedPtr<TechDatabase const>;
class StatusEffectDatabase;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;
struct FramesSpecification;
using FramesSpecificationConstPtr = SharedPtr<FramesSpecification const>;

struct GuiMessage;
using GuiMessagePtr = SharedPtr<GuiMessage>;
class MainInterface;
using MainInterfacePtr = SharedPtr<MainInterface>;

struct GuiMessage {
  GuiMessage() = default;
  GuiMessage(String const& message, float cooldown, float spring = 0);

  String message;
  float cooldown = 0.0f;
  float springState = 0.0f;
};

struct MainInterfaceServices {
  MainInterfaceServices(GuiContext& guiContext, Input& input, Voice& voice);

  GuiContext& guiContext;
  Input& input;
  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  ImageMetadataDatabaseConstPtr imageMetadata;
  FunctionDatabaseConstPtr functionDatabase;
  ItemDatabaseConstPtr itemDatabase;
  ObjectDatabaseConstPtr objectDatabase;
  AiDatabaseConstPtr aiDatabase;
  TechDatabaseConstPtr techDatabase;
  StatusEffectDatabaseConstPtr statusEffectDatabase;
  function<FramesSpecificationConstPtr(String const&)> imageFrames;
  function<void(ListenerWeakPtr)> registerReloadListener;
  function<void()> reloadRoot;
  function<void()> reloadRootForCommand;
  function<void()> hotReloadRoot;
  String outputDirectory;
  Voice& voice;
};

class MainInterface {
public:
  enum RunningState {
    Running,
    ReturnToTitle
  };

  MainInterface(UniverseClientPtr client,
      WorldPainterPtr painter,
      CinematicPtr cinematicOverlay,
      MainInterfaceServices services);

  ~MainInterface();

  [[nodiscard]] RunningState currentState() const;

  [[nodiscard]] MainInterfacePaneManager& paneManager();

  [[nodiscard]] float interfaceScale() const;

  [[nodiscard]] bool escapeDialogOpen() const;

  void openCraftingWindow(Json const& config, EntityId sourceEntityId = NullEntityId);
  void openMerchantWindow(Json const& config, EntityId sourceEntityId = NullEntityId);
  void togglePlainCraftingWindow();

  [[nodiscard]] bool windowsOpen() const;

  [[nodiscard]] observer_ptr<MerchantPane> activeMerchantPane() const;

  // Return true if this event was consumed or should be handled elsewhere.
  [[nodiscard]] bool handleInputEvent(InputEvent const& event);
  // Return true if mouse / keyboard events are currently locked here
  [[nodiscard]] bool inputFocus() const;
  // If input is focused, should MainInterface also accept text input events?
  [[nodiscard]] bool textInputActive() const;
  void handleInteractAction(InteractAction interactAction);

  void preUpdate(float dt);
  // Handles incoming client messages, aims main player, etc.
  void update(float dt);

  // Render things e.g. quest indicators that should be drawn in the world
  // behind interface e.g. chat bubbles
  void renderInWorldElements();
  void render();

  [[nodiscard]] Vec2F cursorWorldPosition() const;

  [[nodiscard]] bool isDebugDisplayed();

  void doChat(String const& chat, bool addToHistory);

  void queueMessage(String const& message, Maybe<float> cooldown, float spring);
  void queueMessage(String const& message);

  void queueItemPickupText(ItemPtr const& item);
  void queueJoinRequest(pair<String, RpcPromiseKeeper<P2PJoinRequestReply>> request);

  [[nodiscard]] bool fixedCamera() const;
  [[nodiscard]] bool hudVisible() const;
  void setHudVisible(bool visible = true);

  void warpToOrbitedWorld(bool deploy = false);
  void warpToOwnShip();
  void warpTo(WarpAction const& warpAction);

  [[nodiscard]] CanvasWidgetPtr fetchCanvas(String const& canvasName, bool ignoreInterfaceScale = false);

  [[nodiscard]] ClientCommandProcessorPtr commandProcessor() const;

  struct ScriptPaneInfo {
    observer_ptr<ScriptPane> scriptPane;
    Json config;
    EntityId sourceEntityId;
    bool visible;
    Vec2I position;
  };

  void takeScriptPanes(List<ScriptPaneInfo>& out);
  void reviveScriptPanes(List<ScriptPaneInfo>& panes);
  void displayDefaultPanes();
private:
  [[nodiscard]] UniquePtr<Pane> createEscapeDialog();
  void initHttpTrustDialog();

  [[nodiscard]] unsigned windowHeight() const;
  [[nodiscard]] unsigned windowWidth() const;
  [[nodiscard]] Vec2F mainBarPosition() const;

  void renderBreath();
  void renderMessages();
  void renderMonsterHealthBar();
  void renderSpecialDamageBar();
  void renderMainBar();
  void renderWindows();
  void renderDebug();

  void updateCursor();
  void renderCursor();

  [[nodiscard]] bool overButton(PolyI const& buttonPoly, Vec2F const& mousePos) const;

  [[nodiscard]] bool overlayClick(Vec2F const& mousePos, MouseButton mouseButton);

  void displayScriptPane(ScriptPanePtr scriptPane, EntityId sourceEntity);

  GuiContext& m_guiContext;
  Input& m_input;
  Voice& m_voice;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  ImageMetadataDatabaseConstPtr m_imageMetadata;
  FunctionDatabaseConstPtr m_functionDatabase;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  AiDatabaseConstPtr m_aiDatabase;
  TechDatabaseConstPtr m_techDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  function<FramesSpecificationConstPtr(String const&)> m_imageFrames;
  function<void(ListenerWeakPtr)> m_registerReloadListener;
  function<void()> m_reloadRoot;
  function<void()> m_reloadRootForCommand;
  function<void()> m_hotReloadRoot;
  String m_outputDirectory;
  MainInterfaceConfigConstPtr m_config;
  InterfaceCursor m_cursor;

  RunningState m_state{Running};

  UniverseClientPtr m_client;
  WorldPainterPtr m_worldPainter;
  CinematicPtr m_cinematicOverlay;

  MainInterfacePaneManager m_paneManager;

  observer_ptr<QuestLogInterface> m_questLogInterface;

  observer_ptr<InventoryPane> m_inventoryWindow;
  observer_ptr<CraftingPane> m_plainCraftingWindow;
  observer_ptr<CraftingPane> m_craftingWindow;
  observer_ptr<MerchantPane> m_merchantWindow;
  observer_ptr<CodexInterface> m_codexInterface;
  observer_ptr<OptionsMenu> m_optionsMenu;
  observer_ptr<ContainerPane> m_containerPane;
  observer_ptr<PopupInterface> m_popupInterface;
  observer_ptr<ConfirmationDialog> m_confirmationDialog;
  observer_ptr<JoinRequestDialog> m_joinRequestDialog;
  observer_ptr<TeleportDialog> m_teleportDialog;
  observer_ptr<QuestTrackerPane> m_questTracker;
  observer_ptr<ScriptPane> m_mmUpgrade;
  observer_ptr<ScriptPane> m_collections;
  Map<EntityId, ScriptPanePtr> m_interactionScriptPanes;

  StringMap<CanvasWidgetPtr> m_canvases;

  observer_ptr<Chat> m_chat;
  ClientCommandProcessorPtr m_clientCommandProcessor;
  observer_ptr<RadioMessagePopup> m_radioMessagePopup;
  observer_ptr<WirePane> m_wireInterface;

  observer_ptr<ActionBar> m_actionBar;
  Vec2F m_cursorScreenPos{};
  Vec2I m_cursorScreenIPos{};
  ItemSlotWidgetPtr m_cursorItem;
  Maybe<String> m_cursorTooltip;

  WidgetRef<LabelWidget> m_planetText;
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

  struct ItemDropMessage {
    size_t count;
    GuiMessagePtr message;
  };
  HashMap<ItemDescriptor, ItemDropMessage> m_itemDropMessages;
  unsigned m_messageOverflow{};
  GuiMessagePtr m_overflowMessage;

  struct QueuedJoinRequest {
    String playerName;
    RpcPromiseKeeper<P2PJoinRequestReply> responsePromise;
  };
  List<QueuedJoinRequest> m_queuedJoinRequests;

  EntityId m_lastMouseoverTarget{NullEntityId};
  GameTimer m_stickyTargetingTimer;
  int m_portraitScale{};

  HashMap<EntityId, float> m_specialDamageBars;

  ContainerInteractorPtr m_containerInteractor;

  MaterialDatabaseConstPtr m_materialDatabase;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  EntityFactoryConstPtr m_entityFactory;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
};

}
