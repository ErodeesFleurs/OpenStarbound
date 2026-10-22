module;
import std;
import star.lua;
import star.format;

namespace Star { class Root; class ItemDatabase; }

export module star.root_lua_bindings;

export namespace Star::LuaBindings {
  // Root and an optional ready builder catalog are borrowed by the callbacks.
  // The catalog override breaks ItemDatabase's own script-construction cycle.
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeRootCallbacks(Root& root, ItemDatabase const* itemCatalog = nullptr);
}

