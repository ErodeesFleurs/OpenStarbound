module;

import star.rect;
import std;
import star.list;
import star.maybe;
import star.string;
import star.set;

import star.vector;
namespace Star {
class DungeonGeneratorWorldFacade;
}

export module star.micro_dungeon;

export namespace Star {

class MicroDungeonFactory {
public:
  MicroDungeonFactory();

  Maybe<std::pair<List<RectI>, Set<Vec2I>>> generate(RectI const& bounds,
      String const& dungeonName,
      Vec2I const& position,
      std::uint64_t seed,
      float threatLevel,
      std::shared_ptr<DungeonGeneratorWorldFacade> facade,
      bool forcePlacement = false);

private:
  List<int> m_placementshifts;
  bool m_generating;
};

}
