#include "StarImageWidget.hpp"

namespace Star {

ImageWidget::ImageWidget(GuiContext& context, String const& image) : Widget(context) {
  setImage(image);
}

void ImageWidget::renderImpl() {
  auto screenPos = screenPosition();
  for (auto const& drawable : m_drawables)
    context().drawDrawable(drawable, Vec2F(screenPos) * context().interfaceScale() + Vec2F(m_offset), context().interfaceScale(), Vec4B::filled(255));
}

bool ImageWidget::interactive() const {
  return false;
}

void ImageWidget::setImage(String const& image) {
  if (image.empty())
    setDrawables({});
  else
    setDrawables({Drawable::makeImage(image, 1.0f, false, Vec2F(), context().imageMetadata())});
}

void ImageWidget::setScale(float scale) {
  m_scale = scale;
  transformDrawables();
}

void ImageWidget::setRotation(float rotation) {
  m_rotation = rotation;
  transformDrawables();
}

String ImageWidget::image() const {
  if (m_drawables.empty())
    return "";
  else
    return AssetPath::join(m_drawables[0].imagePart().image);
}

void ImageWidget::setDrawables(List<Drawable> drawables) {
  m_baseDrawables = std::move(drawables);

  transformDrawables();
}

Vec2I ImageWidget::offset() {
  return m_offset;
}

void ImageWidget::setOffset(Vec2I const& offset) {
  m_offset = offset;
}

bool ImageWidget::centered() {
  return m_centered;
}

void ImageWidget::setCentered(bool centered) {
  m_centered = centered;
  transformDrawables();
}

bool ImageWidget::trim() {
  return m_trim;
}

void ImageWidget::setTrim(bool trim) {
  m_trim = trim;
  transformDrawables();
}

void ImageWidget::setMaxSize(Vec2I const& size) {
  m_maxSize = size;
  transformDrawables();
}

void ImageWidget::setMinSize(Vec2I const& size) {
  m_minSize = size;
  transformDrawables();
}

RectI ImageWidget::screenBoundRect() const {
  RectI base = RectI::withSize(screenPosition(), size());
  if (m_centered) {
    base.translate(-Vec2I::floor(size() / 2));
  }

  return base;
}

void ImageWidget::transformDrawables() {
  m_drawables.clear();
  for (auto drawable : m_baseDrawables)
    m_drawables.append(Drawable(drawable));

  if (m_rotation != 0)
    Drawable::rotateAll(m_drawables, m_rotation);

  // When 'centered' is true, the drawables provided are pre-centered
  // around 0,0. Tooltips use this, as well as quest dialog portraits.
  auto imageMetadata = context().imageMetadata();
  if (m_centered) {
    auto boundBox = Drawable::boundBoxAll(m_drawables, m_trim, imageMetadata);
    Drawable::translateAll(m_drawables, -boundBox.center());
  }

  auto boundBox = Drawable::boundBoxAll(m_drawables, m_trim, imageMetadata);
  auto size = boundBox.size().piecewiseMax({0, 0});
  if (size[0] && size[1]) {
    if ((size[0] > m_maxSize[0]) || (size[1] > m_maxSize[1])) {
      m_scale = std::min(m_maxSize[0] / size[0], m_maxSize[1] / size[1]);
    } else if ((size[0] < m_minSize[0]) || (size[1] < m_minSize[1])) {
      m_scale = std::min(m_minSize[0] / size[0], m_minSize[1] / size[1]);
    }
  }

  Drawable::scaleAll(m_drawables, m_scale);

  setSize(Vec2I((size * m_scale).ceil()));
}

}
