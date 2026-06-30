#pragma once

#include "StarIAssets.hpp"
#include "StarJson.hpp"
#include "StarAnimation.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {

struct InterfaceCursorServices {
  IAssetsConstPtr assets;
  ImageMetadataDatabaseConstPtr imageMetadata;
};

class InterfaceCursor {
public:
  InterfaceCursor(InterfaceCursorServices services = {});

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
  IAssetsConstPtr m_assets;
  ImageMetadataDatabaseConstPtr m_imageMetadata;
};

}
