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
