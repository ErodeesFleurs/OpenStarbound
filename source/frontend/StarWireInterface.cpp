#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarNetElementSystem.hpp"
#include "StarInterpolation.hpp"
#include "StarImage.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarVersion.hpp"
#include "StarEither.hpp"
#include "StarWeightedPool.hpp"
#include "StarRect.hpp"
#include "StarTtlCache.hpp"
#include "StarPerlin.hpp"
#include "StarRandomPoint.hpp"
#include "StarOrderedMap.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarArray.hpp"
#include "StarNetCompatibility.hpp"
#include "StarSectorArray2D.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarOrderedSet.hpp"
#include "StarNetElementFloatFields.hpp"
#include <functional>
#include "StarAStar.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"
#include "StarSpline.hpp"
#include "StarConfig.hpp"


#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
#include "StarLuaRoot.hpp"
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.pane;
import star.world_geometry;
import star.wiring;


import star.wire_interface;
import star.widget_parsing;
import star.gui_reader;
import star.worker_pool;
import star.tile_sector_array;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.tile_modification;
import star.warping;
import star.inspectable_entity;
import star.plant;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.system_world;
import star.damage_manager;
import star.net_packets;
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
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.wire_entity;
import star.liquid_types;
import star.collision_generator;
import star.world_tiles;
import star.entity_rendering_types;
import star.sky_types;
import star.sky_parameters;
import star.sky_render_data;
import star.plant_database;
import star.parallax;
import star.animation;
import star.particle;
import star.weather_types;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.material_render_profile;
import star.tile_drawer;
import star.tile_painter;
import star.environment_painter;
import star.world_camera;
import star.world_painter;
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
import star.ai_types;
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
import star.item;
import star.non_rotated_drawables_item;
import star.tool_user_item;
import star.beam_item;
import star.status_effect_item;
import star.fireable_item;
import star.swingable_item;
import star.durability_item;
import star.pointable_item;
STAR_STRUCT(PreviewTile);
STAR_CLASS(PreviewTileTool);
import star.preview_tile_tool;
import star.tools;

