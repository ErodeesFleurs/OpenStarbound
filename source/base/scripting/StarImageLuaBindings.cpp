#include "StarImageLuaBindings.hpp"
#include "StarLuaConverters.hpp"
#include "StarImage.hpp"
#include "StarAssets.hpp"

namespace Star {

void LuaBindings::registerImageLuaAssets(LuaEngine& engine, AssetsConstPtr assets) {
  engine.setService<Assets const>(std::move(assets));
}

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

  methods.registerMethod("process", [](Image& image, LuaEngine& engine, String const& directives) {
    auto assets = engine.service<Assets const>();
    return processImageOperations(parseImageOperations(directives), image, [assets](String const& path) -> Image const* {
      return assets ? assets->image(path).get() : nullptr;
    });
  });

  return methods;
}

}
