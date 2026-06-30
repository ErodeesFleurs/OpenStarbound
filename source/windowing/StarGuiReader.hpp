#pragma once

#include "StarWidgetParsing.hpp"

namespace Star {

struct GUIBuilderExceptionTag { static constexpr char const* typeName = "GUIBuilderException"; };
using GUIBuilderException = TypedException<StarException, GUIBuilderExceptionTag>;
class GuiReader;
using GuiReaderPtr = SharedPtr<GuiReader>;

class GuiReader : public WidgetParser {
public:
  explicit GuiReader(GuiContext& context);

protected:
  [[nodiscard]] WidgetConstructResult titleHandler(String const&, Json const& config);
  [[nodiscard]] WidgetConstructResult paneFeatureHandler(String const&, Json const& config);
  [[nodiscard]] WidgetConstructResult backgroundHandler(String const&, Json const& config);
};

}
