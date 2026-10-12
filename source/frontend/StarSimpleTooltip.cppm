module;
#include "StarJson.hpp"
#include "StarString.hpp"
#include "StarOrderedMap.hpp"
#include "StarIODevice.hpp"
import star.ref_ptr;
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.font;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
import star.text;
#include "StarDataStream.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
import star.game_types;
#include "StarSet.hpp"
import star.audio;
#include "StarList.hpp"
#include "StarMap.hpp"


import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
#include "StarVariant.hpp"
import star.renderer;
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
import star.widget_parsing;
import star.gui_reader;
import star.pane;

export module star.simple_tooltip;

export namespace Star::SimpleTooltipBuilder {
  PanePtr buildTooltip(String const& text);
}

namespace Star {

PanePtr SimpleTooltipBuilder::buildTooltip(String const& text) {
  PanePtr tooltip = make_shared<Pane>();
  tooltip->removeAllChildren();
  GuiReader reader;
  reader.construct(Root::singleton().assets()->json("/interface/tooltips/simpletooltip.tooltip"), tooltip.get());
  tooltip->setLabel("contentLabel", text);

  auto stretchBackground = tooltip->fetchChild<Widget>("stretchBackground");
  stretchBackground->setSize(Vec2I{tooltip->fetchChild<Widget>("contentLabel")->size()[0] + 8, stretchBackground->size()[1]});
  tooltip->setSize(stretchBackground->size());

  return tooltip;
}

}
