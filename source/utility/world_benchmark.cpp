import star.json;
import star.native_runtime;
import star.file;
import star.xx_hash;
import star.lexical_cast;
import star.logging;
import star.string;
import star.variant;
import star.ordered_map;
import star.ordered_set;
import star.time;
import star.format;
import star.version;
import star.rect;
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
import star.listener;
import star.id_map;
import star.casting;
import star.either;
import star.image;
import star.color;
import star.vector;
import star.game_types;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.strong_typedef;
import star.rpc_promise;
import star.set;
import star.worker_pool;

import std;
import star.sector_array_2d;
import star.net_compatibility;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.data_stream_devices;
import star.btree_database;
import star.map;

import star.option_parser;
import star.version_option_parser;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.root_loader;
import star.lua_root;
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
import star.lua_components;
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
  [[maybe_unused]] Star::NativeRuntime nativeRuntime;
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
    std::unique_ptr<::Star::Root> root;
    OptionParser::Options options;
    std::tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

    coutf("Fully loading root...");
    root->fullyLoad();
    coutf(" done\n");

    String dungeon = options.arguments.first();
    std::shared_ptr<::Star::VisitableWorldParameters> worldParameters = generateFloatingDungeonWorldParameters(*root, dungeon);
    std::uint64_t worldSeed = root->random().randu64();
    if (options.parameters.contains("seed"))
      worldSeed = lexicalCast<std::uint64_t>(options.parameters.get("seed").first());
    auto worldTemplate = std::make_shared<WorldTemplate>(*root, worldParameters, SkyParameters(), worldSeed);

    auto fidelity = options.parameters.maybe("fidelity").apply([](StringList p) { return p.maybeFirst(); }).value({});
    root->configuration()->set("serverFidelity", fidelity.value("high"));

    if (options.switches.contains("unsafe"))
      root->configuration()->set("safeScripts", false);
    if (options.switches.contains("profiling")) {
      root->configuration()->set("scriptProfilingEnabled", true);
      root->configuration()->set("scriptInstructionMeasureInterval", 100);
    }

    std::uint64_t times = 1;
    if (options.parameters.contains("times"))
      times = lexicalCast<std::uint64_t>(options.parameters.get("times").first());

    std::uint64_t steps = 5000;
    if (options.parameters.contains("steps"))
      steps = lexicalCast<std::uint64_t>(options.parameters.get("steps").first());

    std::uint64_t signalEvery = 120;
    if (options.parameters.contains("signalevery"))
      signalEvery = lexicalCast<std::uint64_t>(options.parameters.get("signalevery").first());

    std::uint64_t reportEvery = 0;
    if (options.parameters.contains("reportevery"))
      reportEvery = lexicalCast<std::uint64_t>(options.parameters.get("reportevery").first());

    double sumTime = 0.0;
    for (std::uint64_t i = 0; i < times; ++i) {
      WorldServer worldServer(*root, worldTemplate, File::ephemeralFile(root->random()));

      coutf("Starting world simulation for {} steps\n", steps);
      double start = Time::monotonicTime();
      double lastReport = Time::monotonicTime();
      std::uint64_t entityCount = 0;
      for (std::uint64_t j = 0; j < steps; ++j) {
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
        worldServer.update(root->timing().serverTimestep * root->timing().timescale);
      }
      double totalTime = Time::monotonicTime() - start;
      coutf("Finished run of running dungeon world '{}' with seed {} for {} steps in {} seconds, average FPS: {}\n",
            dungeon, worldSeed, steps, totalTime, steps / totalTime);
      sumTime += totalTime;
      worldServer.close();
    }

    if (times != 1) {
      coutf("Average of all runs - time: {}, FPS: {}\n", sumTime / times, steps / (sumTime / times));
    }

    return 0;
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    cerrf("Exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
