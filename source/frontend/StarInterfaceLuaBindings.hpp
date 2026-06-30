#pragma once

#include "StarLua.hpp"

namespace Star {

class MainInterface;
using MainInterfacePtr = SharedPtr<MainInterface>;
class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeInterfaceCallbacks(MainInterface& mainInterface);
  [[nodiscard]] LuaCallbacks makeChatCallbacks(MainInterface& mainInterface, UniverseClient& client);
}

}
