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
