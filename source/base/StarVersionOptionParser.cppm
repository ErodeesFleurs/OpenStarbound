module;

import std;
import star.string;
import star.variant;
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

  VersionOptionParser();

  // Parse the command line options, or, in the case of an error, -h, or -v,
  // prints the appropriate text and immediately exits.
  Options parseOrDie(StringList const& cmdLineArguments) const;

  // First sets the command name based on argv[0], then calls parseOrDie.
  Options commandParseOrDie(int argc, char** argv);

private:
  std::optional<String> m_versionName;
};

}

export module star.version_option_parser;

export namespace Star {
  using ::Star::VersionOptionParser;
}
