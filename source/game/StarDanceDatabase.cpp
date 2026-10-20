import star.json;
import star.json_extra;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.string;
import star.format;
import star.version;

import star.asset_source;
import star.assets;

import star.dance_database;

import star.vector;
namespace Star {

DanceDatabase::DanceDatabase(std::shared_ptr<Assets const> assets, Logger& logger) : m_logger(logger) {
  auto& files = assets->scanExtension("dance");
  for (auto& file : files) {
    try {
      std::shared_ptr<Dance> dance = readDance(*assets, file);
      m_dances[dance->name] = dance;
    } catch (std::exception const&) {
      ExceptionDiagnostic const e(std::current_exception());
      m_logger.error("Error loading dance file {}: {}", file, outputException(e, true));
    }
  }
}

std::shared_ptr<Dance> DanceDatabase::getDance(String const& name) const {
  if (auto dance = m_dances.ptr(name))
    return *dance;
  else {
    m_logger.error("Invalid dance '{}', using default", name);
    return m_dances.get("assetmissing");
  }
}

std::shared_ptr<Dance> DanceDatabase::readDance(Assets const& assets, String const& path) {
  Json config = assets.json(path);

  String name = config.getString("name");
  List<String> states = config.getArray("states").transformed([](Json const& state) { return state.toString(); });
  float cycle = config.getFloat("cycle");
  bool cyclic = config.getBool("cyclic");
  float duration = config.getFloat("duration");
  List<DanceStep> steps = config.getArray("steps").transformed([](Json const& step) {
    if (step.isType(Json::Type::Object)) {
      std::optional<String> bodyFrame = step.optString("bodyFrame");
      std::optional<String> frontArmFrame = step.optString("frontArmFrame");
      std::optional<String> backArmFrame = step.optString("backArmFrame");
      Vec2F headOffset = step.opt("headOffset").transform(jsonToVec2F).value_or(Vec2F());
      Vec2F frontArmOffset = step.opt("frontArmOffset").transform(jsonToVec2F).value_or(Vec2F());
      Vec2F backArmOffset = step.opt("backArmOffset").transform(jsonToVec2F).value_or(Vec2F());
      float frontArmRotation = step.optFloat("frontArmRotation").value_or(0.0f);
      float backArmRotation = step.optFloat("frontArmRotation").value_or(0.0f);
      return DanceStep{bodyFrame,
          frontArmFrame,
          backArmFrame,
          headOffset,
          frontArmOffset,
          backArmOffset,
          frontArmRotation,
          backArmRotation};
    } else {
      std::optional<String> bodyFrame = step.get(0).optString();
      std::optional<String> frontArmFrame = step.get(1).optString();
      std::optional<String> backArmFrame = step.get(2).optString();
      Vec2F headOffset = step.get(3).opt().transform(jsonToVec2F).value_or(Vec2F());
      Vec2F frontArmOffset = step.get(4).opt().transform(jsonToVec2F).value_or(Vec2F());
      Vec2F backArmOffset = step.get(5).opt().transform(jsonToVec2F).value_or(Vec2F());
      return DanceStep{bodyFrame, frontArmFrame, backArmFrame, headOffset, frontArmOffset, backArmOffset, 0.0f, 0.0f};
    }
  });

  return std::make_shared<Dance>(Dance{name, states, cycle, cyclic, duration, steps});
}

}
