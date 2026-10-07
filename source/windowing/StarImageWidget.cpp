#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVersion.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"

#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
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
import star.image_widget;

namespace Star {

ImageWidget::ImageWidget(String const& image) {
  m_centered = false;
  m_trim = false;
  m_scale = 1;
  m_rotation = 0;
  m_maxSize = Vec2I{4096, 4096};
  m_minSize = Vec2I{0, 0};
  m_offset = Vec2I{0, 0};
  setImage(image);
}

void ImageWidget::renderImpl() {
  auto screenPos = screenPosition();
  for (auto const& drawable : m_drawables)
    context()->drawDrawable(drawable, Vec2F(screenPos) * context()->interfaceScale() + Vec2F(m_offset), context()->interfaceScale(), Vec4B::filled(255));
}

bool ImageWidget::interactive() const {
  return false;
}

void ImageWidget::setImage(String const& image) {
  if (image.empty())
    setDrawables({});
  else
    setDrawables({Drawable::makeImage(image, 1.0f, false, Vec2F())});
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
  if (m_centered) {
    auto boundBox = Drawable::boundBoxAll(m_drawables, m_trim);
    Drawable::translateAll(m_drawables, -boundBox.center());
  }

  auto boundBox = Drawable::boundBoxAll(m_drawables, m_trim);
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
