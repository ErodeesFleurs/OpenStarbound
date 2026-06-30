#pragma once

#include "StarLua.hpp"
#include "StarSongbook.hpp"

namespace Star {

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeSongbookCallbacks(Songbook& songbook);
}
}
