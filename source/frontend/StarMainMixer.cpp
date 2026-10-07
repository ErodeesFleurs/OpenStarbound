#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarPoly.hpp"
#include "StarInterpolation.hpp"
#include "StarImage.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarEither.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarRect.hpp"
#include "StarOrderedMap.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
import star.time;
import star.ttl_cache;
import star.listener;
import star.perlin;
#include "StarRandomPoint.hpp"
#include "StarFont.hpp"
#include "StarStringView.hpp"
import star.text;
#include "StarException.hpp"
#include "StarDataStreamDevices.hpp"
#include "StarOrderedMap.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVariant.hpp"
#include "StarArray.hpp"
#include "StarOrderedSet.hpp"
#include "StarByteArray.hpp"

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
#include "StarNetCompatibility.hpp"
#include "StarConfig.hpp"
#include <atomic>
#include <memory>
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarBTree.hpp"
import star.btree_database;
#include "StarRpcPromise.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarAStar.hpp"
import star.periodic_function;
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"
#include <functional>
#include <queue>


#include "StarLuaRoot.hpp"
import star.mixer;


import star.main_mixer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.root;
import star.configuration;
import star.host_address;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.chat_types;
import star.uuid;
import star.warping;
import star.item_descriptor;
import star.quest_descriptor;
import star.ai_types;
import star.socket;
import star.tcp;
#include "StarP2PNetworkingService.hpp"
import star.worker_pool;
import star.tile_sector_array;
import star.world_layout;
import star.celestial_types;
import star.tile_modification;
import star.interaction_types;
import star.wiring;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.universe_connection;
import star.world_structure;
import star.chat_action;
import star.entity_rendering;
import star.world;
#include "StarLuaComponents.hpp"
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.game_timers;
import star.world_client;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.actor_movement_controller;
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
import star.world_geometry;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.collision_generator;
import star.world_tiles;
import star.drawable;
import star.entity_rendering_types;
import star.sky_types;
import star.celestial_coordinate;
import star.sky_parameters;
import star.sky_render_data;
import star.plant_database;
import star.parallax;
import star.animation;
import star.particle;
import star.weather_types;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.material_render_profile;
import star.renderer;
import star.tile_drawer;
import star.tile_painter;
import star.asset_texture_group;
import star.environment_painter;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable_painter;
import star.world_camera;
import star.world_painter;
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
import star.application_controller;
struct OpusDecoder;
typedef std::unique_ptr<OpusDecoder, void(*)(OpusDecoder*)> OpusDecoderPtr;
struct OpusEncoder;
typedef std::unique_ptr<OpusEncoder, void(*)(OpusEncoder*)> OpusEncoderPtr;
import star.voice;

