#include "StarConfig.hpp"
#include "StarRootLoader.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarUtilityLuaBindings.hpp"

import std;

using namespace Star;

auto main(int argc, char** argv) -> int {
  RootLoader rootLoader({.additionalAssetsSettings = {}, .additionalDefaultConfiguration = {}, .logFile = {}, .logLevel = LogLevel::Error, .quiet = false, .runtimeConfigFile = {}});
  UPtr<Root> root;
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
      return {std::move(line)};
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
