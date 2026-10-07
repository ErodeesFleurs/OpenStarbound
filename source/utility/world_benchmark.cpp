#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarLexicalCast.hpp"
#include "StarLogging.hpp"
#include "StarString.hpp"
#include "StarVariant.hpp"
#include "StarOrderedMap.hpp"
#include "StarOrderedSet.hpp"
#include "StarVersion.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarAssetPath.hpp"
#include "StarRefPtr.hpp"
#include "StarListener.hpp"
#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarMultiArray.hpp"
#include "StarCasting.hpp"
#include "StarEither.hpp"
#include "StarImage.hpp"
#include "StarColor.hpp"
#include "StarInterpolation.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include "StarDataStream.hpp"
#include "StarMaybe.hpp"
#include "StarDirectives.hpp"
#include "StarWeightedPool.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarRpcPromise.hpp"
#include "StarSectorArray2D.hpp"
#include <functional>
#include "StarNetCompatibility.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarSet.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarMap.hpp"
#include "StarRandom.hpp"
#include "StarBlockAllocator.hpp"
#include "StarLruCache.hpp"

import star.option_parser;
import star.version_option_parser;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.root_loader;
#include "StarLuaRoot.hpp"
import star.worker_pool;
import star.tile_sector_array;
import star.collision_block;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.liquid_types;
import star.tile_damage;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.warping;
import star.wiring;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.cellular_light_array;
import star.cellular_lighting;
import star.cellular_liquid;
import star.world_structure;
#include "StarLuaComponents.hpp"
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.world_render_data;
import star.world_client_state;
import star.interpolation_tracker;
import star.spawn_type_database;
import star.spawner;
import star.world_server;
import star.world_template;

using namespace Star;

int main(int argc, char** argv) {
  try {
    RootLoader rootLoader({{}, {}, {}, LogLevel::Error, false, {}});
    rootLoader.addArgument("dungeon", OptionParser::Required, "name of the dungeon to spawn in the world to benchmark");
    rootLoader.addParameter("seed", "seed", OptionParser::Optional, "world seed used to create the WorldTemplate");
    rootLoader.addParameter("steps", "steps", OptionParser::Optional, "number of steps to run the world for, defaults to 5,000");
    rootLoader.addParameter("times", "times", OptionParser::Optional, "how many times to perform the run, defaults to once");
    rootLoader.addParameter("signalevery", "signal steps", OptionParser::Optional, "number of steps to wait between scanning and signaling all entities to stay alive, default 120");
    rootLoader.addParameter("reportevery", "report steps", OptionParser::Optional, "number of steps between each progress report, default 0 (do not report progress)");
    rootLoader.addParameter("fidelity", "server fidelity", OptionParser::Optional, "fidelity to run the server with, default high");
    rootLoader.addSwitch("profiling", "whether to use lua profiling, prints the profile with info logging");
    rootLoader.addSwitch("unsafe", "enables unsafe lua libraries");
    RootUPtr root;
    OptionParser::Options options;
    tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

    coutf("Fully loading root...");
    root->fullyLoad();
    coutf(" done\n");

    String dungeon = options.arguments.first();
    VisitableWorldParametersPtr worldParameters = generateFloatingDungeonWorldParameters(dungeon);
    uint64_t worldSeed = Random::randu64();
    if (options.parameters.contains("seed"))
      worldSeed = lexicalCast<uint64_t>(options.parameters.get("seed").first());
    auto worldTemplate = make_shared<WorldTemplate>(worldParameters, SkyParameters(), worldSeed);

    auto fidelity = options.parameters.maybe("fidelity").apply([](StringList p) { return p.maybeFirst(); }).value({});
    root->configuration()->set("serverFidelity", fidelity.value("high"));

    if (options.switches.contains("unsafe"))
      root->configuration()->set("safeScripts", false);
    if (options.switches.contains("profiling")) {
      root->configuration()->set("scriptProfilingEnabled", true);
      root->configuration()->set("scriptInstructionMeasureInterval", 100);
    }

    uint64_t times = 1;
    if (options.parameters.contains("times"))
      times = lexicalCast<uint64_t>(options.parameters.get("times").first());

    uint64_t steps = 5000;
    if (options.parameters.contains("steps"))
      steps = lexicalCast<uint64_t>(options.parameters.get("steps").first());

    uint64_t signalEvery = 120;
    if (options.parameters.contains("signalevery"))
      signalEvery = lexicalCast<uint64_t>(options.parameters.get("signalevery").first());

    uint64_t reportEvery = 0;
    if (options.parameters.contains("reportevery"))
      reportEvery = lexicalCast<uint64_t>(options.parameters.get("reportevery").first());

    double sumTime = 0.0;
    for (uint64_t i = 0; i < times; ++i) {
      WorldServer worldServer(worldTemplate, File::ephemeralFile());

      coutf("Starting world simulation for {} steps\n", steps);
      double start = Time::monotonicTime();
      double lastReport = Time::monotonicTime();
      uint64_t entityCount = 0;
      for (uint64_t j = 0; j < steps; ++j) {
        if (j % signalEvery == 0) {
          entityCount = 0;
          worldServer.forEachEntity(RectF(Vec2F(), Vec2F(worldServer.geometry().size())), [&](auto const& entity) {
              ++entityCount;
              worldServer.signalRegion(RectI::integral(entity->metaBoundBox().translated(entity->position())));
            });
        }

        if (reportEvery != 0 && j % reportEvery == 0) {
          float fps = reportEvery / (Time::monotonicTime() - lastReport);
          lastReport = Time::monotonicTime();
          coutf("[{}] {}s | FPS: {} | Entities: {}\n", j, Time::monotonicTime() - start, fps, entityCount);
        }
        worldServer.update(ServerGlobalTimestep * GlobalTimescale);
      }
      double totalTime = Time::monotonicTime() - start;
      coutf("Finished run of running dungeon world '{}' with seed {} for {} steps in {} seconds, average FPS: {}\n",
            dungeon, worldSeed, steps, totalTime, steps / totalTime);
      sumTime += totalTime;
    }

    if (times != 1) {
      coutf("Average of all runs - time: {}, FPS: {}\n", sumTime / times, steps / (sumTime / times));
    }

    return 0;
  } catch (std::exception const& e) {
    cerrf("Exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
