#pragma once

#include "StarConfig.hpp"
#include "StarDrawable.hpp"
#include "StarEntityRenderingTypes.hpp"
#include "StarLightSource.hpp"
#include "StarMixer.hpp"
#include "StarParticle.hpp"

namespace Star {

// Callback interface for entities to produce light sources, particles,
// drawables, and sounds on render.  Everything added is expected to already be
// translated into world space.
class RenderCallback {
public:
  virtual ~RenderCallback();

  virtual void addDrawable(Drawable drawable, EntityRenderLayer renderLayer) = 0;
  virtual void addLightSource(LightSource lightSource) = 0;
  virtual void addParticle(Particle particle) = 0;
  virtual void addAudio(Ptr<AudioInstance> audio) = 0;
  virtual void addTilePreview(PreviewTile preview) = 0;
  virtual void addOverheadBar(OverheadBar bar) = 0;

  // Convenience non-virtuals

  void addDrawables(List<Drawable> drawables, EntityRenderLayer renderLayer, Vec2F translate = Vec2F());
  void addLightSources(List<LightSource> lightSources, Vec2F translate = Vec2F());
  void addParticles(List<Particle> particles, Vec2F translate = Vec2F());
  void addAudios(List<Ptr<AudioInstance>> audios, Vec2F translate = Vec2F());
  void addTilePreviews(List<PreviewTile> previews);
  void addOverheadBars(List<OverheadBar> bars, Vec2F translate = Vec2F());
};

}// namespace Star
