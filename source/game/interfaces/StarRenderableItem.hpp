#pragma once

#include "StarEntityRendering.hpp"

namespace Star {

class RenderableItem {
public:
  virtual ~RenderableItem() = default;

  virtual void render(RenderCallback* renderCallback, EntityRenderLayer renderLayer) = 0;
};

}// namespace Star
