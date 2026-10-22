module;

import std;
import star.format;
import star.string;
import star.ordered_map;
import star.ordered_set;
import star.option_parser;
import star.version;

namespace Star {

// Option parser that accepts -h to print the help and exit and -v to print the
// version and exit.
class VersionOptionParser : public OptionParser {
public:
  String getVersionString() const;
  void setVersionName(String const& name);

  [[nodiscard]] static std::expected<VersionOptionParser, StarException> create();

  // Parse the command line options, or, in the case of an error, -h, or -v,
  // prints the appropriate text and immediately exits.
  [[nodiscard]] std::expected<Options, StarException> parseOrDie(StringList const& cmdLineArguments) const;

  // First sets the command name based on argv[0], then calls parseOrDie.
  [[nodiscard]] std::expected<Options, StarException> commandParseOrDie(int argc, char** argv);

protected:
  VersionOptionParser() = default;
  [[nodiscard]] std::expected<void, StarException> initialize();

private:
  std::optional<String> m_versionName;
};

}

export module star.version_option_parser;

export namespace Star {
  using ::Star::VersionOptionParser;
}
