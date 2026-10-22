module;
import std;
import star.format;
import star.type_identity;
import star.string;
import star.json;
import star.id_map;
import star.casting;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.lua;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;

namespace Star {

// All ScriptedEntity methods should only be called on master entities
class ScriptedEntity : public virtual Entity {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<ScriptedEntity, Entity>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<ScriptedEntity>();
  }

  // Missing/unloaded script contexts return an empty optional; execution errors
  // are returned separately without losing the diagnostic.
  [[nodiscard]] virtual std::expected<std::optional<LuaValue>, StarException> callScript(String const& func, LuaVariadic<LuaValue> const& args) = 0;

  [[nodiscard]] virtual std::expected<std::optional<LuaValue>, StarException> evalScript(String const& code) = 0;
};

}

export module star.scripted_entity;

export namespace Star {
  using ::Star::ScriptedEntity;
}
