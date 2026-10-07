module;

#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
import star.mixer;
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
import star.drawable;
#include "StarGameTypes.hpp"
import star.entity_rendering_types;
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
import star.animation;
import star.particle;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
import star.drawable;
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"

import star.light_source;

namespace Star {

STAR_CLASS(RenderCallback);

// Callback interface for entities to produce light sources, particles,
// drawables, and sounds on render.  Everything added is expected to already be
// translated into world space.
class RenderCallback {
public:
  virtual ~RenderCallback();

  virtual void addDrawable(Drawable drawable, EntityRenderLayer renderLayer) = 0;
  virtual void addLightSource(LightSource lightSource) = 0;
  virtual void addParticle(Particle particle) = 0;
  virtual void addAudio(AudioInstancePtr audio) = 0;
  virtual void addTilePreview(PreviewTile preview) = 0;
  virtual void addOverheadBar(OverheadBar bar) = 0;

  // Convenience non-virtuals

  void addDrawables(List<Drawable> drawables, EntityRenderLayer renderLayer, Vec2F translate = Vec2F());
  void addLightSources(List<LightSource> lightSources, Vec2F translate = Vec2F());
  void addParticles(List<Particle> particles, Vec2F translate = Vec2F());
  void addAudios(List<AudioInstancePtr> audios, Vec2F translate = Vec2F());
  void addTilePreviews(List<PreviewTile> previews);
  void addOverheadBars(List<OverheadBar> bars, Vec2F translate = Vec2F());
};

}

export module star.entity_rendering;

export namespace Star {
  using ::Star::RenderCallback;
  using ::Star::RenderCallbackPtr;
  using ::Star::RenderCallbackConstPtr;
  using ::Star::RenderCallbackWeakPtr;
  using ::Star::RenderCallbackConstWeakPtr;
  using ::Star::RenderCallbackUPtr;
  using ::Star::RenderCallbackConstUPtr;
}
