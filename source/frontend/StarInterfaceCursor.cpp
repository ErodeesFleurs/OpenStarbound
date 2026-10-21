import std;
import star.json;
import star.json_extra;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.string;
import star.color;
import star.ordered_map;
import star.bi_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
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

import star.gui_context;
import star.vector;
namespace Star {

InterfaceCursor::InterfaceCursor(GuiContext& context) : m_guiContext(&context) {
  resetCursor();
}

void InterfaceCursor::resetCursor() {
  auto& root = m_guiContext->root();
  auto assets = root.assets();
  setCursor(assets->json("/interface.config:defaultCursor").toString());
}

void InterfaceCursor::setCursor(String const& configFile) {
  if (m_configFile == configFile)
    return;

  m_configFile = configFile;

  auto& root = m_guiContext->root();
  auto assets = root.assets();
  auto imageMetadata = root.imageMetadataDatabase();

  auto config = assets->json(m_configFile);

  m_offset = jsonToVec2I(config.get("offset"));
  if (config.contains("image")) {
    m_drawable = config.getString("image");
    m_size = Vec2I{imageMetadata->imageSize(config.getString("image"))};
  } else {
    m_drawable = Animation(root, config.get("animation"), "/interface");
    m_size = Vec2I(std::get<Animation>(m_drawable).drawable(1.0f).boundBox(*imageMetadata, false).size());
  }

  m_scale = config.getUInt("scale", 0);
}

Drawable InterfaceCursor::drawable() const {
  if (std::holds_alternative<String>(m_drawable))
    return Drawable::makeImage(*m_guiContext->root().imageMetadataDatabase(), std::get<String>(m_drawable), 1.0f, false, {});
  else
    return std::get<Animation>(m_drawable).drawable(1.0f);
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
  if (std::holds_alternative<Animation>(m_drawable)) {
    std::get<Animation>(m_drawable).update(dt);
  }
}

}
