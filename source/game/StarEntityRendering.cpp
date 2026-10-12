#include "StarJson.hpp"
import star.audio;
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
import star.vector;
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
import star.color;
#include "StarString.hpp"
#include "StarDataStream.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
import star.game_types;
#include "StarOrderedMap.hpp"
#include "StarIODevice.hpp"
import star.ref_ptr;
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;

import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

namespace Star {

RenderCallback::~RenderCallback() {}

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

void RenderCallback::addAudios(List<AudioInstancePtr> audios, Vec2F translate) {
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
