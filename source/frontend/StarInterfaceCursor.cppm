module;

#include "StarJson.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;



import star.drawable;
import star.animation;

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
