module;

import std;

namespace Star {
class LuaCallbacks;
class ApplicationController;
class Input;
class RootBase;
class Logger;
}

export module star.clipboard_lua_bindings;

export namespace Star::LuaBindings {
LuaCallbacks makeClipboardCallbacks(Input& input, RootBase& root, Logger& logger, std::shared_ptr<ApplicationController> appController, bool alwaysAllow);
}
