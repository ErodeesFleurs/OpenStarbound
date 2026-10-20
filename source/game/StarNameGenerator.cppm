module;

import std;
import star.string;
import star.format;
import star.json;
import star.random;

namespace Star {
class Root;


struct NameGeneratorExceptionTag {
  static constexpr char const* name() { return "NameGeneratorException"; }
};
using NameGeneratorException = StarError<NameGeneratorExceptionTag, StarException>;

struct MarkovSource {
  std::size_t prefixSize;
  std::size_t endSize;
  StringList starts;
  StringMap<StringList> chains;
  StringSet ends;
};

class PatternedNameGenerator {
public:
  explicit PatternedNameGenerator(Root& root);

  String generateName(String const& rulesAsset) const;
  String generateName(String const& rulesAsset, std::uint64_t seed) const;
  String generateName(String const& rulesAsset, RandomSource& random) const;

private:
  String processRule(JsonArray const& rule, RandomSource& random) const;

  bool isProfane(String const& name) const;

  MarkovSource makeMarkovSource(std::size_t prefixSize, std::size_t endSize, StringList sourceNames);

  Root& m_root;
  StringMap<MarkovSource> m_markovSources;
  StringSet m_profanityFilter;
};

}

export module star.name_generator;

export namespace Star {
  using ::Star::NameGeneratorExceptionTag;
  using ::Star::NameGeneratorException;
  using ::Star::MarkovSource;
  using ::Star::PatternedNameGenerator;
}
