#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarInterpolation.hpp"
#include "StarGameTypes.hpp"

import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
import star.application_controller;
#include "StarVariant.hpp"
import star.renderer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
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
import star.fuel_widget;

namespace Star {

FuelWidget::FuelWidget() {
  auto assets = Root::singleton().assets();

  m_textStyle.fontSize = assets->json("/interface.config:font.buttonSize").toInt();
  m_textStyle.loadJson(assets->json("/interface.config:textStyle"));

  m_fuelLevel = 0;
  m_maxLevel = 0;
  m_potential = 0;
  m_requested = 0;

  m_pingTimeout = 0;
  disableScissoring();
}

void FuelWidget::update(float dt) {
  if ((m_pingTimeout -= dt) < 0)
    m_pingTimeout = 0;
}

void FuelWidget::renderImpl() {
  context()->resetInterfaceScissorRect();

  Vec2F textureSize = Vec2F(context()->textureSize("/interface/fuel/fuelgauge.png"));
  RectF entireTex = RectF::withSize({}, textureSize);
  RectF entirePosition = RectF::withSize(Vec2F(screenPosition()), textureSize);
  Vec2F textPosition = entirePosition.center();

  float fuel = 1;
  float fuelPotential = 1;
  float fuelRequested = 0;
  if (m_maxLevel > 0) {
    fuel = std::min(1.0f, m_fuelLevel / m_maxLevel);
    fuelPotential = std::min(1.0f, (m_fuelLevel + m_potential) / m_maxLevel);
    fuelRequested = std::min(1.0f, m_requested / m_maxLevel);
    fuel -= fuelRequested;
    if (fuel < 0)
      fuel = 0;
  }

  auto shift = [](float begin, float end, RectF templ) {
    RectF result = templ;

    result.min()[0] = lerp(begin, templ.min()[0], templ.max()[0]);
    result.max()[0] = lerp(end, templ.min()[0], templ.max()[0]);

    return result;
  };

  if (std::fmod(m_pingTimeout, 0.2f) > 0.1f)
    context()->drawInterfaceQuad("/interface/fuel/fuelgaugebackgroundflash.png", shift(0, 1, entireTex), shift(0, 1, entirePosition));
  else
    context()->drawInterfaceQuad("/interface/fuel/fuelgaugebackground.png", shift(0, 1, entireTex), shift(0, 1, entirePosition));

  context()->drawInterfaceQuad("/interface/fuel/fuelgaugegreen.png",
      shift(fuel, fuelPotential, entireTex),
      shift(fuel, fuelPotential, entirePosition));

  context()->drawInterfaceQuad("/interface/fuel/fuelgaugered.png",
      shift(fuel, fuelRequested, entireTex),
      shift(fuel, fuelRequested, entirePosition));

  context()->drawInterfaceQuad("/interface/fuel/fuelgauge.png", shift(0, fuel, entireTex), shift(0, fuel, entirePosition));
  context()->drawInterfaceQuad("/interface/fuel/fuelgaugemarkings.png", shift(0, 1, entireTex), shift(0, 1, entirePosition));

  auto& guiContext = GuiContext::singleton();
  guiContext.setTextStyle(m_textStyle);
  if (m_potential != 0) {
    guiContext.setFontColor(Color::White.toRgba());
  } else if (m_fuelLevel == 0) {
    if ((m_requested != 0) && (m_requested == m_fuelLevel))
      guiContext.setFontColor(Color::Orange.toRgba());
    else
      guiContext.setFontColor(Color::Red.toRgba());
  } else {
    guiContext.setFontColor(Color::White.toRgba());
  }

  guiContext.renderInterfaceText(strf("Fuel {}/{}", std::min(m_fuelLevel + m_potential, m_maxLevel), (int)m_maxLevel),
      {textPosition, HorizontalAnchor::HMidAnchor, VerticalAnchor::VMidAnchor});
}

void FuelWidget::setCurrentFuelLevel(float amount) {
  m_fuelLevel = amount;
}

void FuelWidget::setMaxFuelLevel(float amount) {
  m_maxLevel = amount;
}

void FuelWidget::setPotentialFuelAmount(float amount) {
  m_potential = amount;
}

void FuelWidget::setRequestedFuelAmount(float amount) {
  m_requested = amount;
}

void FuelWidget::ping() {
  m_pingTimeout = 1.0f;
}

}
