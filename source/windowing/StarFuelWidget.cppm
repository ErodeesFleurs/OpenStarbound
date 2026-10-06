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

STAR_CLASS(FuelWidget);

class FuelWidget : public Widget {
public:
  FuelWidget();
  virtual ~FuelWidget() {}

  virtual void update(float dt);

  void setCurrentFuelLevel(float amount);
  void setMaxFuelLevel(float amount);
  void setPotentialFuelAmount(float amount);
  void setRequestedFuelAmount(float amount);

  void ping();

protected:
  virtual void renderImpl();

  float m_fuelLevel;
  float m_maxLevel;
  float m_potential;
  float m_requested;

  float m_pingTimeout;

  TextStyle m_textStyle;

private:
};

}

export module star.fuel_widget;

export namespace Star {
  using ::Star::FuelWidget;
  using ::Star::FuelWidgetPtr;
  using ::Star::FuelWidgetConstPtr;
  using ::Star::FuelWidgetWeakPtr;
  using ::Star::FuelWidgetConstWeakPtr;
  using ::Star::FuelWidgetUPtr;
  using ::Star::FuelWidgetConstUPtr;
}
