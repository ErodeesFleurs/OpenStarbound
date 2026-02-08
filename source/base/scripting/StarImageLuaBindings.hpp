#pragma once

#include "StarLua.hpp"
#include "StarLuaConverters.hpp" // IWYU pragma: export

namespace Star {

template <>
struct LuaConverter<Image> : LuaUserDataConverter<Image> {};

template <>
struct LuaUserDataMethods<Image> {
  static auto make() -> LuaMethods<Image>;
};

}
