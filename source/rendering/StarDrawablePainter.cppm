module;

#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
import star.drawable;
#include "StarRenderer.hpp"
#include "StarMaybe.hpp"
#include "StarString.hpp"
#include "StarBiMap.hpp"
#include "StarListener.hpp"
#include "StarRenderer.hpp"
#include "StarAssetPath.hpp"
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
