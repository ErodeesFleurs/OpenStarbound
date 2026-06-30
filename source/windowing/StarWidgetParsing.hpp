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
  WidgetConstructResult() = default;
  WidgetConstructResult(UniquePtr<Widget> obj, String const& name, float zlevel);

  UniquePtr<Widget> obj;
  String name;
  float zlevel = 0.0f;
};

using ConstuctorFunc = std::function<WidgetConstructResult(String const& name, Json const& config)>;

class WidgetParser {
public:
  explicit WidgetParser(GuiContext& context);
  virtual ~WidgetParser() = default;

  virtual void construct(Json const& config, Widget* widget);
  void registerCallback(String const& name, WidgetCallbackFunc callback);
  [[nodiscard]] UniquePtr<Widget> makeSingle(String const& name, Json const& config);

protected:
  void constructImpl(Json const& config, Widget* widget);
  [[nodiscard]] List<WidgetConstructResult> constructor(Json const& config);

  // Parents
  [[nodiscard]] WidgetConstructResult stackHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult scrollAreaHandler(String const& name, Json const& config);

  // Interactive
  [[nodiscard]] WidgetConstructResult radioGroupHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult buttonHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult spinnerHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult textboxHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult itemSlotHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult itemGridHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult listHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult sliderHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult largeCharPlateHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult tabSetHandler(String const& name, Json const& config);

  // Non-interactive
  [[nodiscard]] WidgetConstructResult widgetHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult imageHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult imageStretchHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult portraitHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult labelHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult canvasHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult fuelGaugeHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult progressHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult containerHandler(String const& name, Json const& config);
  [[nodiscard]] WidgetConstructResult layoutHandler(String const& name, Json const& config);

  // Utilities
  void common(Widget& widget, Json const& config, bool getChildren = true);
  [[nodiscard]] ImageStretchSet parseImageStretchSet(Json const& config);
  [[nodiscard]] GuiContext& guiContext() const;

  GuiContext& m_context;
  Pane* m_pane = nullptr;
  StringMap<ConstuctorFunc> m_constructors;
  StringMap<WidgetCallbackFunc> m_callbacks;
};

}
