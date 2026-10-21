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
import star.poly;
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
import star.canvas_widget;

import star.math_common;
import star.algorithm;
namespace Star {

CanvasWidget::CanvasWidget(GuiContext& context) : Widget(context) {
  m_ignoreInterfaceScale = m_captureKeyboard = m_captureMouse = false;
}

void CanvasWidget::setCaptureMouseEvents(bool captureMouse) {
  m_captureMouse = captureMouse;
}

void CanvasWidget::setCaptureKeyboardEvents(bool captureKeyboard) {
  m_captureKeyboard = captureKeyboard;
}

void CanvasWidget::setIgnoreInterfaceScale(bool ignoreInterfaceScale) {
  m_ignoreInterfaceScale = ignoreInterfaceScale;
}

bool CanvasWidget::ignoreInterfaceScale() const {
  return m_ignoreInterfaceScale;
}

void CanvasWidget::clear() {
  m_renderOps.clear();
}

void CanvasWidget::drawImage(String texName, Vec2F const& position, float scale, Vec4B const& color) {
  m_renderOps.append(std::make_tuple(std::move(texName), position, scale, color, false));
}

void CanvasWidget::drawImageCentered(String texName, Vec2F const& position, float scale, Vec4B const& color) {
  m_renderOps.append(std::make_tuple(std::move(texName), position, scale, color, true));
}

void CanvasWidget::drawImageRect(String texName, RectF const& texCoords, RectF const& screenCoords, Vec4B const& color) {
  m_renderOps.append(std::make_tuple(std::move(texName), texCoords, screenCoords, color));
}

void CanvasWidget::drawDrawable(Drawable drawable, Vec2F const& screenPos) {
  m_renderOps.append(std::make_tuple(std::move(drawable), screenPos));
}

void CanvasWidget::drawDrawables(List<Drawable> drawables, Vec2F const& screenPos) {
  for (auto& drawable : drawables)
    drawDrawable(std::move(drawable), screenPos);
}

void CanvasWidget::drawTiledImage(String texName, float textureScale, Vec2D const& offset, RectF const& screenCoords, Vec4B const& color) {
  m_renderOps.append(std::make_tuple(std::move(texName), textureScale, offset, screenCoords, color));
}

void CanvasWidget::drawLine(Vec2F const& begin, Vec2F const end, Vec4B const& color, float lineWidth) {
  m_renderOps.append(std::make_tuple(begin, end, color, lineWidth));
}

void CanvasWidget::drawRect(RectF const& coords, Vec4B const& color) {
  m_renderOps.append(std::make_tuple(coords, color));
}

void CanvasWidget::drawPoly(PolyF const& poly, Vec4B const& color, float lineWidth) {
  m_renderOps.append(std::make_tuple(poly, color, lineWidth));
}

void CanvasWidget::drawTriangles(List<std::tuple<Vec2F, Vec2F, Vec2F>> const& triangles, Vec4B const& color) {
  m_renderOps.append(std::make_tuple(triangles, color));
}

void CanvasWidget::drawText(String s, TextPositioning position, unsigned fontSize, Vec4B const& color, FontMode mode, float lineSpacing, String font, String processingDirectives) {
  TextStyle style;
  style.fontSize = fontSize;
  style.color = color;
  style.shadow = fontModeToColor(mode).toRgba();
  style.lineSpacing = lineSpacing;
  style.font = font;
  style.directives = processingDirectives;
  m_renderOps.append(std::make_tuple(std::move(s), std::move(position), std::move(style)));
}

void CanvasWidget::drawText(String s, TextPositioning position, TextStyle style) {
  m_renderOps.append(std::make_tuple(std::move(s), std::move(position), std::move(style)));
}

Vec2I CanvasWidget::mousePosition() const {
  return m_mousePosition;
}

List<CanvasWidget::ClickEvent> CanvasWidget::pullClickEvents() {
  return take(m_clickEvents);
}

List<CanvasWidget::KeyEvent> CanvasWidget::pullKeyEvents() {
  return take(m_keyEvents);
}

bool CanvasWidget::sendEvent(InputEvent const& event) {
  if (!m_visible)
    return false;

  auto& context = (*this->context());
  auto interfaceScale = m_ignoreInterfaceScale ? 1 : context.interfaceScale();
  if (auto mouseButtonDown = std::get_if<MouseButtonDownEvent>(&event)) {
    if (inMember(*context.mousePosition(event, interfaceScale)) && m_captureMouse) {
      m_clickEvents.append({*context.mousePosition(event, interfaceScale) - screenPosition(), mouseButtonDown->mouseButton, true});
      m_clickEvents.limitSizeBack(MaximumEventBuffer);
      return true;
    }
  } else if (auto mouseButtonUp = std::get_if<MouseButtonUpEvent>(&event)) {
    if (m_captureMouse) {
      m_clickEvents.append({*context.mousePosition(event, interfaceScale) - screenPosition(), mouseButtonUp->mouseButton, false});
      m_clickEvents.limitSizeBack(MaximumEventBuffer);
      return true;
    }
  } else if (std::holds_alternative<MouseMoveEvent>(event)) {
    m_mousePosition = *context.mousePosition(event, interfaceScale) - screenPosition();
    return false;
  } else if (auto keyDown = std::get_if<KeyDownEvent>(&event)) {
    if (m_captureKeyboard) {
      m_keyEvents.append({keyDown->key, true});
      return true;
    }
  } else if (auto keyUp = std::get_if<KeyUpEvent>(&event)) {
    if (m_captureKeyboard) {
      m_keyEvents.append({keyUp->key, false});
      m_keyEvents.limitSizeBack(MaximumEventBuffer);
      return true;
    }
  }

  return Widget::sendEvent(event);
}

KeyboardCaptureMode CanvasWidget::keyboardCaptureMode() const {
  return m_captureKeyboard ? KeyboardCaptureMode::KeyEvents : KeyboardCaptureMode::None;
}

void CanvasWidget::renderImpl() {
  auto renderingOffset = Vec2F(screenPosition());

  for (auto const& op : m_renderOps) {
    if (auto args = std::get_if<ImageOp>(&op))
      std::apply(std::bind_front(&CanvasWidget::renderImage, this, renderingOffset), *args);
    if (auto args = std::get_if<ImageRectOp>(&op))
      std::apply(std::bind_front(&CanvasWidget::renderImageRect, this, renderingOffset), *args);
    if (auto args = std::get_if<DrawableOp>(&op))
      std::apply(std::bind_front(&CanvasWidget::renderDrawable, this, renderingOffset), *args);
    if (auto args = std::get_if<TiledImageOp>(&op))
      std::apply(std::bind_front(&CanvasWidget::renderTiledImage, this, renderingOffset), *args);
    if (auto args = std::get_if<LineOp>(&op))
      std::apply(std::bind_front(&CanvasWidget::renderLine, this, renderingOffset), *args);
    if (auto args = std::get_if<RectOp>(&op))
      std::apply(std::bind_front(&CanvasWidget::renderRect, this, renderingOffset), *args);
    if (auto args = std::get_if<PolyOp>(&op))
      std::apply(std::bind_front(&CanvasWidget::renderPoly, this, renderingOffset), *args);
    if (auto args = std::get_if<TrianglesOp>(&op))
      std::apply(std::bind_front(&CanvasWidget::renderTriangles, this, renderingOffset), *args);
    if (auto args = std::get_if<TextOp>(&op))
      std::apply(std::bind_front(&CanvasWidget::renderText, this, renderingOffset), *args);
  }
}

void CanvasWidget::renderImage(Vec2F const& renderingOffset, String const& texName, Vec2F const& position, float scale, Vec4B const& color, bool centered) {
  auto& context = (*this->context());
  auto texSize = Vec2F(context.textureSize(texName));
  Vec2F pos = centered ? (position - scale * texSize / 2.0f) : position;

  RectF screenCoords;
  if (m_ignoreInterfaceScale)
    screenCoords = RectF::withSize(renderingOffset + pos, texSize * scale);
  else
    screenCoords = RectF::withSize(renderingOffset * context.interfaceScale() + pos * context.interfaceScale(), texSize * scale * context.interfaceScale());

  context.drawQuad(texName, screenCoords, color);
}

void CanvasWidget::renderImageRect(Vec2F const& renderingOffset, String const& texName, RectF const& texCoords, RectF const& screenCoords, Vec4B const& color) {
  auto& context = (*this->context());
  if (m_ignoreInterfaceScale)
    context.drawQuad(texName, texCoords, screenCoords.translated(renderingOffset), color);
  else
    context.drawQuad(texName, texCoords, screenCoords.scaled(context.interfaceScale()).translated(renderingOffset * context.interfaceScale()), color);
}

void CanvasWidget::renderDrawable(Vec2F const& renderingOffset, Drawable drawable, Vec2F const& screenPos) {
  auto& context = (*this->context());
  if (m_ignoreInterfaceScale)
    context.drawDrawable(std::move(drawable), renderingOffset + screenPos, 1);
  else {
    drawable.scale(context.interfaceScale());
    context.drawDrawable(std::move(drawable), renderingOffset * context.interfaceScale() + screenPos * context.interfaceScale(), 1);
  }
}

void CanvasWidget::renderTiledImage(Vec2F const& renderingOffset, String const& texName, float textureScale, Vec2D const& offset, RectF const& screenCoords, Vec4B const& color) {
  auto& context = (*this->context());

  Vec2F texSize = Vec2F(context.textureSize(texName));
  Vec2F texScaledSize = texSize * textureScale;
  Vec2I textureCount = Vec2I::ceil(screenCoords.size().piecewiseDivide(texScaledSize)) + Vec2I(2, 2);
  Vec2F screenLowerLeft = screenCoords.min() - Vec2F(pfmod<double>(texScaledSize[0] - offset[0], texScaledSize[0]), pfmod<double>(texScaledSize[1] - offset[1], texScaledSize[1]));

  for (int x = 0; x < textureCount[0]; ++x) {
    for (int y = 0; y < textureCount[1]; ++y) {
      Vec2F screenPos = screenLowerLeft + texScaledSize.piecewiseMultiply({(float)x, (float)y});
      RectF screenRect = RectF::withSize(screenPos, texScaledSize);

      RectF limitedScreenRect;
      limitedScreenRect.setXMin(std::max(screenRect.xMin(), screenCoords.xMin()));
      limitedScreenRect.setYMin(std::max(screenRect.yMin(), screenCoords.yMin()));
      limitedScreenRect.setXMax(std::min(screenRect.xMax(), screenCoords.xMax()));
      limitedScreenRect.setYMax(std::min(screenRect.yMax(), screenCoords.yMax()));

      RectF limitedTexRect = limitedScreenRect.translated(-screenPos).scaled(1 / textureScale);

      if (limitedScreenRect.isEmpty())
        continue;

      if (m_ignoreInterfaceScale)
        context.drawQuad(texName, limitedTexRect, limitedScreenRect.translated(renderingOffset), color);
      else
        context.drawQuad(texName, limitedTexRect, limitedScreenRect.translated(renderingOffset).scaled(context.interfaceScale()), color);
    }
  }
}

void CanvasWidget::renderLine(Vec2F const& renderingOffset, Vec2F const& begin, Vec2F const end, Vec4B const& color, float lineWidth) {
  auto& context = (*this->context());
  if (m_ignoreInterfaceScale)
    context.drawLine(renderingOffset + begin, renderingOffset + end, color, lineWidth);
  else {
    context.drawLine(
      renderingOffset * context.interfaceScale() + begin * context.interfaceScale(),
      renderingOffset * context.interfaceScale() + end * context.interfaceScale(),
      color, lineWidth);
  }
}

void CanvasWidget::renderRect(Vec2F const& renderingOffset, RectF const& coords, Vec4B const& color) {
  auto& context = (*this->context());

  if (m_ignoreInterfaceScale)
    context.drawQuad(coords.translated(renderingOffset), color);
  else
    context.drawQuad(coords.scaled(context.interfaceScale()).translated(renderingOffset * context.interfaceScale()), color);
}

void CanvasWidget::renderPoly(Vec2F const& renderingOffset, PolyF poly, Vec4B const& color, float lineWidth) {
  auto& context = (*this->context());
  poly.translate(renderingOffset);
  if (m_ignoreInterfaceScale)
    context.drawPolyLines(poly, color, lineWidth);
  else
    context.drawInterfacePolyLines(poly, color, lineWidth);
}

void CanvasWidget::renderTriangles(Vec2F const& renderingOffset, List<std::tuple<Vec2F, Vec2F, Vec2F>> const& triangles, Vec4B const& color) {
  auto& context = (*this->context());
  auto translated = triangles.transformed([&renderingOffset](std::tuple<Vec2F, Vec2F, Vec2F> const& poly) {
      return std::tuple<Vec2F, Vec2F, Vec2F>(std::get<0>(poly) + renderingOffset,
        std::get<1>(poly) + renderingOffset,
        std::get<2>(poly) + renderingOffset);
    });
  if (m_ignoreInterfaceScale)
    context.drawTriangles(translated, color);
  else
    context.drawInterfaceTriangles(translated, color);
}

void CanvasWidget::renderText(Vec2F const& renderingOffset, String const& s, TextPositioning const& position, TextStyle const& style) {
  auto& context = (*this->context());
  context.setTextStyle(style, m_ignoreInterfaceScale ? 1 : context.interfaceScale());

  TextPositioning translatedPosition = position;
  translatedPosition.pos += renderingOffset;
  if (m_ignoreInterfaceScale)
    context.renderText(s, translatedPosition);
  else
    context.renderInterfaceText(s, translatedPosition);

  context.clearTextStyle();
}

}
