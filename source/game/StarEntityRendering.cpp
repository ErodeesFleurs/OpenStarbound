import star.json;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.vector;
import star.bi_map;
import star.color;
import star.string;
import star.data_stream;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.game_types;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.version;

import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;

namespace Star {

class TileDrawer;

RenderCallback::~RenderCallback() {}

TileDrawer* RenderCallback::tileDrawer() const {
  return nullptr;
}

void RenderCallback::addDrawables(List<Drawable> drawables, EntityRenderLayer renderLayer, Vec2F translate) {
  for (auto& drawable : drawables) {
    drawable.translate(translate);
    addDrawable(std::move(drawable), renderLayer);
  }
}

void RenderCallback::addLightSources(List<LightSource> lightSources, Vec2F translate) {
  for (auto& lightSource : lightSources) {
    lightSource.translate(translate);
    addLightSource(std::move(lightSource));
  }
}

void RenderCallback::addParticles(List<Particle> particles, Vec2F translate) {
  for (auto& particle : particles) {
    particle.translate(translate);
    addParticle(std::move(particle));
  }
}

void RenderCallback::addAudios(List<std::shared_ptr<AudioInstance>> audios, Vec2F translate) {
  for (auto& audio : audios) {
    audio->translate(translate);
    addAudio(std::move(audio));
  }
}

void RenderCallback::addTilePreviews(List<PreviewTile> previews) {
  for (auto& preview : previews)
    addTilePreview(std::move(preview));
}

void RenderCallback::addOverheadBars(List<OverheadBar> bars, Vec2F translate) {
  for (auto& bar : bars) {
    bar.entityPosition += translate;
    addOverheadBar(std::move(bar));
  }
}

}
