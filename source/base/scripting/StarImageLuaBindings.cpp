#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarLua.hpp"
#include "StarLuaConverters.hpp"
#include "StarImage.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarAssetPath.hpp"
#include "StarRefPtr.hpp"

import star.image_lua_bindings;
import star.asset_source;
import star.assets;
import star.root_base;

namespace Star {

LuaMethods<Image> LuaUserDataMethods<Image>::make() {
  LuaMethods<Image> methods;

  methods.registerMethodWithSignature<Vec2U, Image&>("size", mem_fn(&Image::size));
  methods.registerMethodWithSignature<void, Image&, Vec2U, Image&>("drawInto", mem_fn(&Image::drawInto));
  methods.registerMethodWithSignature<void, Image&, Vec2U, Image&>("copyInto", mem_fn(&Image::copyInto));
  methods.registerMethod("set", [](Image& image, unsigned x, unsigned y, Color const& color) {
    image.set(x, y, color.toRgba()); 
  });

  methods.registerMethod("get", [](Image& image, unsigned x, unsigned y) {
    return Color::rgba(image.get(x, y));
  });

  methods.registerMethod("subImage", [](Image& image, Vec2U const& min, Vec2U const& size) {
    return image.subImage(min, size);
  });

  methods.registerMethod("process", [](Image& image, String const& directives) {
    return processImageOperations(parseImageOperations(directives), image, [](String const& path) -> Image const* {
      if (auto root = RootBase::singletonPtr())
        return root->assets()->image(path).get();
      else
        return nullptr;
    });
  });

  return methods;
}

}
