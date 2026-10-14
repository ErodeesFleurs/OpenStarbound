module;
import star.json;
import star.string;
import star.variant;
import star.ordered_map;
import star.ordered_set;
import std;
import star.maybe;
import star.version;
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

import star.option_parser;
import star.version_option_parser;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

namespace Star {

// Parses command line flags and loads and returns the Root singleton based on
// them.
//
// It is designed to load settings first from the settings passed into the
// constructor, then from the required boot config file , then from any passed
// in command line flags.  Besides -version, the accepted command line flags
// are:
//
// -bootconfig <bootconfig> - sets path to the boot configuration file, defaults to sbinit.config
// -logfile <logfile> - sets path to logfile, if any, relative to root storage directory
// -loglevel <level> - sets logging level
// -quiet - turns off stdout logging
// -verbose - turns on stdout logging
// -runtimeconfig - sets the path to the runtime configuration storage file, relative to root storage directory
//
// The boot config file can contain the following options:
// 'assetDirectories' - Asset source search directories
// 'logFileBackups' - Number of rotated backups of the target log file
// 'storageDirectory' - Primary Root storage directory
// 'assetsSettings' - Merged with base assets settings
// 'defaultConfiguration' - Merged with base default configuration
class RootLoader : public VersionOptionParser {
public:
  struct Defaults {
    // Merged on top of the base assets settings hard-coded in RootLoader.
    Json additionalAssetsSettings;

    // Merged on top of the base default configuration hard-coded in RootLoader.
    Json additionalDefaultConfiguration;

    // Name of the log file that should be written, if any, relative to the
    // storage directory
    Maybe<String> logFile;

    // The minimum log level to write to any log sink
    LogLevel logLevel;

    // If true, doesn't write any logging to stdout, only to the log file if
    // given.
    bool quiet;

    // If given, will write changed configuration to the given file within the
    // storage directory.
    Maybe<String> runtimeConfigFile;
  };

  RootLoader(Defaults defaults);

  std::pair<Root::Settings, Options> parseOrDie(StringList const& cmdLineArguments) const;
  std::pair<RootUPtr, Options> initOrDie(StringList const& cmdLineArguments) const;

  std::pair<Root::Settings, Options> commandParseOrDie(int argc, char** argv);
  std::pair<RootUPtr, Options> commandInitOrDie(int argc, char** argv);

private:
  Root::Settings rootSettingsForOptions(Options const& options) const;

  Defaults m_defaults;
};

}

export module star.root_loader;

export namespace Star {
  using ::Star::RootLoader;
}
