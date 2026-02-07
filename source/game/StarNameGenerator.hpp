#pragma once

#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarRandom.hpp"

import std;

namespace Star {

using NameGeneratorException = ExceptionDerived<"NameGeneratorException">;

struct MarkovSource {
  size_t prefixSize;
  size_t endSize;
  StringList starts;
  StringMap<StringList> chains;
  StringSet ends;
};

class PatternedNameGenerator {
public:
  PatternedNameGenerator();

  [[nodiscard]] auto generateName(String const& rulesAsset) const -> String;
  [[nodiscard]] auto generateName(String const& rulesAsset, std::uint64_t seed) const -> String;
  auto generateName(String const& rulesAsset, RandomSource& random) const -> String;

private:
  auto processRule(JsonArray const& rule, RandomSource& random) const -> String;

  [[nodiscard]] auto isProfane(String const& name) const -> bool;

  auto makeMarkovSource(size_t prefixSize, size_t endSize, StringList sourceNames) -> MarkovSource;

  StringMap<MarkovSource> m_markovSources;
  StringSet m_profanityFilter;
};

}// namespace Star
