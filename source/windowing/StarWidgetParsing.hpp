#pragma once

#include "StarWidget.hpp"

namespace Star {

class Widget;
using WidgetPtr = SharedPtr<Widget>;
class Pane;
using PanePtr = SharedPtr<Pane>;

struct WidgetParserExceptionTag { static constexpr char const* typeName = "WidgetParserException"; };
using WidgetParserException = TypedException<StarException, WidgetParserExceptionTag>;

struct WidgetConstructResult {
  WidgetConstructResult();
  WidgetConstructResult(WidgetPtr obj, String const& name, float zlevel);

  WidgetPtr obj;
  String name;
  float zlevel;
};

using ConstuctorFunc = std::function<WidgetConstructResult(String const& name, Json const& config)>;

class WidgetParser {
public:
  WidgetParser();
  virtual ~WidgetParser() = default;

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
