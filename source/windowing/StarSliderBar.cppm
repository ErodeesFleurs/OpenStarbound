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
import star.button_group;
import star.button_widget;
import star.image_widget;

namespace Star {

// This class only does left right right now.  If you need advanced
// multiorientation physics please
// implement it in.
class SliderBarWidget : public Widget {
public:
  SliderBarWidget(String const& grid, bool showSpinner = true);

  void setJogImages(String const& baseImage, String const& hoverImage = "", String const& pressedImage = "", String const& disabledImage = "");

  void setRange(int low, int high, int delta);
  void setRange(Vec2I const& range, int delta);
  void setVal(int val, bool callbackIfChanged = true);
  int val() const;

  void setEnabled(bool enabled);

  void setCallback(WidgetCallbackFunc callback);

  virtual void update(float dt) override;

  virtual bool sendEvent(InputEvent const& event) override;

private:
  void leftCallback();
  void rightCallback();

  ButtonWidgetPtr m_leftButton;
  ButtonWidgetPtr m_rightButton;
  ImageWidgetPtr m_grid;
  ButtonWidgetPtr m_jog;
  int m_low;
  int m_high;
  int m_delta;
  int m_val;

  bool m_updateJog;

  Vec2I m_savedJogPos;
  Vec2I m_jogDragPos;
  bool m_jogDragActive;

  bool m_enabled;

  WidgetCallbackFunc m_callback;
};
typedef shared_ptr<SliderBarWidget> SliderBarWidgetPtr;
}

export module star.slider_bar;

export namespace Star {
  using ::Star::SliderBarWidget;
  using ::Star::SliderBarWidgetPtr;
}
