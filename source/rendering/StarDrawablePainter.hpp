#pragma once

#include "StarAssetTextureGroup.hpp"
#include "StarDrawable.hpp"
#include "StarRenderer.hpp"

import std;

namespace Star {

class DrawablePainter {
public:
  DrawablePainter(Ptr<Renderer> renderer, Ptr<AssetTextureGroup> textureGroup);

  void drawDrawable(Drawable const& drawable);

  void cleanup(std::int64_t textureTimeout);

private:
  Ptr<Renderer> m_renderer;
  Ptr<AssetTextureGroup> m_textureGroup;
};

}// namespace Star
