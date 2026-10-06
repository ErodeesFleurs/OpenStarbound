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

namespace Star {

STAR_CLASS(Widget);
STAR_CLASS(Pane);

struct WidgetParserExceptionTag {
  static constexpr char const* name() { return "WidgetParserException"; }
};
using WidgetParserException = StarError<WidgetParserExceptionTag, StarException>;

struct WidgetConstructResult {
  WidgetConstructResult();
  WidgetConstructResult(WidgetPtr obj, String const& name, float zlevel);

  WidgetPtr obj;
  String name;
  float zlevel;
};

typedef std::function<WidgetConstructResult(String const& name, Json const& config)> ConstuctorFunc;

class WidgetParser {
public:
  WidgetParser();
  virtual ~WidgetParser() {}

  virtual void construct(Json const& config, Widget* widget = nullptr);
  void registerCallback(String const& name, WidgetCallbackFunc callback);
  WidgetPtr makeSingle(String const& name, Json const& config);

protected:
  void constructImpl(Json const& config, Widget* widget);
  List<WidgetConstructResult> constructor(Json const& config);

  // Parents
  WidgetConstructResult stackHandler(String const& name, Json const& config);
  WidgetConstructResult scrollAreaHandler(String const& name, Json const& config);

  // Interactive
  WidgetConstructResult radioGroupHandler(String const& name, Json const& config);
  WidgetConstructResult buttonHandler(String const& name, Json const& config);
  WidgetConstructResult spinnerHandler(String const& name, Json const& config);
  WidgetConstructResult textboxHandler(String const& name, Json const& config);
  WidgetConstructResult itemSlotHandler(String const& name, Json const& config);
  WidgetConstructResult itemGridHandler(String const& name, Json const& config);
  WidgetConstructResult listHandler(String const& name, Json const& config);
  WidgetConstructResult sliderHandler(String const& name, Json const& config);
  WidgetConstructResult largeCharPlateHandler(String const& name, Json const& config);
  WidgetConstructResult tabSetHandler(String const& name, Json const& config);

  // Non-interactive
  WidgetConstructResult widgetHandler(String const& name, Json const& config);
  WidgetConstructResult imageHandler(String const& name, Json const& config);
  WidgetConstructResult imageStretchHandler(String const& name, Json const& config);
  WidgetConstructResult portraitHandler(String const& name, Json const& config);
  WidgetConstructResult labelHandler(String const& name, Json const& config);
  WidgetConstructResult canvasHandler(String const& name, Json const& config);
  WidgetConstructResult fuelGaugeHandler(String const& name, Json const& config);
  WidgetConstructResult progressHandler(String const& name, Json const& config);
  WidgetConstructResult containerHandler(String const& name, Json const& config);
  WidgetConstructResult layoutHandler(String const& name, Json const& config);

  // Utilities
  void common(WidgetPtr widget, Json const& config, bool getChildren = true);
  ImageStretchSet parseImageStretchSet(Json const& config);

  Pane* m_pane;
  StringMap<ConstuctorFunc> m_constructors;
  StringMap<WidgetCallbackFunc> m_callbacks;
};

}

export module star.widget_parsing;

export namespace Star {
  using ::Star::WidgetParserExceptionTag;
  using ::Star::WidgetConstructResult;
  using ::Star::WidgetParser;
  using ::Star::Widget;
  using ::Star::WidgetPtr;
  using ::Star::WidgetConstPtr;
  using ::Star::WidgetWeakPtr;
  using ::Star::WidgetConstWeakPtr;
  using ::Star::WidgetUPtr;
  using ::Star::WidgetConstUPtr;
  using ::Star::Pane;
  using ::Star::PanePtr;
  using ::Star::PaneConstPtr;
  using ::Star::PaneWeakPtr;
  using ::Star::PaneConstWeakPtr;
  using ::Star::PaneUPtr;
  using ::Star::PaneConstUPtr;
  using ::Star::WidgetParserException;
  using ::Star::ConstuctorFunc;
}
