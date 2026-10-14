module;
#include "StarConfig.hpp"
import star.json;

import star.string;
import star.variant;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;



import star.drawable;
import star.animation;

import star.vector;
namespace Star {

class InterfaceCursor {
public:
  InterfaceCursor();

  // Sets the cursor to the default defined in interface.config
  void resetCursor();

  // Sets the cursor config to the given config IF the config is different than
  // the current one.  Expects a full asset path to the cursor config.
  void setCursor(String const& configFile);

  Drawable drawable() const;
  Vec2I size() const;
  Vec2I offset() const;
  float scale(float interfaceScale = 0) const;

  void update(float dt);

private:
  String m_configFile;
  Vec2I m_offset;
  Vec2I m_size;
  unsigned int m_scale;
  MVariant<String, Animation> m_drawable;
};

}

export module star.interface_cursor;

export namespace Star {
  using ::Star::InterfaceCursor;
}
