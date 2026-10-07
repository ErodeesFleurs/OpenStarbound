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
#include "StarLexicalCast.hpp"
#include "StarInterpolation.hpp"

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
import star.progress_widget;

namespace Star {

ProgressWidget::ProgressWidget(String const& background,
    String const& overlay,
    ImageStretchSet const& progressSet,
    GuiDirection direction)
  : m_background(background),
    m_overlay(overlay),
    m_bar(progressSet),
    m_direction(direction) {

  m_progressLevel = 0;
  m_maxLevel = 1;

  if (!m_background.empty())
    setSize(Vec2I(context()->textureSize(m_background)));
  else if (!m_overlay.empty())
    setSize(Vec2I(context()->textureSize(m_overlay)));

  m_color = Color::White;
}

void ProgressWidget::renderImpl() {
  float progress = 1;
  if (m_maxLevel > 0)
    progress = m_progressLevel / m_maxLevel;

  auto shift = [&](float begin, float end, RectF templ) {
    RectF result = templ;

    if (m_direction == GuiDirection::Horizontal) {
      result.min()[0] = lerp(begin, templ.min()[0], templ.max()[0]);
      result.max()[0] = lerp(end, templ.min()[0], templ.max()[0]);
    } else {
      result.min()[1] = lerp(begin, templ.min()[1], templ.max()[1]);
      result.max()[1] = lerp(end, templ.min()[1], templ.max()[1]);
    }

    return result;
  };

  if (!m_background.empty())
    context()->drawInterfaceQuad(m_background, shift(0, 1, RectF(Vec2F(), Vec2F(size()))), shift(0, 1, RectF(screenBoundRect())));

  context()->drawImageStretchSet(m_bar, shift(0, progress, RectF(screenBoundRect())), m_direction, m_color.toRgba());

  if (!m_overlay.empty())
    context()->drawInterfaceQuad(m_overlay, shift(0, 1, RectF({}, Vec2F(size()))), shift(0, 1, RectF(screenBoundRect())));
}

void ProgressWidget::setCurrentProgressLevel(float amount) {
  m_progressLevel = amount;
}

void ProgressWidget::setMaxProgressLevel(float amount) {
  m_maxLevel = amount;
}

void ProgressWidget::setColor(Color const& color) {
  m_color = color;
}

void ProgressWidget::setOverlay(String const& overlay) {
  m_overlay = overlay;
}

}
