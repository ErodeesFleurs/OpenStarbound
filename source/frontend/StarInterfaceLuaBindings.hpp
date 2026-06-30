#pragma once

#include "StarLua.hpp"

namespace Star {

class MainInterface;
using MainInterfacePtr = SharedPtr<MainInterface>;
class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;

namespace LuaBindings {
  LuaCallbacks makeInterfaceCallbacks(MainInterface* mainInterface);
  LuaCallbacks makeChatCallbacks(MainInterface* mainInterface, UniverseClient* client);
}

}
