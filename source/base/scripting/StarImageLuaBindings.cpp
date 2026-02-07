#include "StarImageLuaBindings.hpp"
#include "StarColor.hpp"
#include "StarImage.hpp"
#include "StarRootBase.hpp"

import std;

namespace Star {

auto LuaUserDataMethods<Image>::make() -> LuaMethods<Image> {
  LuaMethods<Image> methods;

  methods.registerMethodWithSignature<Vec2U, Image&>("size", std::mem_fn(&Image::size));
  methods.registerMethodWithSignature<void, Image&, Vec2U, Image&>("drawInto", std::mem_fn(&Image::drawInto));
  methods.registerMethodWithSignature<void, Image&, Vec2U, Image&>("copyInto", std::mem_fn(&Image::copyInto));
  methods.registerMethod("set", [](Image& image, unsigned x, unsigned y, Color const& color) -> void {
    image.set(x, y, color.toRgba());
  });

  methods.registerMethod("get", [](Image& image, unsigned x, unsigned y) -> Color {
    return Color::rgba(image.get(x, y));
  });

  methods.registerMethod("subImage", [](Image& image, Vec2U const& min, Vec2U const& size) -> Image {
    return image.subImage(min, size);
  });

  methods.registerMethod("process", [](Image& image, String const& directives) -> Image {
    return processImageOperations(parseImageOperations(directives), image, [](String const& path) -> Image const* {
      if (auto root = RootBase::singletonPtr())
        return root->assets()->image(path).get();
      else
        return nullptr;
    });
  });

  return methods;
}

}// namespace Star
