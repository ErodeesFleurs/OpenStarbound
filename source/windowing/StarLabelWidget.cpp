import std;
import star.json;
import star.vector;
import star.casting;
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
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;

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
import star.label_widget;

namespace Star {

LabelWidget::LabelWidget(GuiContext& context)
  : LabelWidget(context, String(), Color::White, HorizontalAnchor::LeftAnchor, VerticalAnchor::BottomAnchor, {}, {}) {}

LabelWidget::LabelWidget(GuiContext& context, String text,
    Color const& color,
    HorizontalAnchor const& hAnchor,
    VerticalAnchor const& vAnchor,
    std::optional<unsigned> wrapWidth,
    std::optional<float> lineSpacing)
  : Widget(context), m_hAnchor(hAnchor),
    m_vAnchor(vAnchor),
    m_wrapWidth(std::move(wrapWidth)) {
  auto assets = this->context()->root().assets();
  m_style = assets->json("/interface.config:labelTextStyle");
  m_style.color = color.toRgba();
  if (lineSpacing)
    m_style.lineSpacing = *lineSpacing;
  setText(std::move(text));
}

String const& LabelWidget::text() const {
  return m_text;
}

std::optional<unsigned> LabelWidget::getTextCharLimit() const {
  return m_textCharLimit;
}

void LabelWidget::setText(String newText) {
  m_text = std::move(newText);
  updateTextRegion();
}

void LabelWidget::setFontSize(int fontSize) {
  m_style.fontSize = fontSize;
  updateTextRegion();
}

void LabelWidget::setFontMode(FontMode fontMode) {
  m_style.shadow = fontModeToColor(fontMode).toRgba();
}

void LabelWidget::setColor(Color newColor) {
  m_style.color = newColor.toRgba();
}

void LabelWidget::setFont(String const& font) {
  m_style.font = font;
  updateTextRegion();
}

void LabelWidget::setAnchor(HorizontalAnchor hAnchor, VerticalAnchor vAnchor) {
  m_hAnchor = hAnchor;
  m_vAnchor = vAnchor;
  updateTextRegion();
}

void LabelWidget::setWrapWidth(std::optional<unsigned> wrapWidth) {
  m_wrapWidth = std::move(wrapWidth);
  updateTextRegion();
}

void LabelWidget::setLineSpacing(std::optional<float> lineSpacing) {
  m_style.lineSpacing = lineSpacing.value_or(DefaultLineSpacing);
  updateTextRegion();
}

void LabelWidget::setDirectives(String const& directives) {
  m_style.directives = directives;
  updateTextRegion();
}

void LabelWidget::setTextCharLimit(std::optional<unsigned> charLimit) {
  m_textCharLimit = charLimit;
  updateTextRegion();
}

void LabelWidget::setTextStyle(TextStyle const& textStyle) {
  m_style = textStyle;
  updateTextRegion();
}


RectI LabelWidget::relativeBoundRect() const {
  return RectI(m_textRegion).translated(relativePosition());
}

RectI LabelWidget::getScissorRect() const {
  return noScissor();
}

void LabelWidget::renderImpl() {
  context()->setTextStyle(m_style);
  context()->renderInterfaceText(m_text, {Vec2F(screenPosition()), m_hAnchor, m_vAnchor, m_wrapWidth, m_textCharLimit});
}

void LabelWidget::updateTextRegion() {
  context()->setTextStyle(m_style);
  m_textRegion = RectI(context()->determineInterfaceTextSize(m_text, {Vec2F(), m_hAnchor, m_vAnchor, m_wrapWidth, m_textCharLimit}));
  setSize(m_textRegion.size());
}

}
