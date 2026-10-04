module;

#include <memory>

namespace Star {
class LuaCallbacks;
class ApplicationController;
using ApplicationControllerPtr = std::shared_ptr<ApplicationController>;
}

export module star.clipboard_lua_bindings;

export namespace Star::LuaBindings {
LuaCallbacks makeClipboardCallbacks(ApplicationControllerPtr appController, bool alwaysAllow);
}
