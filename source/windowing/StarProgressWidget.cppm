module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
#include "StarRect.hpp"
import star.application_controller;
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarVariant.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarRefPtr.hpp"
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
#include "StarPoly.hpp"
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
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

STAR_CLASS(ProgressWidget);
class ProgressWidget : public Widget {
public:
  ProgressWidget(String const& background,
      String const& overlay,
      ImageStretchSet const& progressSet,
      GuiDirection direction);
  virtual ~ProgressWidget() {}

  void setCurrentProgressLevel(float amount);
  void setMaxProgressLevel(float amount);

  void setColor(Color const& color);
  void setOverlay(String const& overlay);

protected:
  virtual void renderImpl();
  RectI shift(float begin, float end, RectI templ);

  float m_progressLevel;
  float m_maxLevel;

  Color m_color;

  String m_background;
  String m_overlay;
  ImageStretchSet m_bar;
  GuiDirection m_direction;

private:
};

}

export module star.progress_widget;

export namespace Star {
  using ::Star::ProgressWidget;
  using ::Star::ProgressWidgetPtr;
  using ::Star::ProgressWidgetConstPtr;
  using ::Star::ProgressWidgetWeakPtr;
  using ::Star::ProgressWidgetConstWeakPtr;
  using ::Star::ProgressWidgetUPtr;
  using ::Star::ProgressWidgetConstUPtr;
}
