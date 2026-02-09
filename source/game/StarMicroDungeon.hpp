#pragma once

#include "StarConfig.hpp"
#include "StarRect.hpp"
#include "StarString.hpp"

import std;

namespace Star {

class DungeonGeneratorWorldFacade;

class MicroDungeonFactory {
public:
  MicroDungeonFactory();

  auto generate(RectI const& bounds,
                String const& dungeonName,
                Vec2I const& position,
                std::uint64_t seed,
                float threatLevel,
                Ptr<DungeonGeneratorWorldFacade> facade,
                bool forcePlacement = false) -> std::optional<std::pair<List<RectI>, Set<Vec2I>>>;

private:
  List<int> m_placementshifts;
  bool m_generating;
};

}// namespace Star
