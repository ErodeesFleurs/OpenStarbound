#pragma once

#include "StarList.hpp"

struct PreviewTile;

namespace Star {

class PreviewTileTool {
public:
  virtual ~PreviewTileTool() = default;
  [[nodiscard]] virtual auto previewTiles(bool shifting) const -> List<PreviewTile> = 0;
};

}// namespace Star
