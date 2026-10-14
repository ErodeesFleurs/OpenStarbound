module;

import std;
import star.string;
import star.format;
import star.json;
import star.random;

namespace Star {

struct NameGeneratorExceptionTag {
  static constexpr char const* name() { return "NameGeneratorException"; }
};
using NameGeneratorException = StarError<NameGeneratorExceptionTag, StarException>;

class PatternedNameGenerator;
using PatternedNameGeneratorPtr = std::shared_ptr<PatternedNameGenerator>;
using PatternedNameGeneratorConstPtr = std::shared_ptr<const PatternedNameGenerator>;
using PatternedNameGeneratorWeakPtr = std::weak_ptr<PatternedNameGenerator>;
using PatternedNameGeneratorConstWeakPtr = std::weak_ptr<const PatternedNameGenerator>;
using PatternedNameGeneratorUPtr = std::unique_ptr<PatternedNameGenerator>;
using PatternedNameGeneratorConstUPtr = std::unique_ptr<const PatternedNameGenerator>;

struct MarkovSource {
  std::size_t prefixSize;
  std::size_t endSize;
  StringList starts;
  StringMap<StringList> chains;
  StringSet ends;
};

class PatternedNameGenerator {
public:
  PatternedNameGenerator();

  String generateName(String const& rulesAsset) const;
  String generateName(String const& rulesAsset, std::uint64_t seed) const;
  String generateName(String const& rulesAsset, RandomSource& random) const;

private:
  String processRule(JsonArray const& rule, RandomSource& random) const;

  bool isProfane(String const& name) const;

  MarkovSource makeMarkovSource(std::size_t prefixSize, std::size_t endSize, StringList sourceNames);

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
  using ::Star::PatternedNameGeneratorPtr;
  using ::Star::PatternedNameGeneratorConstPtr;
  using ::Star::PatternedNameGeneratorWeakPtr;
  using ::Star::PatternedNameGeneratorConstWeakPtr;
  using ::Star::PatternedNameGeneratorUPtr;
  using ::Star::PatternedNameGeneratorConstUPtr;
}
