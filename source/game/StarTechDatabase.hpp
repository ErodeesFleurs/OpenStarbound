#pragma once

#include "StarBiMap.hpp"
#include "StarException.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

using TechDatabaseException = ExceptionDerived<"TechDatabaseException">;

enum class TechType {
  Head,
  Body,
  Legs
};
extern EnumMap<TechType> const TechTypeNames;

struct TechConfig {
  String name;
  String path;
  Json parameters;

  TechType type;

  StringList scripts;
  std::optional<String> animationConfig;

  String description;
  String shortDescription;
  Rarity rarity;
  String icon;
};

class TechDatabase {
public:
  TechDatabase();

  [[nodiscard]] auto contains(String const& techName) const -> bool;
  [[nodiscard]] auto tech(String const& techName) const -> TechConfig;

private:
  [[nodiscard]] auto parseTech(Json const& config, String const& path) const -> TechConfig;

  StringMap<TechConfig> m_tech;
};

}// namespace Star
