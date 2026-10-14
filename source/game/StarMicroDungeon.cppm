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
using DungeonGeneratorWorldFacadePtr = std::shared_ptr<DungeonGeneratorWorldFacade>;
using DungeonGeneratorWorldFacadeConstPtr = std::shared_ptr<const DungeonGeneratorWorldFacade>;
using DungeonGeneratorWorldFacadeWeakPtr = std::weak_ptr<DungeonGeneratorWorldFacade>;
using DungeonGeneratorWorldFacadeConstWeakPtr = std::weak_ptr<const DungeonGeneratorWorldFacade>;
using DungeonGeneratorWorldFacadeUPtr = std::unique_ptr<DungeonGeneratorWorldFacade>;
using DungeonGeneratorWorldFacadeConstUPtr = std::unique_ptr<const DungeonGeneratorWorldFacade>;
}

export module star.micro_dungeon;

export namespace Star {

class MicroDungeonFactory;
using MicroDungeonFactoryPtr = std::shared_ptr<MicroDungeonFactory>;
using MicroDungeonFactoryConstPtr = std::shared_ptr<const MicroDungeonFactory>;
using MicroDungeonFactoryWeakPtr = std::weak_ptr<MicroDungeonFactory>;
using MicroDungeonFactoryConstWeakPtr = std::weak_ptr<const MicroDungeonFactory>;
using MicroDungeonFactoryUPtr = std::unique_ptr<MicroDungeonFactory>;
using MicroDungeonFactoryConstUPtr = std::unique_ptr<const MicroDungeonFactory>;

class MicroDungeonFactory {
public:
  MicroDungeonFactory();

  Maybe<std::pair<List<RectI>, Set<Vec2I>>> generate(RectI const& bounds,
      String const& dungeonName,
      Vec2I const& position,
      std::uint64_t seed,
      float threatLevel,
      DungeonGeneratorWorldFacadePtr facade,
      bool forcePlacement = false);

private:
  List<int> m_placementshifts;
  bool m_generating;
};

}
