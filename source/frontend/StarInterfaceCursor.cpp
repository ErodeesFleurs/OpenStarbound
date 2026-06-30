#include "StarInterfaceCursor.hpp"
#include "StarJsonExtra.hpp"
#include "StarException.hpp"
#include "StarAssets.hpp"

namespace Star {

InterfaceCursor::InterfaceCursor(InterfaceCursorServices services)
  : m_assets(std::move(services.assets)),
    m_imageMetadata(std::move(services.imageMetadata)) {
  if (!m_assets)
    throw StarException("InterfaceCursor requires assets service");
  if (!m_imageMetadata)
    throw StarException("InterfaceCursor requires image metadata service");

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
    m_drawable = Animation(config.get("animation"), "/interface");
    m_size = Vec2I(m_drawable.get<Animation>().drawable(1.0f).boundBox(false).size());
  }

  m_scale = config.getUInt("scale", 0);
}

Drawable InterfaceCursor::drawable() const {
  if (m_drawable.is<String>())
    return Drawable::makeImage(m_drawable.get<String>(), 1.0f, false, {});
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
