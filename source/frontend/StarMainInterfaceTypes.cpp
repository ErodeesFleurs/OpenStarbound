import std;
import star.format;
import star.json;
import star.json_extra;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.bi_map;
import star.game_types;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.version;
import star.string;
import star.text;
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.ordered_map;


import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.renderer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
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


import star.image_metadata_database;

namespace Star {

extern std::expected<EnumMap<MainInterfacePanes>, StarException> const MainInterfacePanesNames = EnumMap<MainInterfacePanes>::create({
  {MainInterfacePanes::EscapeDialog, "EscapeDialog"},
  {MainInterfacePanes::Inventory, "Inventory"},
  {MainInterfacePanes::Cockpit, "Cockpit"},
  {MainInterfacePanes::Tech, "Tech"},
  {MainInterfacePanes::Ai, "Ai"},
  {MainInterfacePanes::Confirmation, "Confirmation"},
  {MainInterfacePanes::HttpTrustDialog, "HttpTrustDialog"},
  {MainInterfacePanes::JoinRequest, "JoinRequest"},
  {MainInterfacePanes::Options, "Options"},
  {MainInterfacePanes::QuestLog, "QuestLog"},
  {MainInterfacePanes::ActionBar, "ActionBar"},
  {MainInterfacePanes::TeamBar, "TeamBar"},
  {MainInterfacePanes::WireInterface, "WireInterface"},
  {MainInterfacePanes::PlanetText, "PlanetText"},
  {MainInterfacePanes::RadioMessagePopup, "RadioMessagePopup"},
  {MainInterfacePanes::CraftingPlain, "CraftingPlain"},
  {MainInterfacePanes::MmUpgrade, "MmUpgrade"},
  {MainInterfacePanes::Collections, "Collections"},
  {MainInterfacePanes::CharacterSwap, "CharacterSwap"}
});
std::expected<std::unique_ptr<MainInterfaceConfig>, StarException> MainInterfaceConfig::loadFromAssets(GuiContext& context) {
  auto& root = context.root();
  auto assets = root.assets();
  if (!assets) return std::unexpected(std::move(assets.error()));


  auto config = std::make_unique<MainInterfaceConfig>();

  {
    auto json = (*assets)->json("/interface.config:textStyle");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = TextStyle::create(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->textStyle = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.inventory.base");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->inventoryImage = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.inventory.hover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->inventoryImageHover = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.inventory.glow");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->inventoryImageGlow = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.inventory.glowHover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->inventoryImageGlowHover = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.inventory.open");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->inventoryImageOpen = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.inventory.openHover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->inventoryImageOpenHover = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:mainBar.beam.base");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->beamDownImage = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.beam.hover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->beamDownImageHover = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:mainBar.deploy.base");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->deployImage = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.deploy.hover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->deployImageHover = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.deploy.disabled");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->deployImageDisabled = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.beamUp.base");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->beamUpImage = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.beamUp.hover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->beamUpImageHover = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:mainBar.craft.base");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->craftImage = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.craft.hover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->craftImageHover = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.craft.open");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->craftImageOpen = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.craft.openHover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->craftImageOpenHover = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:mainBar.codex.base");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->codexImage = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.codex.hover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->codexImageHover = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.codex.open");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->codexImageOpen = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.codex.openHover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->codexImageHoverOpen = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:mainBar.questLog.base");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->questLogImage = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.questLog.hover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->questLogImageHover = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.questLog.open");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->questLogImageOpen = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.questLog.openHover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->questLogImageHoverOpen = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:mainBar.mmUpgrade.base");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->mmUpgradeImage = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.mmUpgrade.hover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->mmUpgradeImageHover = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.mmUpgrade.open");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->mmUpgradeImageOpen = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.mmUpgrade.openHover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->mmUpgradeImageHoverOpen = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.mmUpgrade.disabled");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->mmUpgradeImageDisabled = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:mainBar.collections.base");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->collectionsImage = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.collections.hover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->collectionsImageHover = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.collections.open");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->collectionsImageOpen = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.collections.openHover");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->collectionsImageHoverOpen = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:mainBar.inventory.pos");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarInventoryButtonOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.craft.pos");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarCraftButtonOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.beam.pos");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarBeamButtonOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.deploy.pos");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarDeployButtonOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.codex.pos");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarCodexButtonOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.questLog.pos");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarQuestLogButtonOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.mmUpgrade.pos");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarMmUpgradeButtonOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.collections.pos");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarCollectionsButtonOffset = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:mainBar.inventory.poly");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToPolyI(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarInventoryButtonPoly = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.craft.poly");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToPolyI(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarCraftButtonPoly = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.beam.poly");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToPolyI(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarBeamButtonPoly = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.deploy.poly");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToPolyI(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarDeployButtonPoly = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.codex.poly");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToPolyI(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarCodexButtonPoly = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.questLog.poly");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToPolyI(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarQuestLogButtonPoly = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.mmUpgrade.poly");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToPolyI(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarMmUpgradeButtonPoly = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:mainBar.collections.poly");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToPolyI(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarCollectionsButtonPoly = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:mainBar.size");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->mainBarSize = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:itemCountRightAnchor");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->itemCountRightAnchor = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:inventoryItemMouseOffset");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->inventoryItemMouseOffset = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:maxMessageCount");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toUInt();
    if (!value) return std::unexpected(std::move(value.error()));
    config->maxMessageCount = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:overflowMessageText");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->overflowMessageText = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:message.barPos");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageBarPos = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:message.itemOffset");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageItemOffset = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:message.textContainer");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageTextContainer = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:message.textContainerOffset");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageTextContainerOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:message.textOffset");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageTextOffset = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:message.showTime");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toFloat();
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageTime = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:message.hideTime");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toFloat();
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageHideTime = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:message.offset");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageActiveOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:message.offsetHidden");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageHiddenOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:message.offsetHiddenBar");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageHiddenOffsetBar = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:message.windowSpring");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toFloat();
    if (!value) return std::unexpected(std::move(value.error()));
    config->messageWindowSpring = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:monsterHealth.showTime");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toFloat();
    if (!value) return std::unexpected(std::move(value.error()));
    config->monsterHealthBarTime = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:hungerIcon");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->hungerIcon = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:planetNameTime");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toFloat();
    if (!value) return std::unexpected(std::move(value.error()));
    config->planetNameTime = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:planetNameFadeTime");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toFloat();
    if (!value) return std::unexpected(std::move(value.error()));
    config->planetNameFadeTime = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:planetNameFormatString");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toString();
    if (!value) return std::unexpected(std::move(value.error()));
    config->planetNameFormatString = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:planetTextStyle");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = TextStyle::create(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->planetNameTextStyle = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:planetTextOffset");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->planetNameOffset = std::move(*value);
  }

  {
    auto json = (*assets)->json("/interface.config:renderVirtualCursor");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toBool();
    if (!value) return std::unexpected(std::move(value.error()));
    config->renderVirtualCursor = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:cursorItemSlot");
    if (!json) return std::unexpected(std::move(json.error()));
    config->cursorItemSlot = std::move(*json);
  }

  {
    auto json = (*assets)->json("/interface.config:debugOffset");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->debugOffset = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:debugTextStyle");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = TextStyle::create(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->debugTextStyle = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:debugSpatialClearTime");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toFloat();
    if (!value) return std::unexpected(std::move(value.error()));
    config->debugSpatialClearTime = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:debugMapClearTime");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toFloat();
    if (!value) return std::unexpected(std::move(value.error()));
    config->debugMapClearTime = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:debugBackgroundColor");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToColor(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    config->debugBackgroundColor = std::move(*value);
  }
  {
    auto json = (*assets)->json("/interface.config:debugBackgroundPad");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = json->toUInt();
    if (!value) return std::unexpected(std::move(value.error()));
    config->debugBackgroundPad = std::move(*value);
  }

  auto macros = (*assets)->scanExtension("macros");
  if (!macros) return std::unexpected(std::move(macros.error()));
  for (auto const& path : macros->get()) {
    auto json = (*assets)->json(path.string());
    if (!json) return std::unexpected(std::move(json.error()));
    auto entries = json->iterateObject();
    if (!entries) return std::unexpected(std::move(entries.error()));
    for (auto const& pair : *entries) {
      auto commands = jsonToStringList(pair.second);
      if (!commands) return std::unexpected(std::move(commands.error()));
      auto added = config->macroCommands.add(pair.first, std::move(*commands));
      if (!added) return std::unexpected(std::move(added.error()));
    }
  }

  return config;
}

}