namespace Star {

MainMixer::MainMixer(unsigned sampleRate, unsigned channels) {
  m_mixer = make_shared<Mixer>(sampleRate, channels);
}

void MainMixer::setUniverseClient(UniverseClientPtr universeClient) {
  m_universeClient = std::move(universeClient);
}

void MainMixer::setWorldPainter(WorldPainterPtr worldPainter) {
  m_worldPainter = std::move(worldPainter);
}

void MainMixer::update(float dt, bool muteSfx, bool muteMusic) {
  auto assets = Root::singleton().assets();

  auto updateGroupVolume = [&](MixerGroup group, bool muted, String const& settingName) {
      if (m_mutedGroups.contains(group) != muted) {
        if (muted) {
          m_mutedGroups.add(group);
          m_mixer->setGroupVolume(group, 0, 1.0f);
        } else {
          m_mutedGroups.remove(group);
          m_mixer->setGroupVolume(group, m_groupVolumes[group], 1.0f);
        }
      } else if (!m_mutedGroups.contains(group)) {
        float volumeSetting = Root::singleton().configuration()->get(settingName).toFloat() / 100.0f;
        volumeSetting = perceptualToAmplitude(volumeSetting);
        if (!m_groupVolumes.contains(group) || volumeSetting != m_groupVolumes[group]) {
          m_mixer->setGroupVolume(group, volumeSetting);
          m_groupVolumes[group] = volumeSetting;
        }
      }
    };

  updateGroupVolume(MixerGroup::Effects, muteSfx, "sfxVol");
  updateGroupVolume(MixerGroup::Music, muteMusic, "musicVol");
  updateGroupVolume(MixerGroup::Cinematic, false, "sfxVol");
  updateGroupVolume(MixerGroup::Instruments, muteSfx, "instrumentVol");

  WorldClientPtr currentWorld;
  if (m_universeClient)
    currentWorld = m_universeClient->worldClient();

  if (currentWorld) {
    for (auto audioInstance : currentWorld->pullPendingAudio())
      m_mixer->play(audioInstance);

    for (auto audioInstance : currentWorld->pullPendingMusic()) {
      audioInstance->setMixerGroup(MixerGroup::Music);
      m_mixer->play(audioInstance);
    }

    if (m_universeClient && m_universeClient->mainPlayer()->underwater()) {
      if (!m_mixer->hasEffect("lowpass"))
        m_mixer->addEffect("lowpass", m_mixer->lowpass(32), 0.50f);
      if (!m_mixer->hasEffect("echo"))
        m_mixer->addEffect("echo", m_mixer->echo(0.2f, 0.6f, 0.4f), 0.50f);
    } else {
      if (m_mixer->hasEffect("lowpass"))
        m_mixer->removeEffect("lowpass", 0.5f);
      if (m_mixer->hasEffect("echo"))
        m_mixer->removeEffect("echo", 0.5f);
    }

    float baseMaxDistance = assets->json("/sfx.config:baseMaxDistance").toFloat();
    Vec2F stereoAdjustmentRange = jsonToVec2F(assets->json("/sfx.config:stereoAdjustmentRange"));
    float attenuationGamma = assets->json("/sfx.config:attenuationGamma").toFloat();
    auto playerPos = m_universeClient->mainPlayer()->position();
    auto cameraPos = m_worldPainter->camera().centerWorldPosition();
    auto worldGeometry = currentWorld->geometry();

    Mixer::PositionalAttenuationFunction attenuationFunction = [&](unsigned channel, Vec2F pos, float rangeMultiplier) {
      Vec2F playerDiff = worldGeometry.diff(pos, playerPos);
      Vec2F cameraDiff = worldGeometry.diff(pos, cameraPos);
      float playerMagSq = playerDiff.magnitudeSquared();
      float cameraMagSq = cameraDiff.magnitudeSquared();

      Vec2F diff;
      float diffMagnitude;
      if (playerMagSq < cameraMagSq) {
        diff = playerDiff;
        diffMagnitude = sqrt(playerMagSq);
      }
      else {
        diff = cameraDiff;
        diffMagnitude = sqrt(cameraMagSq);
      }

      if (diffMagnitude == 0.0f)
        return 0.0f;

      Vec2F diffNorm = diff / diffMagnitude;

      float stereoIncidence = channel == 0 ? -diffNorm[0] : diffNorm[0];

      float maxDistance = baseMaxDistance * rangeMultiplier * lerp((stereoIncidence + 1.0f) / 2.0f, stereoAdjustmentRange[0], stereoAdjustmentRange[1]);

      return pow(clamp(diffMagnitude / maxDistance, 0.0f, 1.0f), 1.0f / attenuationGamma);
    };

    if (Voice* voice = Voice::singletonPtr())
      voice->update(dt, attenuationFunction);

    m_mixer->update(dt, attenuationFunction);

  } else {
    if (m_mixer->hasEffect("lowpass"))
      m_mixer->removeEffect("lowpass", 0);
    if (m_mixer->hasEffect("echo"))
      m_mixer->removeEffect("echo", 0);

    if (Voice* voice = Voice::singletonPtr())
      voice->update(dt);

    m_mixer->update(dt);
  }
}

MixerPtr MainMixer::mixer() const {
  return m_mixer;
}

void MainMixer::setSpeed(float speed) {
  m_mixer->setSpeed(max(speed, 0.0f));
}

void MainMixer::setVolume(float volume, float rampTime) {
  m_mixer->setVolume(volume, rampTime);
}

void MainMixer::read(int16_t* sampleData, size_t frameCount, Mixer::ExtraMixFunction extraMixFunction) {
  m_mixer->read(sampleData, frameCount, extraMixFunction);
}

}
