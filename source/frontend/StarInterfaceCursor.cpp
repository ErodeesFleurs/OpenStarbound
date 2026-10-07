#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;

import star.drawable;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;


import star.animation;


import star.interface_cursor;


import star.image_metadata_database;

namespace Star {

InterfaceCursor::InterfaceCursor() {
  resetCursor();
}

void InterfaceCursor::resetCursor() {
  auto& root = Root::singleton();
  auto assets = root.assets();
  setCursor(assets->json("/interface.config:defaultCursor").toString());
}

void InterfaceCursor::setCursor(String const& configFile) {
  if (m_configFile == configFile)
    return;

  m_configFile = configFile;

  auto& root = Root::singleton();
  auto assets = root.assets();
  auto imageMetadata = root.imageMetadataDatabase();

  auto config = assets->json(m_configFile);

  m_offset = jsonToVec2I(config.get("offset"));
  if (config.contains("image")) {
    m_drawable = config.getString("image");
    m_size = Vec2I{imageMetadata->imageSize(config.getString("image"))};
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
