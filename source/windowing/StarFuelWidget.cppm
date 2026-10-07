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
