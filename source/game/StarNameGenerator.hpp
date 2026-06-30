#pragma once

#include "StarAssets.hpp"
#include "StarJson.hpp"
#include "StarRandom.hpp"

namespace Star {

struct NameGeneratorExceptionTag { static constexpr char const* typeName = "NameGeneratorException"; };
using NameGeneratorException = TypedException<StarException, NameGeneratorExceptionTag>;

class PatternedNameGenerator;
using PatternedNameGeneratorPtr = SharedPtr<PatternedNameGenerator>;
using PatternedNameGeneratorConstPtr = SharedPtr<PatternedNameGenerator const>;

struct MarkovSource {
  size_t prefixSize;
  size_t endSize;
  StringList starts;
  StringMap<StringList> chains;
  StringSet ends;
};

class PatternedNameGenerator {
public:
  PatternedNameGenerator(AssetsConstPtr assets);

  String generateName(String const& rulesAsset) const;
  String generateName(String const& rulesAsset, uint64_t seed) const;
  String generateName(String const& rulesAsset, RandomSource& random) const;

private:
  String processRule(JsonArray const& rule, RandomSource& random) const;

  bool isProfane(String const& name) const;

  MarkovSource makeMarkovSource(size_t prefixSize, size_t endSize, StringList sourceNames);

  AssetsConstPtr m_assets;
  StringMap<MarkovSource> m_markovSources;
  StringSet m_profanityFilter;
};

}
