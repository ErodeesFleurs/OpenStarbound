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
import star.label_widget;

namespace Star {

LabelWidget::LabelWidget(String text,
    Color const& color,
    HorizontalAnchor const& hAnchor,
    VerticalAnchor const& vAnchor,
    Maybe<unsigned> wrapWidth,
    Maybe<float> lineSpacing)
  : m_hAnchor(hAnchor),
    m_vAnchor(vAnchor),
    m_wrapWidth(std::move(wrapWidth)) {
  auto assets = Root::singleton().assets();
  m_style = assets->json("/interface.config:labelTextStyle");
  m_style.color = color.toRgba();
  if (lineSpacing)
    m_style.lineSpacing = *lineSpacing;
  setText(std::move(text));
}

String const& LabelWidget::text() const {
  return m_text;
}

Maybe<unsigned> LabelWidget::getTextCharLimit() const {
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

void LabelWidget::setWrapWidth(Maybe<unsigned> wrapWidth) {
  m_wrapWidth = std::move(wrapWidth);
  updateTextRegion();
}

void LabelWidget::setLineSpacing(Maybe<float> lineSpacing) {
  m_style.lineSpacing = lineSpacing.value(DefaultLineSpacing);
  updateTextRegion();
}

void LabelWidget::setDirectives(String const& directives) {
  m_style.directives = directives;
  updateTextRegion();
}

void LabelWidget::setTextCharLimit(Maybe<unsigned> charLimit) {
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
