#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
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
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarRandom.hpp"


import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
import star.application_controller;
#include "StarVariant.hpp"
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


import star.popup_interface;
import star.widget_parsing;
import star.gui_reader;
import star.label_widget;

namespace Star {

PopupInterface::PopupInterface() {
  auto assets = Root::singleton().assets();

  GuiReader reader;

  reader.registerCallback("close", [this](Widget*) { dismiss(); });
  reader.registerCallback("ok", [this](Widget*) { dismiss(); });

  reader.construct(assets->json("/interface/windowconfig/popup.config:paneLayout"), this);
}

void PopupInterface::displayMessage(String const& message, String const& title, String const& subtitle, Maybe<String> const& onShowSound) {
  setTitleString(title, subtitle);
  fetchChild<LabelWidget>("message")->setText(message);
  show();
  auto sound = onShowSound.value(Random::randValueFrom(Root::singleton().assets()->json("/interface/windowconfig/popup.config:onShowSound").toArray(), "").toString());
  if (!sound.empty())
    context()->playAudio(sound);
}

}
