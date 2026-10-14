import star.json;
import star.file;
import star.lexical_cast;
import star.image;
import star.string;
import star.variant;
import star.ordered_map;
import star.ordered_set;
import std;
import star.format;
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
import star.random;
import star.color;
import star.multi_array;

import star.option_parser;
import star.version_option_parser;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.root_loader;

import star.terrain_database;

import star.vector;
import star.math_common;
using namespace Star;

int main(int argc, char** argv) {
  try {
    RootLoader rootLoader({{}, {}, {}, LogLevel::Error, false, {}});

    rootLoader.setSummary("Generate a heatmap image visualizing the output of a given terrain selector");

    rootLoader.addParameter("selector", "selector", OptionParser::Required, "name of the terrain selector to be rendered");
    rootLoader.addParameter("size", "size", OptionParser::Required, "x,y size of the region to be rendered");
    rootLoader.addParameter("seed", "seed", OptionParser::Optional, "seed value for the selector");
    rootLoader.addParameter("commonality", "commonality", OptionParser::Optional, "commonality value for the selector (default 1)");
    rootLoader.addParameter("scale", "scale", OptionParser::Optional, "maximum distance from 0 for color range");
    rootLoader.addParameter("mode", "mode", OptionParser::Optional, "color mode: heatmap, terrain");

    RootUPtr root;
    OptionParser::Options options;
    std::tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

    auto size = Vec2U(lexicalCast<unsigned>(options.parameters["size"].first().split(",")[0]), lexicalCast<unsigned>(options.parameters["size"].first().split(",")[1]));

    auto seed = Random::randu64();
    if (!options.parameters["seed"].empty())
      seed = lexicalCast<std::uint64_t>(options.parameters["seed"].first());

    float commonality = 1.0f;
    if (!options.parameters["commonality"].empty())
      commonality = lexicalCast<float>(options.parameters["commonality"].first());

    float scale = 1.0f;
    bool autoScale = true;
    if (!options.parameters["scale"].empty()) {
      autoScale = false;
      scale = lexicalCast<float>(options.parameters["scale"].first());
    }

    String mode = "heatmap";
    if (!options.parameters["mode"].empty())
      mode = options.parameters["mode"].first();

    auto selectorParameters = TerrainSelectorParameters(JsonObject{
        {"worldWidth", size[0]},
        {"baseHeight", size[1] / 2},
        {"seed", seed},
        {"commonality", commonality}
      });

    auto selector = Root::singleton().terrainDatabase()->createNamedSelector(options.parameters["selector"].first(), selectorParameters);

    MultiArray<float, 2> terrainResult({size[0], size[1]}, 0.0f);
    for (std::size_t x = 0; x < size[0]; ++x) {
      for (std::size_t y = 0; y < size[1]; ++y) {
        auto value = selector->get(x, y);
        terrainResult(x, y) = value;
        if (autoScale)
          scale = std::max(scale, std::abs(value));
      }
    }

    coutf("Generating {} size image for selector with scale {}\n", size, scale);
    auto outputImage = std::make_shared<Image>(size, PixelFormat::RGB24);

    for (std::size_t x = 0; x < size[0]; ++x) {
      for (std::size_t y = 0; y < size[1]; ++y) {
        // Image y = 0 is the top, so reverse it for the world position
        auto value = terrainResult(x, y) / scale;
        if (mode == "heatmap") {
          Color color = Color::rgb(255, 0, 0);
          color.setHue(clamp(value / 2 + 0.5f, 0.0f, 1.0f));
          outputImage->set(x, y, color.toRgb());
        } else if (mode == "terrain") {
          if (value > 0)
            outputImage->set(x, y, Vec3B(0, 100 + std::floor(155 * value), std::floor(255 * value)));
          else
            outputImage->set(x, y, Vec3B(std::floor(255 * -value), 0, 0));
        }
      }
    }

    outputImage->writePng(File::open("terrain.png", IOMode::Write));
    return 0;
  } catch (std::exception const& e) {
    cerrf("exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
