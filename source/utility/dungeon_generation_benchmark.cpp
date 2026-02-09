#include "StarConfig.hpp"
#include "StarRootLoader.hpp"
#include "StarWorldServer.hpp"
#include "StarWorldTemplate.hpp"

import std;

using namespace Star;

auto main(int argc, char** argv) -> int {
  try {
    unsigned repetitions = 5;
    unsigned reportEvery = 1;
    String dungeonWorldName = "outpost";

    RootLoader rootLoader({.additionalAssetsSettings = {}, .additionalDefaultConfiguration = {}, .logFile = {}, .logLevel = LogLevel::Error, .quiet = false, .runtimeConfigFile = {}});
    rootLoader.addParameter("dungeonWorld", "dungeonWorld", OptionParser::Optional, strf("dungeonWorld to test, default is {}", dungeonWorldName));
    rootLoader.addParameter("repetitions", "repetitions", OptionParser::Optional, strf("number of times to generate, default {}", repetitions));
    rootLoader.addParameter("reportevery", "report repetitions", OptionParser::Optional, strf("number of repetitions before each progress report, default {}", reportEvery));

    UPtr<Root> root;
    OptionParser::Options options;
    std::tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

    coutf("Fully loading root...");
    root->fullyLoad();
    coutf(" done\n");

    if (auto repetitionsOption = options.parameters.maybe("repetitions"))
      repetitions = lexicalCast<unsigned>(repetitionsOption->first());

    if (auto reportEveryOption = options.parameters.maybe("reportevery"))
      reportEvery = lexicalCast<unsigned>(reportEveryOption->first());

    if (auto dungeonWorldOption = options.parameters.maybe("dungeonWorld"))
      dungeonWorldName = dungeonWorldOption->first();

    double start = Time::monotonicTime();
    double lastReport = Time::monotonicTime();

    coutf("testing {} generations of dungeonWorld {}\n", repetitions, dungeonWorldName);

    for (unsigned i = 0; i < repetitions; ++i) {
      if (i > 0 && i % reportEvery == 0) {
        float gps = reportEvery / (Time::monotonicTime() - lastReport);
        lastReport = Time::monotonicTime();
        coutf("[{}] {}s | Generations Per Second: {}\n", i, Time::monotonicTime() - start, gps);
      }

      Ptr<VisitableWorldParameters> worldParameters = generateFloatingDungeonWorldParameters(dungeonWorldName);
      auto worldTemplate = make_shared<WorldTemplate>(worldParameters, SkyParameters(), 1234);
      WorldServer worldServer(std::move(worldTemplate), File::ephemeralFile());
    }

    coutf("Finished {} generations of dungeonWorld {} in {} seconds", repetitions, dungeonWorldName, Time::monotonicTime() - start);

    return 0;

  } catch (std::exception const& e) {
    cerrf("Exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
