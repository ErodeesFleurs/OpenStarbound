module;

#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"
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
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarOrderedMap.hpp"


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

namespace Star {

STAR_STRUCT(MainInterfaceConfig);

enum class MainInterfacePanes {
  EscapeDialog,
  Inventory,
  Codex,
  Cockpit,
  Tech,
  Songbook,
  Ai,
  Popup,
  Confirmation,
  HttpTrustDialog, // sorry i need it here
  JoinRequest,
  Options,
  QuestLog,
  ActionBar,
  TeamBar,
  StatusPane,
  Chat,
  WireInterface,
  PlanetText,
  RadioMessagePopup,
  CraftingPlain,
  QuestTracker,
  MmUpgrade,
  Collections,
  CharacterSwap
};

extern EnumMap<MainInterfacePanes> const MainInterfacePanesNames;

typedef RegisteredPaneManager<MainInterfacePanes> MainInterfacePaneManager;

struct MainInterfaceConfig {
  static MainInterfaceConfigPtr loadFromAssets();

  TextStyle textStyle;

  String inventoryImage;
  String inventoryImageHover;
  String inventoryImageGlow;
  String inventoryImageGlowHover;
  String inventoryImageOpen;
  String inventoryImageOpenHover;

  String beamDownImage;
  String beamDownImageHover;

  String deployImage;
  String deployImageHover;
  String deployImageDisabled;
  String beamUpImage;
  String beamUpImageHover;

  String craftImage;
  String craftImageHover;
  String craftImageOpen;
  String craftImageOpenHover;

  String codexImage;
  String codexImageHover;
  String codexImageOpen;
  String codexImageHoverOpen;

  String questLogImage;
  String questLogImageHover;
  String questLogImageOpen;
  String questLogImageHoverOpen;

  String mmUpgradeImage;
  String mmUpgradeImageHover;
  String mmUpgradeImageOpen;
  String mmUpgradeImageHoverOpen;
  String mmUpgradeImageDisabled;

  String collectionsImage;
  String collectionsImageHover;
  String collectionsImageOpen;
  String collectionsImageHoverOpen;
  String collectionsImageDisabled;

  Vec2I mainBarInventoryButtonOffset;
  Vec2I mainBarCraftButtonOffset;
  Vec2I mainBarCodexButtonOffset;
  Vec2I mainBarBeamButtonOffset;
  Vec2I mainBarDeployButtonOffset;
  Vec2I mainBarQuestLogButtonOffset;
  Vec2I mainBarMmUpgradeButtonOffset;
  Vec2I mainBarCollectionsButtonOffset;

  PolyI mainBarInventoryButtonPoly;
  PolyI mainBarCraftButtonPoly;
  PolyI mainBarCodexButtonPoly;
  PolyI mainBarBeamButtonPoly;
  PolyI mainBarDeployButtonPoly;
  PolyI mainBarQuestLogButtonPoly;
  PolyI mainBarMmUpgradeButtonPoly;
  PolyI mainBarCollectionsButtonPoly;

  PolyI mainBarPoly;
  Vec2I mainBarSize;

  Vec2I itemCountRightAnchor;
  Vec2I inventoryItemMouseOffset;

  unsigned maxMessageCount;
  String overflowMessageText;

  Vec2I messageBarPos;
  Vec2I messageItemOffset;

  String messageTextContainer;
  Vec2I messageTextContainerOffset;
  Vec2I messageTextOffset;

  float messageTime;
  float messageHideTime;
  Vec2I messageActiveOffset;
  Vec2I messageHiddenOffset;
  Vec2I messageHiddenOffsetBar;
  float messageWindowSpring;
  float monsterHealthBarTime;

  String hungerIcon;

  float planetNameTime;
  float planetNameFadeTime;
  String planetNameFormatString;
  TextStyle planetNameTextStyle;
  Vec2I planetNameOffset;

  bool renderVirtualCursor;
  Json cursorItemSlot;

  Vec2I debugOffset;
  TextStyle debugTextStyle;
  float debugSpatialClearTime;
  float debugMapClearTime;
  Color debugBackgroundColor;
  int debugBackgroundPad;

  StringMap<StringList> macroCommands;
};

}

export module star.main_interface_types;

export namespace Star {
  using ::Star::MainInterfaceConfig;
  using ::Star::MainInterfaceConfigPtr;
  using ::Star::MainInterfaceConfigConstPtr;
  using ::Star::MainInterfaceConfigWeakPtr;
  using ::Star::MainInterfaceConfigConstWeakPtr;
  using ::Star::MainInterfaceConfigUPtr;
  using ::Star::MainInterfaceConfigConstUPtr;
  using ::Star::MainInterfacePanes;
  using ::Star::MainInterfacePanesNames;
  using ::Star::MainInterfacePaneManager;
}
