#include "StarInterfaceCursor.hpp"
#include "StarAlgorithm.hpp"
#include "StarJsonExtra.hpp"
#include "StarAssets.hpp"

namespace Star {

InterfaceCursor::InterfaceCursor(InterfaceCursorServices services)
  : m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "InterfaceCursor", "assets")),
    m_imageMetadata(requireServiceValueAs<StarException>(std::move(services.imageMetadata), "InterfaceCursor", "image metadata")) {
  resetCursor();
}

void InterfaceCursor::resetCursor() {
  setCursor(m_assets->json("/interface.config:defaultCursor").toString());
}

void InterfaceCursor::setCursor(String const& configFile) {
  if (m_configFile == configFile)
    return;

  m_configFile = configFile;

  auto config = m_assets->json(m_configFile);

  m_offset = jsonToVec2I(config.get("offset"));
  if (config.contains("image")) {
    m_drawable = config.getString("image");
    m_size = Vec2I{m_imageMetadata->imageSize(config.getString("image"))};
  } else {
    m_drawable = Animation(config.get("animation"), "/interface", m_assets, m_imageMetadata);
    m_size = Vec2I(m_drawable.get<Animation>().drawable(1.0f).boundBox(false, m_imageMetadata).size());
  }

  m_scale = config.getUInt("scale", 0);
}

Drawable InterfaceCursor::drawable() const {
  if (m_drawable.is<String>())
    return Drawable::makeImage(m_drawable.get<String>(), 1.0f, false, {}, Color::White, m_imageMetadata);
  else
    return m_drawable.get<Animation>().drawable(1.0f);
}

Vec2I InterfaceCursor::size() const {
  return m_size;
}

Vec2I InterfaceCursor::offset() const {
  return m_offset;
}

float InterfaceCursor::scale(float interfaceScale) const {
  return m_scale ? m_scale : interfaceScale;
}

void InterfaceCursor::update(float dt) {
  if (m_drawable.is<Animation>()) {
    m_drawable.get<Animation>().update(dt);
  }
}

}
