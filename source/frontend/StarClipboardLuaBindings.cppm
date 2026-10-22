module;

import std;
import star.format;

namespace Star {
class LuaCallbacks;
class ApplicationController;
class Input;
class RootBase;
class Logger;
}

export module star.clipboard_lua_bindings;

export namespace Star::LuaBindings {
[[nodiscard]] std::expected<LuaCallbacks, StarException> makeClipboardCallbacks(Input& input, RootBase& root, Logger& logger, std::shared_ptr<ApplicationController> appController, bool alwaysAllow);
}
