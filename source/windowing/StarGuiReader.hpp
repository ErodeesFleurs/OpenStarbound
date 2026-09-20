#pragma once

#include "StarWidgetParsing.hpp"

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
