module;

import std;

namespace Star {
class LuaCallbacks;
class ApplicationController;
}

export module star.clipboard_lua_bindings;

export namespace Star::LuaBindings {
LuaCallbacks makeClipboardCallbacks(std::shared_ptr<ApplicationController> appController, bool alwaysAllow);
}
