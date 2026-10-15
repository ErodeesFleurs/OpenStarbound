module;
import std;
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

class DrawablePainter {
public:
  DrawablePainter(std::shared_ptr<Renderer> renderer, std::shared_ptr<AssetTextureGroup> textureGroup);

  void drawDrawable(Drawable const& drawable);

  void cleanup(std::int64_t textureTimeout);

private:
  std::shared_ptr<Renderer> m_renderer;
  std::shared_ptr<AssetTextureGroup> m_textureGroup;
};

}

export module star.drawable_painter;

export namespace Star {
  using ::Star::DrawablePainter;
}
