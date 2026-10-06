module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
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
