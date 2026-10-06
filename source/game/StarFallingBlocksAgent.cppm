module;

#include "StarVector.hpp"
#include "StarSet.hpp"
#include "StarMap.hpp"
#include "StarRandom.hpp"
#include "StarGameTypes.hpp"
#include "StarWorldTiles.hpp"

namespace Star {

STAR_CLASS(MaterialDatabase);
STAR_CLASS(FallingBlocksFacade);
STAR_CLASS(FallingBlocksAgent);

enum class FallingBlockType {
  Immovable,
  Falling,
  Cascading,
  Open
};

class FallingBlocksFacade {
public:
  virtual ~FallingBlocksFacade() = default;

  virtual FallingBlockType blockType(Vec2I const& pos) = 0;
  virtual void moveBlock(Vec2I const& from, Vec2I const& to) = 0;
};

class FallingBlocksAgent {
public:
  FallingBlocksAgent(FallingBlocksFacadePtr worldFacade);

  void update();

  void visitLocation(Vec2I const& location);
  void visitRegion(RectI const& region);

private:
  FallingBlocksFacadePtr m_facade;
  float m_immediateUpwardPropagateProbability;
  HashSet<Vec2I> m_pending;
  RandomSource m_random;
};

}

export module star.falling_blocks_agent;

export namespace Star {
  using ::Star::FallingBlockType;
  using ::Star::MaterialDatabase;
  using ::Star::MaterialDatabasePtr;
  using ::Star::MaterialDatabaseConstPtr;
  using ::Star::MaterialDatabaseWeakPtr;
  using ::Star::MaterialDatabaseConstWeakPtr;
  using ::Star::MaterialDatabaseUPtr;
  using ::Star::MaterialDatabaseConstUPtr;
  using ::Star::FallingBlocksFacade;
  using ::Star::FallingBlocksFacadePtr;
  using ::Star::FallingBlocksFacadeConstPtr;
  using ::Star::FallingBlocksFacadeWeakPtr;
  using ::Star::FallingBlocksFacadeConstWeakPtr;
  using ::Star::FallingBlocksFacadeUPtr;
  using ::Star::FallingBlocksFacadeConstUPtr;
  using ::Star::FallingBlocksAgent;
  using ::Star::FallingBlocksAgentPtr;
  using ::Star::FallingBlocksAgentConstPtr;
  using ::Star::FallingBlocksAgentWeakPtr;
  using ::Star::FallingBlocksAgentConstWeakPtr;
  using ::Star::FallingBlocksAgentUPtr;
  using ::Star::FallingBlocksAgentConstUPtr;
}
