module;
#include "StarJson.hpp"
#include "StarString.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVersion.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"


import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
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
