module;

#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
import star.drawable;
#include "StarVariant.hpp"
#include "StarImage.hpp"
#include "StarBiMap.hpp"
#include "StarRefPtr.hpp"
import star.renderer;
#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarBiMap.hpp"
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
