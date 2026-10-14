module;
import std;
import star.list;


struct PreviewTile;
using PreviewTilePtr = std::shared_ptr<PreviewTile>;
using PreviewTileConstPtr = std::shared_ptr<const PreviewTile>;
using PreviewTileWeakPtr = std::weak_ptr<PreviewTile>;
using PreviewTileConstWeakPtr = std::weak_ptr<const PreviewTile>;
using PreviewTileUPtr = std::unique_ptr<PreviewTile>;
using PreviewTileConstUPtr = std::unique_ptr<const PreviewTile>;

class PreviewTileTool;
using PreviewTileToolPtr = std::shared_ptr<PreviewTileTool>;
using PreviewTileToolConstPtr = std::shared_ptr<const PreviewTileTool>;
using PreviewTileToolWeakPtr = std::weak_ptr<PreviewTileTool>;
using PreviewTileToolConstWeakPtr = std::weak_ptr<const PreviewTileTool>;
using PreviewTileToolUPtr = std::unique_ptr<PreviewTileTool>;
using PreviewTileToolConstUPtr = std::unique_ptr<const PreviewTileTool>;

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
