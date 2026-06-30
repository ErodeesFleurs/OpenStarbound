#pragma once

#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"
#include "StarAssets.hpp"

namespace Star {

struct TechDatabaseExceptionTag { static constexpr char const* typeName = "TechDatabaseException"; };
using TechDatabaseException = TypedException<StarException, TechDatabaseExceptionTag>;

class TechDatabase;
using TechDatabasePtr = SharedPtr<TechDatabase>;
using TechDatabaseConstPtr = SharedPtr<TechDatabase const>;

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
  Maybe<String> animationConfig;

  String description;
  String shortDescription;
  Rarity rarity;
  String icon;
};

class TechDatabase {
public:
  TechDatabase(AssetsConstPtr assets);

  [[nodiscard]] bool contains(String const& techName) const;
  [[nodiscard]] TechConfig tech(String const& techName) const;

private:
  [[nodiscard]] TechConfig parseTech(Json const& config, String const& path) const;

  StringMap<TechConfig> m_tech;
};

}
