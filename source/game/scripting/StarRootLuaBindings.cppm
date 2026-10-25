module;
import std;
import star.lua;
import star.format;

namespace Star { class Root; class ItemDatabase; }

export module star.root_lua_bindings;

export namespace Star::LuaBindings {
  // Root and an optional ready builder catalog are borrowed by the callbacks.
  // The override breaks ItemDatabase construction cycles and must outlive them.
  [[nodiscard]] std::expected<LuaCallbacks, StarException> makeRootCallbacks(Root& root, std::optional<std::reference_wrapper<ItemDatabase const>> itemCatalog = std::nullopt);
}

