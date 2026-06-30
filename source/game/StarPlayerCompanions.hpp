#pragma once

#include "StarAiTypes.hpp"
#include "StarUuid.hpp"
#include "StarDrawable.hpp"
#include "StarLuaComponents.hpp"
#include "StarWorld.hpp"

namespace Star {

class Companion;
using CompanionPtr = SharedPtr<Companion>;
class PlayerCompanions;
using PlayerCompanionsPtr = SharedPtr<PlayerCompanions>;
class Player;

class Companion {
public:
  Companion(Json const& json);
  [[nodiscard]] Json toJson() const;

  [[nodiscard]] Uuid podUuid() const;
  [[nodiscard]] Maybe<String> name() const;
  [[nodiscard]] Maybe<String> description() const;

  [[nodiscard]] List<Drawable> portrait() const;

  [[nodiscard]] Maybe<float> resource(String const& resourceName) const;
  [[nodiscard]] Maybe<float> resourceMax(String const& resourceName) const;

  [[nodiscard]] Maybe<float> stat(String const& statName) const;

private:
  Json m_json;
  List<Drawable> m_portrait;
};

class PlayerCompanions {
public:
  PlayerCompanions(Json const& config);

  void diskLoad(Json const& diskStore);
  [[nodiscard]] Json diskStore() const;

  [[nodiscard]] List<CompanionPtr> getCompanions(String const& category) const;

  void init(Player& player, World& world);
  void uninit();

  void dismissCompanion(String const& category, Uuid const& podUuid);

  [[nodiscard]] Maybe<Json> receiveMessage(String const& message, bool localMessage, JsonArray const& args = {});
  void update(float dt);

private:
  [[nodiscard]] LuaCallbacks makeCompanionsCallbacks();

  World* m_world;
  Json m_config;
  StringMap<List<CompanionPtr>> m_companions;

  LuaMessageHandlingComponent<LuaStorableComponent<LuaUpdatableComponent<LuaWorldComponent<LuaBaseComponent>>>>
      m_scriptComponent;
};

}
