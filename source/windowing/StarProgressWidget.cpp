import star.json;
import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
#include "StarConfig.hpp"
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.lexical_cast;
import star.interpolation;

import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.variant;
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
