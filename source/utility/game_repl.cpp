#include "StarJson.hpp"
#include "StarString.hpp"
#include "StarVariant.hpp"
#include "StarOrderedMap.hpp"
#include "StarOrderedSet.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;

import star.option_parser;
import star.version_option_parser;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.root_loader;
#include "StarUtilityLuaBindings.hpp"

import star.root_lua_bindings;

using namespace Star;

int main(int argc, char** argv) {
  RootLoader rootLoader({{}, {}, {}, LogLevel::Error, false, {}});
  RootUPtr root;
  OptionParser::Options options;
  tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

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
