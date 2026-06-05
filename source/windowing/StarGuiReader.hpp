#pragma once

#include "StarWidgetParsing.hpp"

namespace Star {

struct GUIBuilderExceptionTag { static constexpr char const* typeName = "GUIBuilderException"; };
using GUIBuilderException = TypedException<StarException, GUIBuilderExceptionTag>;
class GuiReader;
using GuiReaderPtr = SharedPtr<GuiReader>;

class GuiReader : public WidgetParser {
public:
  GuiReader();

protected:
  WidgetConstructResult titleHandler(String const&, Json const& config);
  WidgetConstructResult paneFeatureHandler(String const&, Json const& config);
  WidgetConstructResult backgroundHandler(String const&, Json const& config);
};

}
