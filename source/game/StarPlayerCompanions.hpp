#pragma once

#include <optional>

#include "StarAiTypes.hpp"
#include "StarUuid.hpp"
#include "StarDrawable.hpp"
#include "StarLuaComponents.hpp"
#include "StarWorld.hpp"

namespace Star {

STAR_CLASS(Companion);
STAR_CLASS(PlayerCompanions);

class Companion {
public:
  Companion(Json const& json);
  Json toJson() const;

  Uuid podUuid() const;
  std::optional<String> name() const;
  std::optional<String> description() const;

  List<Drawable> portrait() const;

  std::optional<float> resource(String const& resourceName) const;
  std::optional<float> resourceMax(String const& resourceName) const;

  std::optional<float> stat(String const& statName) const;

private:
  Json m_json;
  List<Drawable> m_portrait;
};

class PlayerCompanions {
public:
  PlayerCompanions(Json const& config);

  void diskLoad(Json const& diskStore);
  Json diskStore() const;

  List<CompanionPtr> getCompanions(String const& category) const;

  void init(Entity* player, World* world);
  void uninit();

  void dismissCompanion(String const& category, Uuid const& podUuid);

  std::optional<Json> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});
  void update(float dt);

private:
  LuaCallbacks makeCompanionsCallbacks();

  World* m_world;
  Json m_config;
  StringMap<List<CompanionPtr>> m_companions;

  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>
      m_scriptComponent;
};

}
