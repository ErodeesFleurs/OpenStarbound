module;
import std;
import star.format;
import star.json;

import star.casting;
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
import star.bi_map;
import star.ref_ptr;
import star.renderer;
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
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
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

class Widget;
class Pane;

struct WidgetParserExceptionTag {
  static constexpr char const* name() { return "WidgetParserException"; }
};
using WidgetParserException = StarError<WidgetParserExceptionTag, StarException>;

struct WidgetConstructResult {
  WidgetConstructResult();
  WidgetConstructResult(std::shared_ptr<Widget> obj, String const& name, float zlevel);

  std::shared_ptr<Widget> obj;
  String name;
  float zlevel;
};

typedef std::function<WidgetConstructResult(String const& name, Json const& config)> ConstuctorFunc;

class WidgetParser {
public:
  WidgetParser(GuiContext& context);
  virtual ~WidgetParser() {}

  virtual void construct(Json const& config, Widget* widget = nullptr);
  void registerCallback(String const& name, WidgetCallbackFunc callback);
  std::shared_ptr<Widget> makeSingle(String const& name, Json const& config);

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
  void common(std::shared_ptr<Widget> widget, Json const& config, bool getChildren = true);
  ImageStretchSet parseImageStretchSet(Json const& config);

  GuiContext* m_context;
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
  using ::Star::Pane;
  using ::Star::WidgetParserException;
  using ::Star::ConstuctorFunc;
}
