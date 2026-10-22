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
  [[nodiscard]] static std::expected<std::shared_ptr<PatternedNameGenerator>, StarException> create(Root& root);

  [[nodiscard]] std::expected<String, StarException> generateName(String const& rulesAsset) const;
  [[nodiscard]] std::expected<String, StarException> generateName(String const& rulesAsset, std::uint64_t seed) const;
  [[nodiscard]] std::expected<String, StarException> generateName(String const& rulesAsset, RandomSource& random) const;

private:
  explicit PatternedNameGenerator(Root& root);
  [[nodiscard]] std::expected<String, StarException> processRule(JsonArray const& rule, RandomSource& random) const;

  [[nodiscard]] std::expected<bool, StarException> isProfane(String const& name) const;

  [[nodiscard]] std::expected<MarkovSource, StarException> makeMarkovSource(std::size_t prefixSize, std::size_t endSize, StringList sourceNames);

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
