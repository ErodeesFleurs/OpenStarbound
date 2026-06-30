#pragma once

#include "StarLua.hpp"

namespace Star {

class Image;
class Assets;
using AssetsConstPtr = SharedPtr<Assets const>;
class LuaEngine;

template <>
struct LuaConverter<Image> : LuaUserDataConverter<Image> {};

template <>
struct LuaUserDataMethods<Image> {
  static LuaMethods<Image> make();
};

namespace LuaBindings {
  void registerImageLuaAssets(LuaEngine& engine, AssetsConstPtr assets);
}

}
