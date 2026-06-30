#pragma once

#include "StarLua.hpp"
#include "StarApplicationController.hpp"
#include "StarAssets.hpp"

namespace Star {

namespace LuaBindings {
[[nodiscard]] LuaCallbacks makeClipboardCallbacks(ApplicationControllerPtr appController, AssetsConstPtr assets, bool alwaysAllow, function<bool()> clipboardAllowed);
}

}// namespace Star
