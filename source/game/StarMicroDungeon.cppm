module;

import star.rect;
#include "StarConfig.hpp"
import star.list;
import star.maybe;
import star.string;
import star.set;

import star.vector;
namespace Star {
STAR_CLASS(DungeonGeneratorWorldFacade);
}

export module star.micro_dungeon;

export namespace Star {

STAR_CLASS(MicroDungeonFactory);

class MicroDungeonFactory {
public:
  MicroDungeonFactory();

  Maybe<pair<List<RectI>, Set<Vec2I>>> generate(RectI const& bounds,
      String const& dungeonName,
      Vec2I const& position,
      uint64_t seed,
      float threatLevel,
      DungeonGeneratorWorldFacadePtr facade,
      bool forcePlacement = false);

private:
  List<int> m_placementshifts;
  bool m_generating;
};

}
