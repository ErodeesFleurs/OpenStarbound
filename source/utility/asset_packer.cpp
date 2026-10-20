import star.io_device;
import star.json;
import star.asset_source;
import star.string;
import star.directory_asset_source;
import star.thread;
import star.time;
import star.json_extra;
import star.file;
import star.variant;
import star.ordered_map;
import star.ordered_set;
import star.option_parser;
import std;
import star.format;
import star.version;
import star.version_option_parser;
import star.packed_asset_source;
import star.logging;
import star.native_runtime;

using namespace Star;

int main(int argc, char** argv) {
  [[maybe_unused]] Star::NativeRuntime nativeRuntime;
  try {
    Logger logger;
    double startTime = Time::monotonicTime();

    VersionOptionParser optParse;
    optParse.setVersionName("Asset Packer");
    optParse.setSummary("Packs asset folder into a starbound .pak file");
    optParse.addParameter("c", "configFile", OptionParser::Optional, "JSON file with ignore lists and ordering info");
    optParse.addSwitch("s", "Enable server mode");
    optParse.addSwitch("v", "Verbose, list each file added");
    optParse.addArgument("assets folder path", OptionParser::Required, "Path to the assets to be packed");
    optParse.addArgument("output filename", OptionParser::Required, "Output pak file");

    auto opts = optParse.commandParseOrDie(argc, argv);

    String assetsFolderPath = opts.arguments.at(0);
    String outputFilename = opts.arguments.at(1);

    StringList ignoreFiles;
    StringList extensionOrdering;
    if (opts.parameters.contains("c")) {
      String configFile = opts.parameters.get("c").first();
      String configFileContents;
      try {
        configFileContents = File::readFileString(configFile);
      } catch (IOException const& e) {
        cerrf("Could not open specified configFile: {}\n", configFile);
        cerrf("For the following reason: {}\n", outputException(e, false));
        return 1;
      }

      Json configFileJson;
      try {
        configFileJson = Json::parseJson(configFileContents);
      } catch (JsonParsingException const& e) {
        cerrf("Could not parse the specified configFile: {}\n", configFile);
        cerrf("For the following reason: {}\n", outputException(e, false));
        return 1;
      }

      try {
        ignoreFiles = jsonToStringList(configFileJson.get("globalIgnore", JsonArray()));
        if (opts.switches.contains("s"))
          ignoreFiles.appendAll(jsonToStringList(configFileJson.get("serverIgnore", JsonArray())));
        extensionOrdering = jsonToStringList(configFileJson.get("extensionOrdering", JsonArray()));
      } catch (JsonException const& e) {
        cerrf("Could not read the asset_packer config file {}\n", configFile);
        cerrf("{}\n", outputException(e, false));
        return 1;
      }
    }

    bool verbose = opts.switches.contains("v");

    std::function<void(std::size_t, std::size_t, String, String, bool)> BuildProgressCallback;
    auto progressCallback = [verbose](std::size_t, std::size_t, String filePath, String assetPath) {
      if (verbose)
        coutf("Adding file '{}' to the target pak as '{}'\n", filePath, assetPath);
    };

    outputFilename = File::relativeTo(File::fullPath(File::dirName(outputFilename)), File::baseName(outputFilename));
    DirectoryAssetSource directorySource(logger, assetsFolderPath, ignoreFiles);
    PackedAssetSource::build(directorySource, outputFilename, extensionOrdering, progressCallback);

    coutf("Output packed assets to {} in {}s\n", outputFilename, Time::monotonicTime() - startTime);
    return 0;

  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    cerrf("Exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
