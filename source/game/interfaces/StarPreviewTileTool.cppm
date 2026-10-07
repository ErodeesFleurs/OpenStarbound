module;
#include "StarList.hpp"


STAR_STRUCT(PreviewTile);

STAR_CLASS(PreviewTileTool);

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
export using ::PreviewTilePtr;
export using ::PreviewTileConstPtr;
export using ::PreviewTileWeakPtr;
export using ::PreviewTileConstWeakPtr;
export using ::PreviewTileUPtr;
export using ::PreviewTileConstUPtr;
export using ::PreviewTileTool;
export using ::PreviewTileToolPtr;
export using ::PreviewTileToolConstPtr;
export using ::PreviewTileToolWeakPtr;
export using ::PreviewTileToolConstWeakPtr;
export using ::PreviewTileToolUPtr;
export using ::PreviewTileToolConstUPtr;
