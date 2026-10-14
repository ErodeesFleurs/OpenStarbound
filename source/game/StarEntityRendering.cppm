module;
#include "StarConfig.hpp"
import star.json;

import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.vector;
import star.maybe;
import star.bi_map;
import star.mixer;
import star.color;
import star.string;
import star.data_stream;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.drawable;
import star.game_types;
import star.entity_rendering_types;
import star.animation;
import star.particle;
import star.drawable;
import star.game_types;

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
