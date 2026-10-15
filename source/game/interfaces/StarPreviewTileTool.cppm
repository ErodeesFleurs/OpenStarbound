module;
import std;
import star.list;


struct PreviewTile;

import star.entity_rendering_types;

namespace Star {

class PreviewTileTool {
public:
  virtual ~PreviewTileTool() {}
  virtual List<PreviewTile> previewTiles(bool shifting) const = 0;
};

}

export module star.preview_tile_tool;

export namespace Star {
  using ::Star::PreviewTileTool;
}

export using ::PreviewTile;
