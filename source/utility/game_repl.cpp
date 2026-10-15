import star.json;
import star.lua;
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

import star.option_parser;
import star.version_option_parser;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.root_loader;
import star.utility_lua_bindings;

import star.root_lua_bindings;

using namespace Star;

int main(int argc, char** argv) {
  RootLoader rootLoader({{}, {}, {}, LogLevel::Error, false, {}});
  std::unique_ptr<::Star::Root> root;
  OptionParser::Options options;
  std::tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

  auto engine = LuaEngine::create(true);
  auto context = engine->createContext();
  context.setCallbacks("sb", LuaBindings::makeUtilityCallbacks());
  context.setCallbacks("root", LuaBindings::makeRootCallbacks());

  String code;
  bool continuation = false;
  while (!std::cin.eof()) {
    auto getline = [](std::istream& stream) -> String {
      std::string line;
      std::getline(stream, line);
      return String(std::move(line));
    };

    if (continuation) {
      std::cout << ">> ";
      std::cout.flush();
      code += getline(std::cin);
      code += '\n';
    } else {
      std::cout << "> ";
      std::cout.flush();
      code = getline(std::cin);
      code += '\n';
    }

    try {
      auto result = context.eval<LuaVariadic<LuaValue>>(code);
      for (auto r : result)
        coutf("{}\n", r);
      continuation = false;
    } catch (LuaIncompleteStatementException const&) {
      continuation = true;
    } catch (std::exception const& e) {
      coutf("Error: {}\n", outputException(e, false));
      continuation = false;
    }
  }
  return 0;
}
