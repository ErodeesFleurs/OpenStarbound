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

STAR_CLASS(ImageStretchWidget);

class ImageStretchWidget : public Widget {
public:
  ImageStretchWidget(ImageStretchSet const& imageStretchSet, GuiDirection direction);
  void setImageStretchSet(String const& beginImage, String const& innerImage, String const& endImage);

  virtual ~ImageStretchWidget() {}

protected:
  virtual void renderImpl();

private:
  ImageStretchSet m_imageStretchSet;
  GuiDirection m_direction;
};

}

export module star.image_stretch_widget;

export namespace Star {
  using ::Star::ImageStretchWidget;
  using ::Star::ImageStretchWidgetPtr;
  using ::Star::ImageStretchWidgetConstPtr;
  using ::Star::ImageStretchWidgetWeakPtr;
  using ::Star::ImageStretchWidgetConstWeakPtr;
  using ::Star::ImageStretchWidgetUPtr;
  using ::Star::ImageStretchWidgetConstUPtr;
}
