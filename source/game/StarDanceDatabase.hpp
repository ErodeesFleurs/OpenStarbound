#pragma once

#include "StarConfig.hpp"
#include "StarString.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

struct DanceStep {
  std::optional<String> bodyFrame;
  std::optional<String> frontArmFrame;
  std::optional<String> backArmFrame;
  Vec2F headOffset;
  Vec2F frontArmOffset;
  Vec2F backArmOffset;
  float frontArmRotation;
  float backArmRotation;
};

struct Dance {
  String name;
  List<String> states;
  float cycle;
  bool cyclic;
  float duration;
  List<DanceStep> steps;
};

class DanceDatabase {
public:
  DanceDatabase();

  [[nodiscard]] auto getDance(String const& name) const -> Ptr<Dance>;

private:
  static auto readDance(String const& path) -> Ptr<Dance>;

  StringMap<Ptr<Dance>> m_dances;
};

}// namespace Star
