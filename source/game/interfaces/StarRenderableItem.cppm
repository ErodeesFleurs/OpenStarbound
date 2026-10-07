module;
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarGameTypes.hpp"
#include "StarDirectives.hpp"

import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;

namespace Star {

  STAR_CLASS(RenderableItem);

  class RenderableItem {
  public:
    virtual ~RenderableItem() {}

    virtual void render(RenderCallback* renderCallback, EntityRenderLayer renderLayer) = 0;
  };

}

export module star.renderable_item;

export namespace Star {
  using ::Star::RenderableItem;
  using ::Star::RenderableItemPtr;
  using ::Star::RenderableItemConstPtr;
  using ::Star::RenderableItemWeakPtr;
  using ::Star::RenderableItemConstWeakPtr;
  using ::Star::RenderableItemUPtr;
  using ::Star::RenderableItemConstUPtr;
}
