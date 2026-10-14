module;
#include "StarConfig.hpp"
import star.json;

import star.string;
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
import star.variant;
import star.image;
import star.bi_map;
import star.ref_ptr;
import star.renderer;
import star.maybe;
import star.listener;
import star.asset_texture_group;

namespace Star {

STAR_CLASS(DrawablePainter);

class DrawablePainter {
public:
  DrawablePainter(RendererPtr renderer, AssetTextureGroupPtr textureGroup);

  void drawDrawable(Drawable const& drawable);

  void cleanup(int64_t textureTimeout);

private:
  RendererPtr m_renderer;
  AssetTextureGroupPtr m_textureGroup;
};

}

export module star.drawable_painter;

export namespace Star {
  using ::Star::DrawablePainter;
  using ::Star::DrawablePainterPtr;
  using ::Star::DrawablePainterConstPtr;
  using ::Star::DrawablePainterWeakPtr;
  using ::Star::DrawablePainterConstWeakPtr;
  using ::Star::DrawablePainterUPtr;
  using ::Star::DrawablePainterConstUPtr;
}
