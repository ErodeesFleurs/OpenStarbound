#include "StarDanceDatabase.hpp"

#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

DanceDatabase::DanceDatabase() {
  auto assets = Root::singleton().assets();
  auto& files = assets->scanExtension("dance");
  for (auto& file : files) {
    try {
      Ptr<Dance> dance = readDance(file);
      m_dances[dance->name] = dance;
    } catch (std::exception const& e) {
      Logger::error("Error loading dance file {}: {}", file, outputException(e, true));
    }
  }
}

auto DanceDatabase::getDance(String const& name) const -> Ptr<Dance> {
  if (auto dance = m_dances.ptr(name))
    return *dance;
  else {
    Logger::error("Invalid dance '{}', using default", name);
    return m_dances.get("assetmissing");
  }
}

auto DanceDatabase::readDance(String const& path) -> Ptr<Dance> {
  auto assets = Root::singleton().assets();
  Json config = assets->json(path);

  String name = config.getString("name");
  List<String> states = config.getArray("states").transformed([](Json const& state) -> String { return state.toString(); });
  float cycle = config.getFloat("cycle");
  bool cyclic = config.getBool("cyclic");
  float duration = config.getFloat("duration");
  List<DanceStep> steps = config.getArray("steps").transformed([](Json const& step) -> DanceStep {
    if (step.isType(Json::Type::Object)) {
      std::optional<String> bodyFrame = step.optString("bodyFrame");
      std::optional<String> frontArmFrame = step.optString("frontArmFrame");
      std::optional<String> backArmFrame = step.optString("backArmFrame");
      Vec2F headOffset = step.opt("headOffset").transform(jsonToVec2F).value_or(Vec2F());
      Vec2F frontArmOffset = step.opt("frontArmOffset").transform(jsonToVec2F).value_or(Vec2F());
      Vec2F backArmOffset = step.opt("backArmOffset").transform(jsonToVec2F).value_or(Vec2F());
      float frontArmRotation = step.optFloat("frontArmRotation").value_or(0.0f);
      float backArmRotation = step.optFloat("frontArmRotation").value_or(0.0f);
      return DanceStep{.bodyFrame = bodyFrame,
                       .frontArmFrame = frontArmFrame,
                       .backArmFrame = backArmFrame,
                       .headOffset = headOffset,
                       .frontArmOffset = frontArmOffset,
                       .backArmOffset = backArmOffset,
                       .frontArmRotation = frontArmRotation,
                       .backArmRotation = backArmRotation};
    } else {
      std::optional<String> bodyFrame = step.get(0).optString();
      std::optional<String> frontArmFrame = step.get(1).optString();
      std::optional<String> backArmFrame = step.get(2).optString();
      Vec2F headOffset = step.get(3).opt().transform(jsonToVec2F).value_or(Vec2F());
      Vec2F frontArmOffset = step.get(4).opt().transform(jsonToVec2F).value_or(Vec2F());
      Vec2F backArmOffset = step.get(5).opt().transform(jsonToVec2F).value_or(Vec2F());
      return DanceStep{.bodyFrame = bodyFrame, .frontArmFrame = frontArmFrame, .backArmFrame = backArmFrame, .headOffset = headOffset, .frontArmOffset = frontArmOffset, .backArmOffset = backArmOffset, .frontArmRotation = 0.0f, .backArmRotation = 0.0f};
    }
  });

  return std::make_shared<Dance>(Dance{.name = name, .states = states, .cycle = cycle, .cyclic = cyclic, .duration = duration, .steps = steps});
}

}// namespace Star
