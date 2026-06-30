#pragma once

#include "StarList.hpp"

namespace Star {

struct PreviewTile;
class PreviewTileTool;

class PreviewTileTool {
public:
  virtual ~PreviewTileTool() = default;
  virtual List<PreviewTile> previewTiles(bool shifting) const = 0;
};

}
