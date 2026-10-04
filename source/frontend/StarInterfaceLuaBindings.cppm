module;

namespace Star {
class LuaCallbacks;
class MainInterface;
class UniverseClient;
}

export module star.interface_lua_bindings;

export namespace Star::LuaBindings {
  LuaCallbacks makeInterfaceCallbacks(MainInterface* mainInterface);
  LuaCallbacks makeChatCallbacks(MainInterface* mainInterface, UniverseClient* client);
}