namespace Star {

WirePane::WirePane(WorldClientPtr worldClient, PlayerPtr player, WorldPainterPtr worldPainter) {
  m_worldClient = worldClient;
  m_player = player;
  m_worldPainter = worldPainter;

  m_connecting = false;

  auto assets = Root::singleton().assets();
  GuiReader reader;
  reader.construct(assets->json("/interface/wires/wires.config:gui"), this);

  m_nodeSize = Vec2F(1.8f, 1.8f);

  JsonObject config = assets->json("/player.config:wireConfig").toObject();
  m_minBeamWidth = config.get("minWireWidth").toFloat();
  m_maxBeamWidth = config.get("maxWireWidth").toFloat();
  m_beamWidthDev = config.value("wireWidthDev", (m_maxBeamWidth - m_minBeamWidth) / 3).toFloat();
  m_minBeamTrans = config.get("minWireTrans").toFloat();
  m_maxBeamTrans = config.get("maxWireTrans").toFloat();
  m_beamTransDev = config.value("wireTransDev", (m_maxBeamTrans - m_minBeamTrans) / 3).toFloat();
  m_innerBrightnessScale = config.get("innerBrightnessScale").toFloat();
  m_firstStripeThickness = config.get("firstStripeThickness").toFloat();
  m_secondStripeThickness = config.get("secondStripeThickness").toFloat();

  setTitle({}, "", "Wire you looking at me like that?");
  disableScissoring();
  markAsContainer();
}

void WirePane::reset() {
  m_connecting = false;
}

void WirePane::update(float) {
  if (!active())
    return;
  if (!m_worldClient->inWorld()) {
    dismiss();
    return;
  }

  if (m_connecting) {
    for (auto entity : m_worldClient->atTile<WireEntity>(m_sourceConnector.entityLocation)) {
      if (m_sourceConnector.nodeIndex < entity->nodeCount(m_sourceDirection))
        return;
    }

    // stop pending connection if node has been removed
    m_connecting = false;
  }
}

void WirePane::renderWire(Vec2F from, Vec2F to, Color baseColor) {
  if (m_worldClient->isTileProtected(Vec2I::floor(from)) || m_worldClient->isTileProtected(Vec2I::floor(to)))
    return;

  from = m_worldPainter->camera().worldToScreen(from);
  to = m_worldPainter->camera().worldToScreen(to);

  auto rangeRand = [&](float dev, float min, float max) {
    return clamp<float>(Random::nrandf(dev, max), min, max);
  };

  float lineThickness = m_worldPainter->camera().pixelRatio() * rangeRand(m_beamWidthDev, m_minBeamWidth, m_maxBeamWidth);
  float beamTransparency = rangeRand(m_beamTransDev, m_minBeamTrans, m_maxBeamTrans);
  baseColor.setAlphaF(baseColor.alphaF() * beamTransparency);
  Color innerStripe = baseColor;
  innerStripe.setValue(1 - (1 - innerStripe.value()) / m_innerBrightnessScale);
  innerStripe.setSaturation(innerStripe.saturation() / m_innerBrightnessScale);
  Color firstStripe = innerStripe;
  innerStripe.setValue(1 - (1 - innerStripe.value()) / m_innerBrightnessScale);
  innerStripe.setSaturation(innerStripe.saturation() / m_innerBrightnessScale);
  Color secondStripe = innerStripe;

  context()->drawLine(from, to, baseColor.toRgba(), lineThickness);
  context()->drawLine(from, to, firstStripe.toRgba(), lineThickness * m_firstStripeThickness);
  context()->drawLine(from, to, secondStripe.toRgba(), lineThickness * m_secondStripeThickness);
}

void WirePane::renderImpl() {
  if (!m_worldClient->inWorld())
    return;

  auto region = RectF(m_worldClient->clientWindow());

  auto const& camera = m_worldPainter->camera();
  auto badWire = Color::rgbf(0.6f + (float)sin(Time::monotonicTime() * Constants::pi * 2.0) * 0.4f, 0.0f, 0.0f);
  auto white = Color::White.toRgba();
  float phase = 0.5f + 0.5f * std::sin((double)Time::monotonicMilliseconds() / 100.0);
  auto drawLineColor = Color::Red.mix(Color::White, phase);

  for (auto entity : m_worldClient->query<WireEntity>(region)) {
    for (size_t i = 0; i < entity->nodeCount(WireDirection::Input); ++i) {
      Vec2I position = entity->tilePosition() + entity->nodePosition({WireDirection::Input, i});
      if (!m_worldClient->isTileProtected(position)) {
        auto icon = entity->nodeIcon({WireDirection::Input, i});
        context()->drawQuad(icon,
            camera.worldToScreen(centerOfTile(position) - ((Vec2F(context()->textureSize(icon)) / TilePixels) / 2.0f)),
            camera.pixelRatio(), white);
      }
    }

    for (size_t i = 0; i < entity->nodeCount(WireDirection::Output); ++i) {
      Vec2I position = entity->tilePosition() + entity->nodePosition({WireDirection::Output, i});
      if (!m_worldClient->isTileProtected(position)) {
        auto icon = entity->nodeIcon({WireDirection::Output, i});
        context()->drawQuad(icon,
            camera.worldToScreen(centerOfTile(position) - ((Vec2F(context()->textureSize(icon)) / TilePixels) / 2.0f)),
            camera.pixelRatio(), white);
      }
    }
  }

  HashSet<pair<WireConnection, WireConnection>> visitedConnections;
  for (auto entity : m_worldClient->query<WireEntity>(region)) {
    for (size_t i = 0; i < entity->nodeCount(WireDirection::Input); ++i) {
      Vec2I tilePosition = entity->tilePosition();
      Vec2I inPosition = tilePosition + entity->nodePosition({WireDirection::Input, i});

      for (auto const& connection : entity->connectionsForNode({WireDirection::Input, i})) {
        visitedConnections.add({{tilePosition, i}, connection});

        auto wire = entity->nodeColor({WireDirection::Input, i}).mix(Color::Black, 0.8f);
        Vec2I outPosition = connection.entityLocation;
        if (auto sourceEntity = m_worldClient->atTile<WireEntity>(connection.entityLocation).get(0)) {
          if (connection.nodeIndex < sourceEntity->nodeCount(WireDirection::Output)) {
            outPosition += sourceEntity->nodePosition({WireDirection::Output, connection.nodeIndex});
            wire = sourceEntity->nodeColor({WireDirection::Output, connection.nodeIndex});
            if (!sourceEntity->nodeState(WireNode{WireDirection::Output, connection.nodeIndex}))
              wire = wire.mix(Color::Black, 0.8f);
          } else {
            wire = badWire;
          }
        }

        renderWire(centerOfTile(inPosition), centerOfTile(outPosition), wire);
      }
    }

    for (size_t i = 0; i < entity->nodeCount(WireDirection::Output); ++i) {
      Vec2I tilePosition = entity->tilePosition();
      Vec2I outPosition = tilePosition + entity->nodePosition({WireDirection::Output, i});

      auto wire = entity->nodeColor({WireDirection::Output, i});
      if (!entity->nodeState({WireDirection::Output, i}))
        wire = wire.mix(Color::Black, 0.8f);

      for (auto const& connection : entity->connectionsForNode({WireDirection::Output, i})) {
        if (visitedConnections.add({connection, {tilePosition, i}}))
          continue;

        Vec2I inPosition = connection.entityLocation;
        if (auto sourceEntity = m_worldClient->atTile<WireEntity>(connection.entityLocation).get(0)) {
          if (connection.nodeIndex < sourceEntity->nodeCount(WireDirection::Input))
            inPosition += sourceEntity->nodePosition({WireDirection::Input, connection.nodeIndex});
          else
            wire = badWire;
        }

        renderWire(centerOfTile(outPosition), centerOfTile(inPosition), wire);
      }
    }
  }

  if (m_connecting) {
    Vec2F aimPos = m_worldPainter->camera().screenToWorld(Vec2F(m_mousePos) * m_context->interfaceScale());
    Vec2I sourcePosition = m_sourceConnector.entityLocation;
    if (auto sourceEntity = m_worldClient->atTile<WireEntity>(m_sourceConnector.entityLocation).get(0)) {
      if (m_sourceDirection == WireDirection::Input){
        sourcePosition += sourceEntity->nodePosition({WireDirection::Input, m_sourceConnector.nodeIndex});
        drawLineColor = sourceEntity->nodeColor({WireDirection::Input, m_sourceConnector.nodeIndex}).mix(Color::White, phase);
      }else{
        sourcePosition += sourceEntity->nodePosition({WireDirection::Output, m_sourceConnector.nodeIndex});
        drawLineColor = sourceEntity->nodeColor({WireDirection::Output, m_sourceConnector.nodeIndex}).mix(Color::White, phase);
      }
    }
    renderWire(centerOfTile(sourcePosition), aimPos, drawLineColor);
  }
}

bool WirePane::sendEvent(InputEvent const& event) {
  if (event.is<MouseMoveEvent>())
    m_mousePos = *context()->mousePosition(event);

  if (event.is<MouseButtonDownEvent>())
    m_mousePos = *context()->mousePosition(event);

  return false;
}

WireConnector::SwingResult WirePane::swing(WorldGeometry const& geometry, Vec2F pos, FireMode mode) {
  pos = geometry.xwrap(pos);

  if (m_worldClient->isTileProtected((Vec2I)pos)) {
    m_connecting = false;
    return Protected;
  }

  RectF bounds = {pos - Vec2F(16, 16), pos + Vec2F(16, 16)};

  if (mode == FireMode::Primary) {
    Maybe<WireConnection> matchNode;
    WireDirection matchDirection = WireDirection::Output;
    float bestDist = 10000;
    for (auto entity : m_worldClient->query<WireEntity>(bounds)) {
      for (size_t i = 0; i < entity->nodeCount(WireDirection::Input); ++i) {
        RectF inbounds = RectF::withSize(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Input, i})) - (m_nodeSize / 2.0f), m_nodeSize);
        if (geometry.rectContains(inbounds, pos)) {
          if (!matchNode) {
            matchNode = WireConnection{entity->tilePosition(), i};
            matchDirection = WireDirection::Input;
            bestDist = geometry.diff(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Input, i})), pos).magnitudeSquared();
          } else {
            float thisDist = geometry.diff(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Input, i})), pos).magnitudeSquared();
            if (thisDist < bestDist) {
              matchNode = WireConnection{entity->tilePosition(), i};
              matchDirection = WireDirection::Input;
              bestDist = thisDist;
            }
          }
        }
      }

      for (size_t i = 0; i < entity->nodeCount(WireDirection::Output); ++i) {
        RectF outbounds = RectF::withSize(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Output, i})) - (m_nodeSize / 2.0f), m_nodeSize);
        if (geometry.rectContains(outbounds, pos)) {
          if (!matchNode) {
            matchNode = WireConnection{entity->tilePosition(), i};
            matchDirection = WireDirection::Output;
            bestDist = geometry.diff(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Output, i})), pos).magnitudeSquared();
          } else {
            float thisDist = geometry.diff(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Output, i})), pos).magnitudeSquared();
            if (thisDist < bestDist) {
              matchNode = WireConnection{entity->tilePosition(), i};
              matchDirection = WireDirection::Output;
              bestDist = thisDist;
            }
          }
        }
      }
    }

    if (matchNode) {
      if (m_connecting) {
        if (m_sourceDirection == matchDirection) {
          return Mismatch;
        } else if (m_sourceConnector.entityLocation == matchNode->entityLocation) {
          return Mismatch;
        } else {
          if (matchDirection == WireDirection::Output)
            m_worldClient->connectWire(*matchNode, m_sourceConnector);
          else
            m_worldClient->connectWire(m_sourceConnector, *matchNode);
        }
      } else {
        m_connecting = true;
        m_sourceDirection = matchDirection;
        m_sourceConnector = *matchNode;
      }
      return Connect;
    }

  } else {
    m_connecting = false;

    Maybe<WireNode> matchNode;
    Maybe<Vec2I> matchPosition;
    float bestDist = 10000;
    for (auto entity : m_worldClient->query<WireEntity>(bounds)) {
      for (size_t i = 0; i < entity->nodeCount(WireDirection::Input); ++i) {
        RectF inbounds = RectF::withSize(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Input, i})) - (m_nodeSize / 2.0f), m_nodeSize);
        if (geometry.rectContains(inbounds, pos) && entity->connectionsForNode({WireDirection::Input, i}).size() > 0) {
          if (!matchNode) {
            matchPosition = entity->tilePosition();
            matchNode = WireNode{WireDirection::Input, i};
            bestDist = geometry.diff(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Input, i})), pos).magnitudeSquared();
          } else {
            float thisDist = geometry.diff(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Input, i})), pos).magnitudeSquared();
            if (thisDist < bestDist) {
              matchPosition = entity->tilePosition();
              matchNode = WireNode{WireDirection::Input, i};
              bestDist = thisDist;
            }
          }
        }
      }

      for (size_t i = 0; i < entity->nodeCount(WireDirection::Output); ++i) {
        RectF outbounds = RectF::withSize(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Output, i})) - (m_nodeSize / 2.0f), m_nodeSize);
        if (geometry.rectContains(outbounds, pos) && entity->connectionsForNode({WireDirection::Output, i}).size() > 0) {
          if (!matchNode) {
            matchPosition = entity->tilePosition();
            matchNode = WireNode{WireDirection::Output, i};
            bestDist = geometry.diff(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Output, i})), pos).magnitudeSquared();
          } else {
            float thisDist = geometry.diff(centerOfTile(entity->tilePosition() + entity->nodePosition({WireDirection::Output, i})), pos).magnitudeSquared();
            if (thisDist < bestDist) {
              matchPosition = entity->tilePosition();
              matchNode = WireNode{WireDirection::Output, i};
              bestDist = thisDist;
            }
          }
        }
      }
    }

    if (matchNode) {
      m_worldClient->disconnectAllWires(*matchPosition, *matchNode);
      return Connect;
    }
  }
  return Nothing;
}

bool WirePane::connecting() {
  return m_connecting;
}

}
