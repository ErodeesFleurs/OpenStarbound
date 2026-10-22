import std;
import star.json;
import star.native_runtime;
import star.lua;
import star.string;
import star.format;
import star.output_proxy;
import star.logging;
import star.option_parser;
import star.assets;
import star.image;
import star.asset_path;
import star.root;
import star.root_loader;
import star.utility_lua_bindings;
import star.root_lua_bindings;
import star.lua_root;

using namespace Star;

int main(int argc, char** argv) {
  auto run = [&]() -> std::expected<int, StarException> {
    auto nativeRuntime = NativeRuntime::create();
    if (!nativeRuntime)
      return std::unexpected(std::move(nativeRuntime.error()));
    auto rootLoader = RootLoader::create({{}, {}, {}, LogLevel::Error, false, {}});
    if (!rootLoader)
      return std::unexpected(std::move(rootLoader.error()));
    auto initialized = rootLoader->commandInitOrDie(argc, argv);
    if (!initialized)
      return std::unexpected(std::move(initialized.error()));
    auto& root = initialized->first;
    auto engine = LuaEngine::create(true);
    if (!engine)
      return std::unexpected(std::move(engine.error()));
    (*engine)->setExecutionHost(std::make_unique<LuaExecutionScope>(*root));
    (*engine)->setImageResolver([root = root.get()](String const& path) -> std::expected<std::shared_ptr<Image const>, StarException> {
      auto assets = root->assets();
      if (!assets)
        return std::unexpected(std::move(assets.error()));
      return (*assets)->image(path);
    });
    auto context = (*engine)->createContext();
    if (!context)
      return std::unexpected(std::move(context.error()));
    auto utilityCallbacks = LuaBindings::makeUtilityCallbacks(root->random(), root->logger(), root->logMap());
    if (!utilityCallbacks)
      return std::unexpected(std::move(utilityCallbacks.error()));
    auto registered = context->setCallbacks("sb", *utilityCallbacks);
    if (!registered)
      return std::unexpected(std::move(registered.error()));
    auto rootCallbacks = LuaBindings::makeRootCallbacks(*root);
    if (!rootCallbacks)
      return std::unexpected(std::move(rootCallbacks.error()));
    registered = context->setCallbacks("root", *rootCallbacks);
    if (!registered)
      return std::unexpected(std::move(registered.error()));

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
        code += "\n";
      } else {
        std::cout << "> ";
        std::cout.flush();
        code = getline(std::cin);
        code += "\n";
      }

      auto evaluated = [&]() -> std::expected<void, StarException> {
        auto values = context->eval<LuaVariadic<LuaValue>>(code);
        if (!values)
          return std::unexpected(std::move(values.error()));
        for (auto const& value : *values) {
          auto text = printLuaValue(value);
          if (!text)
            return std::unexpected(std::move(text.error()));
          coutf("{}\n", *text);
        }
        return {};
      }();
      if (evaluated) {
        continuation = false;
      } else if (evaluated.error().is<LuaIncompleteStatementException>()) {
        continuation = true;
      } else {
        coutf("Error: {}\n", outputException(ExceptionDiagnostic(evaluated.error()), false));
        continuation = false;
      }
    }
    return 0;
  };
  auto result = run();
  if (!result) {
    cerrf("exception caught: {}\n", outputException(ExceptionDiagnostic(result.error()), true));
    return 1;
  }
  return *result;
}
