#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarEither.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
import star.sky_types;
#include "StarMaybe.hpp"
#include "StarWeightedPool.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_types;
#include "StarRootLoader.hpp"
#include "StarWorldTemplate.hpp"
#include "StarWorldServer.hpp"


import star.celestial_database;

using namespace Star;

int main(int argc, char** argv) {
  try {
    unsigned repetitions = 5;
    unsigned reportEvery = 1;
    String dungeonWorldName = "outpost";

    RootLoader rootLoader({{}, {}, {}, LogLevel::Error, false, {}});
    rootLoader.addParameter("dungeonWorld", "dungeonWorld", OptionParser::Optional, strf("dungeonWorld to test, default is {}", dungeonWorldName));
    rootLoader.addParameter("repetitions", "repetitions", OptionParser::Optional, strf("number of times to generate, default {}", repetitions));
    rootLoader.addParameter("reportevery", "report repetitions", OptionParser::Optional, strf("number of repetitions before each progress report, default {}", reportEvery));

    RootUPtr root;
    OptionParser::Options options;
    tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

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

      VisitableWorldParametersPtr worldParameters = generateFloatingDungeonWorldParameters(dungeonWorldName);
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
