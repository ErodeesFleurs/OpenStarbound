#include "StarCanvasWidget.hpp"

namespace Star {

CanvasWidget::CanvasWidget() {
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
  m_renderOps.append(make_tuple(std::move(texName), position, scale, color, false));
}

void CanvasWidget::drawImageCentered(String texName, Vec2F const& position, float scale, Vec4B const& color) {
  m_renderOps.append(make_tuple(std::move(texName), position, scale, color, true));
}

void CanvasWidget::drawImageRect(String texName, RectF const& texCoords, RectF const& screenCoords, Vec4B const& color) {
  m_renderOps.append(make_tuple(std::move(texName), texCoords, screenCoords, color));
}

void CanvasWidget::drawDrawable(Drawable drawable, Vec2F const& screenPos) {
  m_renderOps.append(make_tuple(std::move(drawable), screenPos));
}

void CanvasWidget::drawDrawables(List<Drawable> const& drawables, Vec2F const& screenPos) {
  for (auto& drawable : drawables)
    drawDrawable(std::move(drawable), screenPos);
}

void CanvasWidget::drawTiledImage(String texName, float textureScale, Vec2D const& offset, RectF const& screenCoords, Vec4B const& color) {
  m_renderOps.append(make_tuple(std::move(texName), textureScale, offset, screenCoords, color));
}

void CanvasWidget::drawLine(Vec2F const& begin, Vec2F const end, Vec4B const& color, float lineWidth) {
  m_renderOps.append(make_tuple(begin, end, color, lineWidth));
}

void CanvasWidget::drawRect(RectF const& coords, Vec4B const& color) {
  m_renderOps.append(make_tuple(coords, color));
}

void CanvasWidget::drawPoly(PolyF const& poly, Vec4B const& color, float lineWidth) {
  m_renderOps.append(make_tuple(poly, color, lineWidth));
}

void CanvasWidget::drawTriangles(List<tuple<Vec2F, Vec2F, Vec2F>> const& triangles, Vec4B const& color) {
  m_renderOps.append(make_tuple(triangles, color));
}

void CanvasWidget::drawText(String s, TextPositioning position, unsigned fontSize, Vec4B const& color, FontMode mode, float lineSpacing, String font, String processingDirectives) {
  TextStyle style;
  style.fontSize = fontSize;
  style.color = color;
  style.shadow = fontModeToColor(mode).toRgba();
  style.lineSpacing = lineSpacing;
  style.font = font;
  style.directives = processingDirectives;
  m_renderOps.append(make_tuple(std::move(s), std::move(position), std::move(style)));
}

void CanvasWidget::drawText(String s, TextPositioning position, TextStyle style) {
  m_renderOps.append(make_tuple(std::move(s), std::move(position), std::move(style)));
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

  auto* context = this->context();
  auto interfaceScale = m_ignoreInterfaceScale ? 1 : context->interfaceScale();
  if (auto mouseButtonDown = event.ptr<MouseButtonDownEvent>()) {
    if (inMember(*context->mousePosition(event, interfaceScale)) && m_captureMouse) {
      m_clickEvents.append({*context->mousePosition(event, interfaceScale) - screenPosition(), mouseButtonDown->mouseButton, true});
      m_clickEvents.limitSizeBack(MaximumEventBuffer);
      return true;
    }
  } else if (auto mouseButtonUp = event.ptr<MouseButtonUpEvent>()) {
    if (m_captureMouse) {
      m_clickEvents.append({*context->mousePosition(event, interfaceScale) - screenPosition(), mouseButtonUp->mouseButton, false});
      m_clickEvents.limitSizeBack(MaximumEventBuffer);
      return true;
    }
  } else if (event.is<MouseMoveEvent>()) {
    m_mousePosition = *context->mousePosition(event, interfaceScale) - screenPosition();
    return false;
  } else if (auto keyDown = event.ptr<KeyDownEvent>()) {
    if (m_captureKeyboard) {
      m_keyEvents.append({keyDown->key, true});
      return true;
    }
  } else if (auto keyUp = event.ptr<KeyUpEvent>()) {
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
    if (auto imageArgs = op.ptr<ImageOp>())
      tupleUnpackFunction([this, renderingOffset](auto&&... args) { return renderImage(renderingOffset, std::forward<decltype(args)>(args)...); }, *imageArgs);
    else if (auto imageRectArgs = op.ptr<ImageRectOp>())
      tupleUnpackFunction([this, renderingOffset](auto&&... args) { return renderImageRect(renderingOffset, std::forward<decltype(args)>(args)...); }, *imageRectArgs);
    else if (auto drawableArgs = op.ptr<DrawableOp>())
      tupleUnpackFunction([this, renderingOffset](auto&&... args) { return renderDrawable(renderingOffset, std::forward<decltype(args)>(args)...); }, *drawableArgs);
    else if (auto tiledImageArgs = op.ptr<TiledImageOp>())
      tupleUnpackFunction([this, renderingOffset](auto&&... args) { return renderTiledImage(renderingOffset, std::forward<decltype(args)>(args)...); }, *tiledImageArgs);
    else if (auto lineArgs = op.ptr<LineOp>())
      tupleUnpackFunction([this, renderingOffset](auto&&... args) { return renderLine(renderingOffset, std::forward<decltype(args)>(args)...); }, *lineArgs);
    else if (auto rectArgs = op.ptr<RectOp>())
      tupleUnpackFunction([this, renderingOffset](auto&&... args) { return renderRect(renderingOffset, std::forward<decltype(args)>(args)...); }, *rectArgs);
    else if (auto polyArgs = op.ptr<PolyOp>())
      tupleUnpackFunction([this, renderingOffset](auto&&... args) { return renderPoly(renderingOffset, std::forward<decltype(args)>(args)...); }, *polyArgs);
    else if (auto trianglesArgs = op.ptr<TrianglesOp>())
      tupleUnpackFunction([this, renderingOffset](auto&&... args) { return renderTriangles(renderingOffset, std::forward<decltype(args)>(args)...); }, *trianglesArgs);
    else if (auto textArgs = op.ptr<TextOp>())
      tupleUnpackFunction([this, renderingOffset](auto&&... args) { return renderText(renderingOffset, std::forward<decltype(args)>(args)...); }, *textArgs);
  }
}

void CanvasWidget::renderImage(Vec2F const& renderingOffset, String const& texName, Vec2F const& position, float scale, Vec4B const& color, bool centered) {
  auto* context = this->context();
  auto texSize = Vec2F(context->textureSize(texName));
  Vec2F pos = centered ? (position - scale * texSize / 2.0f) : position;

  RectF screenCoords;
  if (m_ignoreInterfaceScale)
    screenCoords = RectF::withSize(renderingOffset + pos, texSize * scale);
  else
    screenCoords = RectF::withSize(renderingOffset * context->interfaceScale() + pos * context->interfaceScale(), texSize * scale * context->interfaceScale());

  context->drawQuad(texName, screenCoords, color);
}

void CanvasWidget::renderImageRect(Vec2F const& renderingOffset, String const& texName, RectF const& texCoords, RectF const& screenCoords, Vec4B const& color) {
  auto* context = this->context();
  if (m_ignoreInterfaceScale)
    context->drawQuad(texName, texCoords, screenCoords.translated(renderingOffset), color);
  else
    context->drawQuad(texName, texCoords, screenCoords.scaled(context->interfaceScale()).translated(renderingOffset * context->interfaceScale()), color);
}

void CanvasWidget::renderDrawable(Vec2F const& renderingOffset, Drawable drawable, Vec2F const& screenPos) {
  auto* context = this->context();
  if (m_ignoreInterfaceScale)
    context->drawDrawable(std::move(drawable), renderingOffset + screenPos, 1);
  else {
    drawable.scale(context->interfaceScale());
    context->drawDrawable(std::move(drawable), renderingOffset * context->interfaceScale() + screenPos * context->interfaceScale(), 1);
  }
}

void CanvasWidget::renderTiledImage(Vec2F const& renderingOffset, String const& texName, float textureScale, Vec2D const& offset, RectF const& screenCoords, Vec4B const& color) {
  auto* context = this->context();

  Vec2F texSize = Vec2F(context->textureSize(texName));
  Vec2F texScaledSize = texSize * textureScale;
  Vec2I textureCount = Vec2I::ceil(screenCoords.size().piecewiseDivide(texScaledSize)) + Vec2I(2, 2);
  Vec2F screenLowerLeft = screenCoords.min() - Vec2F(pfmod<double>(texScaledSize[0] - offset[0], texScaledSize[0]), pfmod<double>(texScaledSize[1] - offset[1], texScaledSize[1]));

  for (int x = 0; x < textureCount[0]; ++x) {
    for (int y = 0; y < textureCount[1]; ++y) {
      Vec2F screenPos = screenLowerLeft + texScaledSize.piecewiseMultiply({static_cast<float>(x), static_cast<float>(y)});
      RectF screenRect = RectF::withSize(screenPos, texScaledSize);

      RectF limitedScreenRect;
      limitedScreenRect.setXMin(max(screenRect.xMin(), screenCoords.xMin()));
      limitedScreenRect.setYMin(max(screenRect.yMin(), screenCoords.yMin()));
      limitedScreenRect.setXMax(min(screenRect.xMax(), screenCoords.xMax()));
      limitedScreenRect.setYMax(min(screenRect.yMax(), screenCoords.yMax()));

      RectF limitedTexRect = limitedScreenRect.translated(-screenPos).scaled(1 / textureScale);

      if (limitedScreenRect.isEmpty())
        continue;

      if (m_ignoreInterfaceScale)
        context->drawQuad(texName, limitedTexRect, limitedScreenRect.translated(renderingOffset), color);
      else
        context->drawQuad(texName, limitedTexRect, limitedScreenRect.translated(renderingOffset).scaled(context->interfaceScale()), color);
    }
  }
}

void CanvasWidget::renderLine(Vec2F const& renderingOffset, Vec2F const& begin, Vec2F const end, Vec4B const& color, float lineWidth) {
  auto* context = this->context();
  if (m_ignoreInterfaceScale)
    context->drawLine(renderingOffset + begin, renderingOffset + end, color, lineWidth);
  else {
    context->drawLine(
      renderingOffset * context->interfaceScale() + begin * context->interfaceScale(),
      renderingOffset * context->interfaceScale() + end * context->interfaceScale(),
      color, lineWidth);
  }
}

void CanvasWidget::renderRect(Vec2F const& renderingOffset, RectF const& coords, Vec4B const& color) {
  auto* context = this->context();

  if (m_ignoreInterfaceScale)
    context->drawQuad(coords.translated(renderingOffset), color);
  else
    context->drawQuad(coords.scaled(context->interfaceScale()).translated(renderingOffset * context->interfaceScale()), color);
}

void CanvasWidget::renderPoly(Vec2F const& renderingOffset, PolyF poly, Vec4B const& color, float lineWidth) {
  auto* context = this->context();
  poly.translate(renderingOffset);
  if (m_ignoreInterfaceScale)
    context->drawPolyLines(poly, color, lineWidth);
  else
    context->drawInterfacePolyLines(poly, color, lineWidth);
}

void CanvasWidget::renderTriangles(Vec2F const& renderingOffset, List<tuple<Vec2F, Vec2F, Vec2F>> const& triangles, Vec4B const& color) {
  auto* context = this->context();
  auto translated = triangles.transformed([&renderingOffset](tuple<Vec2F, Vec2F, Vec2F> const& poly) {
      return tuple<Vec2F, Vec2F, Vec2F>(get<0>(poly) + renderingOffset,
        get<1>(poly) + renderingOffset,
        get<2>(poly) + renderingOffset);
    });
  if (m_ignoreInterfaceScale)
    context->drawTriangles(translated, color);
  else
    context->drawInterfaceTriangles(translated, color);
}

void CanvasWidget::renderText(Vec2F const& renderingOffset, String const& s, TextPositioning const& position, TextStyle const& style) {
  auto* context = this->context();
  context->setTextStyle(style, m_ignoreInterfaceScale ? 1 : context->interfaceScale());

  TextPositioning translatedPosition = position;
  translatedPosition.pos += renderingOffset;
  if (m_ignoreInterfaceScale)
    context->renderText(s, translatedPosition);
  else
    context->renderInterfaceText(s, translatedPosition);

  context->clearTextStyle();
}

}
