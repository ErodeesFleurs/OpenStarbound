module;

import star.rect;
import std;
import star.list;
import star.string;
import star.set;

import star.vector;
namespace Star {
class DungeonGeneratorWorldFacade;
class Root;
}

export module star.micro_dungeon;

export namespace Star {

class MicroDungeonFactory {
  Root& m_root;
public:
  MicroDungeonFactory(Root& root);

  std::optional<std::pair<List<RectI>, Set<Vec2I>>> generate(RectI const& bounds,
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
