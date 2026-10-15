module;
import std;
import star.json;

import star.poly;
import star.bi_map;
import star.game_types;
import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
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
  static std::shared_ptr<MainInterfaceConfig> loadFromAssets();

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
  using ::Star::MainInterfacePanes;
  using ::Star::MainInterfacePanesNames;
  using ::Star::MainInterfacePaneManager;
}
