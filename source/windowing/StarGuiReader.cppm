module;
#include "StarJson.hpp"

#include "StarCasting.hpp"
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.color;
import star.font;
#include "StarVariant.hpp"
#include "StarBiMap.hpp"
import star.ref_ptr;
import star.renderer;
#include "StarList.hpp"
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
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
import star.game_types;
#include "StarSet.hpp"
import star.audio;
#include "StarList.hpp"
#include "StarMap.hpp"
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

namespace Star {

struct GUIBuilderExceptionTag {
  static constexpr char const* name() { return "GUIBuilderException"; }
};
using GUIBuilderException = StarError<GUIBuilderExceptionTag, StarException>;
STAR_CLASS(GuiReader);

class GuiReader : public WidgetParser {
public:
  GuiReader();

protected:
  WidgetConstructResult titleHandler(String const& _unused, Json const& config);
  WidgetConstructResult paneFeatureHandler(String const& _unused, Json const& config);
  WidgetConstructResult backgroundHandler(String const& _unused, Json const& config);
};

}

export module star.gui_reader;

export namespace Star {
  using ::Star::GUIBuilderExceptionTag;
  using ::Star::GuiReader;
  using ::Star::GuiReaderPtr;
  using ::Star::GuiReaderConstPtr;
  using ::Star::GuiReaderWeakPtr;
  using ::Star::GuiReaderConstWeakPtr;
  using ::Star::GuiReaderUPtr;
  using ::Star::GuiReaderConstUPtr;
  using ::Star::GUIBuilderException;
}
