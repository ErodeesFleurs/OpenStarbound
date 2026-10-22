module;
import std;
import star.json;
import star.format;

import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.vector;
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

class TileDrawer;

// Callback interface for entities to produce light sources, particles,
// drawables, and sounds on render.  Everything added is expected to already be
// translated into world space.
class RenderCallback {
public:
  virtual ~RenderCallback();

  virtual TileDrawer* tileDrawer() const;

  [[nodiscard]] virtual std::expected<void, StarException> addDrawable(Drawable drawable, EntityRenderLayer renderLayer) = 0;
  [[nodiscard]] virtual std::expected<void, StarException> addLightSource(LightSource lightSource) = 0;
  [[nodiscard]] virtual std::expected<void, StarException> addParticle(Particle particle) = 0;
  [[nodiscard]] virtual std::expected<void, StarException> addAudio(std::shared_ptr<AudioInstance> audio) = 0;
  [[nodiscard]] virtual std::expected<void, StarException> addTilePreview(PreviewTile preview) = 0;
  [[nodiscard]] virtual std::expected<void, StarException> addOverheadBar(OverheadBar bar) = 0;

  // Convenience non-virtuals

  [[nodiscard]] std::expected<void, StarException> addDrawables(List<Drawable> drawables, EntityRenderLayer renderLayer, Vec2F translate = Vec2F());
  [[nodiscard]] std::expected<void, StarException> addLightSources(List<LightSource> lightSources, Vec2F translate = Vec2F());
  [[nodiscard]] std::expected<void, StarException> addParticles(List<Particle> particles, Vec2F translate = Vec2F());
  [[nodiscard]] std::expected<void, StarException> addAudios(List<std::shared_ptr<AudioInstance>> audios, Vec2F translate = Vec2F());
  [[nodiscard]] std::expected<void, StarException> addTilePreviews(List<PreviewTile> previews);
  [[nodiscard]] std::expected<void, StarException> addOverheadBars(List<OverheadBar> bars, Vec2F translate = Vec2F());
};

}

export module star.entity_rendering;

export namespace Star {
  using ::Star::RenderCallback;
}
