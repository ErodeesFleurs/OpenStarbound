#pragma once

#include "StarConfig.hpp"
#include "StarRandom.hpp"
#include "StarRect.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"

namespace Star {

enum class FallingBlockType {
  Immovable,
  Falling,
  Cascading,
  Open
};

class FallingBlocksFacade {
public:
  virtual ~FallingBlocksFacade() = default;

  virtual auto blockType(Vec2I const& pos) -> FallingBlockType = 0;
  virtual void moveBlock(Vec2I const& from, Vec2I const& to) = 0;
};

class FallingBlocksAgent {
public:
  FallingBlocksAgent(Ptr<FallingBlocksFacade> worldFacade);

  void update();

  void visitLocation(Vec2I const& location);
  void visitRegion(RectI const& region);

private:
  Ptr<FallingBlocksFacade> m_facade;
  float m_immediateUpwardPropagateProbability;
  HashSet<Vec2I> m_pending;
  RandomSource m_random;
};

}// namespace Star
