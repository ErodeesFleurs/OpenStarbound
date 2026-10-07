#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarSet.hpp"
#include "StarMap.hpp"
#include "StarRandom.hpp"
#include "StarGameTypes.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include <functional>
#include "StarGameTypes.hpp"
#include "StarMathCommon.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarThread.hpp"
#include "StarInterpolation.hpp"
import star.perlin;
#include "StarMaybe.hpp"
import star.weighted_pool;
#include "StarColor.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarVariant.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarOrderedMap.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;

import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.falling_blocks_agent;

namespace Star {

FallingBlocksAgent::FallingBlocksAgent(FallingBlocksFacadePtr worldFacade)
  : m_facade(std::move(worldFacade)) {
  m_immediateUpwardPropagateProbability = Root::singleton().assets()->json("/worldserver.config:fallingBlocksImmediateUpwardPropogateProbability").toFloat();
}

void FallingBlocksAgent::update() {
  HashSet<Vec2I> processing = take(m_pending);

  while (!processing.empty()) {
    List<Vec2I> positions;
    for (auto const& pos : take(processing))
      positions.append(pos);

    m_random.shuffle(positions);

    positions.sort([](auto const& a, auto const& b) {
        return a[1] < b[1];
      });

    for (auto const& pos : positions) {
      Vec2I belowPos = pos + Vec2I(0, -1);
      Vec2I belowLeftPos = pos + Vec2I(-1, -1);
      Vec2I belowRightPos = pos + Vec2I(1, -1);

      FallingBlockType thisBlock = m_facade->blockType(pos);
      FallingBlockType belowBlock = m_facade->blockType(belowPos);

      Maybe<Vec2I> moveTo;

      if (thisBlock == FallingBlockType::Falling) {
        if (belowBlock == FallingBlockType::Open)
          moveTo = belowPos;
      } else if (thisBlock == FallingBlockType::Cascading) {
        if (belowBlock == FallingBlockType::Open) {
          moveTo = belowPos;
        } else {
          FallingBlockType belowLeftBlock = m_facade->blockType(belowLeftPos);
          FallingBlockType belowRightBlock = m_facade->blockType(belowRightPos);

          if (belowLeftBlock == FallingBlockType::Open && belowRightBlock == FallingBlockType::Open)
            moveTo = m_random.randb() ? belowLeftPos : belowRightPos;
          else if (belowLeftBlock == FallingBlockType::Open)
            moveTo = belowLeftPos;
          else if (belowRightBlock == FallingBlockType::Open)
            moveTo = belowRightPos;
        }
      }

      if (moveTo) {
        m_facade->moveBlock(pos, *moveTo);
        if (m_random.randf() < m_immediateUpwardPropagateProbability) {
          processing.add(pos + Vec2I(0, 1));
          processing.add(pos + Vec2I(-1, 1));
          processing.add(pos + Vec2I(1, 1));
        }

        visitLocation(pos);
        visitLocation(*moveTo);
      }
    }
  }
}

void FallingBlocksAgent::visitLocation(Vec2I const& location) {
  visitRegion(RectI::withSize(location, Vec2I(1, 1)));
}

void FallingBlocksAgent::visitRegion(RectI const& region) {
  for (int x = region.xMin() - 1; x <= region.xMax(); ++x) {
    for (int y = region.yMin(); y <= region.yMax(); ++y) 
      m_pending.add({x, y});
  }
}

}
