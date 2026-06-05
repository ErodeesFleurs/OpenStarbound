#pragma once

#include "StarRect.hpp"
#include "StarString.hpp"

namespace Star {

class DungeonGeneratorWorldFacade;
using DungeonGeneratorWorldFacadePtr = SharedPtr<DungeonGeneratorWorldFacade>;
class MicroDungeonFactory;
using MicroDungeonFactoryPtr = SharedPtr<MicroDungeonFactory>;

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
