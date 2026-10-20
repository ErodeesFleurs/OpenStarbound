import std;
import star.string;
import star.json;
import star.json_extra;
import star.lua;
import star.lua_converters;
import star.image;
import star.color;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;

import star.image_lua_bindings;

import star.vector;
namespace Star {

LuaMethods<Image> LuaUserDataMethods<Image>::make() {
  LuaMethods<Image> methods;

  methods.registerMethodWithSignature<Vec2U, Image&>("size", std::mem_fn(&Image::size));
  methods.registerMethodWithSignature<void, Image&, Vec2U, Image&>("drawInto", std::mem_fn(&Image::drawInto));
  methods.registerMethodWithSignature<void, Image&, Vec2U, Image&>("copyInto", std::mem_fn(&Image::copyInto));
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
    List<std::shared_ptr<Image const>> resolvedImages;
    return processImageOperations(parseImageOperations(directives), image, [&](String const& path) -> Image const* {
      if (!engine.imageResolver())
        return nullptr;
      auto resolved = engine.imageResolver()(path);
      auto ptr = resolved.get();
      resolvedImages.append(std::move(resolved));
      return ptr;
    });
  });

  return methods;
}

}
