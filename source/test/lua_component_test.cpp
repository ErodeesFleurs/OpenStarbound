#include "gtest/gtest.h"

import std;
import star.format;
import star.algorithm;
import star.file;
import star.game_test;
import star.game_types;
import star.json;
import star.lua;
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_converters;
import star.lua_game_converters;
import star.lua_root;
import star.root;
import star.rpc_promise;
import star.string;
import star.vector;
import star.world_server;

using namespace Star;

namespace {

static_assert(!std::is_copy_constructible_v<LuaMessageScript>);
static_assert(!std::is_move_constructible_v<LuaMessageScript>);
static_assert(!std::is_move_constructible_v<LuaStoredEntityScript>);
static_assert(!std::is_move_constructible_v<LuaActorScript>);
static_assert(!std::is_move_constructible_v<LuaActionScript>);

void expectNoResponse(std::expected<std::optional<ChainableJsonMessageResponse>, StarException> response) {
  ASSERT_TRUE(response) << response.error().what();
  EXPECT_FALSE(*response);
}

void expectResponse(std::expected<std::optional<ChainableJsonMessageResponse>, StarException> response, Json const& expected) {
  ASSERT_TRUE(response) << response.error().what();
  ASSERT_TRUE(*response);
  auto value = std::get_if<Json>(&**response);
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(*value, expected);
}

void expectPending(RpcPromise<Json> const& promise) {
  auto finished = promise.finished();
  ASSERT_TRUE(finished) << finished.error().what();
  EXPECT_FALSE(*finished);
}

void expectPromiseFailure(RpcPromise<Json> const& promise) {
  auto finished = promise.finished();
  ASSERT_TRUE(finished) << finished.error().what();
  EXPECT_TRUE(*finished);
  auto error = promise.error();
  ASSERT_TRUE(error) << error.error().what();
  ASSERT_TRUE(*error);
}

template <typename Host>
void expectStorageLifecycle(Root& root, WorldServer* world) {
  Host component(root);
  auto cleanup = finally([&] { EXPECT_TRUE(component.uninit()); });
  JsonObject initial{{"total", 7}, {"saves", 0}};
  ASSERT_TRUE(component.setScriptStorage(initial));
  auto beforeInit = component.getScriptStorage();
  ASSERT_TRUE(beforeInit) << beforeInit.error().what();
  EXPECT_EQ(*beforeInit, initial);

  // Empty scripts still run the real host's context setup. In particular,
  // storage and self must exist before any script code accesses them.
  ASSERT_TRUE(component.init(world));
  ASSERT_TRUE(component.initialized());
  auto missing = component.template invoke<Json>("missingEntryPoint");
  ASSERT_TRUE(missing) << missing.error().what();
  EXPECT_FALSE(*missing);
  auto missingUpdate = component.template update<Json>(0.25f, 5);
  ASSERT_TRUE(missingUpdate) << missingUpdate.error().what();
  EXPECT_FALSE(*missingUpdate);

  auto defined = component.template eval<bool>(R"LUA(
    self.marker = "old context"
    function update(dt, amount)
      storage.total = storage.total + amount
      storage.lastDt = dt
      return {storage.total, dt}
    end
    function uninit()
      storage.saves = storage.saves + 1
    end
    return storage.total == 7 and storage.saves == 0
  )LUA");
  ASSERT_TRUE(defined) << defined.error().what();
  ASSERT_TRUE(*defined);
  EXPECT_TRUE(**defined);

  auto updated = component.template update<Json>(0.25f, 5);
  ASSERT_TRUE(updated) << updated.error().what();
  ASSERT_TRUE(*updated);
  EXPECT_EQ(**updated, Json(JsonArray{12, 0.25}));
  JsonObject live{{"total", 12}, {"saves", 0}, {"lastDt", 0.25}};
  auto liveStorage = component.getScriptStorage();
  ASSERT_TRUE(liveStorage) << liveStorage.error().what();
  EXPECT_EQ(*liveStorage, live);

  // uninit must execute before the storage snapshot and context teardown.
  ASSERT_TRUE(component.uninit());
  EXPECT_FALSE(component.initialized());
  JsonObject saved{{"total", 12}, {"saves", 1}, {"lastDt", 0.25}};
  auto stored = component.getScriptStorage();
  ASSERT_TRUE(stored) << stored.error().what();
  EXPECT_EQ(*stored, saved);
  auto stoppedUpdate = component.template update<Json>(0.25f, 5);
  ASSERT_TRUE(stoppedUpdate) << stoppedUpdate.error().what();
  EXPECT_FALSE(*stoppedUpdate);

  ASSERT_TRUE(component.init(world));
  auto restored = component.template eval<bool>(R"LUA(
    return storage.total == 12 and storage.saves == 1 and storage.lastDt == 0.25
      and self.marker == nil and update == nil and uninit == nil
  )LUA");
  ASSERT_TRUE(restored) << restored.error().what();
  ASSERT_TRUE(*restored);
  EXPECT_TRUE(**restored);
  ASSERT_TRUE(component.uninit());
  auto unchanged = component.getScriptStorage();
  ASSERT_TRUE(unchanged) << unchanged.error().what();
  EXPECT_EQ(*unchanged, saved);
}

const GameTestRegistration persistentStorage("LuaComponentLifecycle", "PersistentHostsPreserveStorageAcrossContexts", [](GameTest& fixture) {
  auto storage = File::ephemeralFile(fixture.root().random());
  ASSERT_TRUE(storage) << storage.error().what();
  auto created = WorldServer::create(fixture.root(), Vec2U(2048, 2048), *storage);
  ASSERT_TRUE(created) << created.error().what();
  auto world = *created;
  auto close = finally([&] { EXPECT_TRUE(world->close()); });
  {
    SCOPED_TRACE("LuaStoredEntityScript");
    expectStorageLifecycle<LuaStoredEntityScript>(fixture.root(), world.get());
  }
  {
    SCOPED_TRACE("LuaActorScript");
    expectStorageLifecycle<LuaActorScript>(fixture.root(), world.get());
  }
  {
    SCOPED_TRACE("LuaActionScript");
    expectStorageLifecycle<LuaActionScript>(fixture.root(), world.get());
  }
});

const GameTestRegistration messageLifecycle("LuaComponentLifecycle", "MessageArgumentsAndHandlersFollowContextLifetime", [](GameTest& fixture) {
  auto luaRoot = LuaRoot::create(fixture.root());
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  unsigned calls = 0;
  LuaMessageScript component(fixture.root());
  auto cleanup = finally([&] { EXPECT_TRUE(component.uninit()); });
  ASSERT_TRUE(component.setLuaRoot(*luaRoot));
  LuaCallbacks observer;
  ASSERT_TRUE(observer.registerCallback("record", [&] { ++calls; }));
  ASSERT_TRUE(component.addCallbacks("observer", std::move(observer)));
  auto initialized = component.init();
  ASSERT_TRUE(initialized) << initialized.error().what();
  ASSERT_TRUE(*initialized);
  auto defined = component.eval<bool>(R"LUA(
    message.setHandler("legacy", function(name, localMessage, value, tail)
      observer.record()
      return {name, localMessage, value, tail}
    end)
    message.setHandler({name = "localUnnamed", localOnly = true}, function(value)
      observer.record()
      return value * 2
    end)
    message.setHandler({name = "localNamed", localOnly = true, passName = true}, function(name, value)
      observer.record()
      return {name, value}
    end)
    message.setHandler({name = "remoteUnnamed"}, function(localMessage, value)
      observer.record()
      return {localMessage, value}
    end)
    return true
  )LUA");
  ASSERT_TRUE(defined) << defined.error().what();
  ASSERT_TRUE(*defined);
  EXPECT_TRUE(**defined);

  expectNoResponse(component.handleMessage("localUnnamed", false, {7}));
  EXPECT_EQ(calls, 0u);
  expectResponse(component.handleMessage("localUnnamed", true, {7}), Json(14));
  expectResponse(component.handleMessage("localNamed", true, {9}), Json(JsonArray{"localNamed", 9}));
  expectResponse(component.handleMessage("remoteUnnamed", false, {11}), Json(JsonArray{false, 11}));
  expectResponse(component.handleMessage("legacy", true, {13, "tail"}), Json(JsonArray{"legacy", true, 13, "tail"}));
  EXPECT_EQ(calls, 4u);

  ASSERT_TRUE(component.uninit());
  expectNoResponse(component.handleMessage("legacy", true, {13, "tail"}));
  EXPECT_EQ(calls, 4u);
  auto reinitialized = component.init();
  ASSERT_TRUE(reinitialized) << reinitialized.error().what();
  ASSERT_TRUE(*reinitialized);
  // Checking after reinit catches retained LuaFunction handlers; checking only
  // while uninitialized would merely exercise the initialized() guard.
  for (auto name : {"legacy", "localUnnamed", "localNamed", "remoteUnnamed"})
    expectNoResponse(component.handleMessage(name, true, {17, "unused"}));
  EXPECT_EQ(calls, 4u);
  auto registered = component.eval<bool>(R"LUA(
    message.setHandler({name = "replacement", localOnly = true}, function(value)
      observer.record()
      return value
    end)
    return true
  )LUA");
  ASSERT_TRUE(registered) << registered.error().what();
  ASSERT_TRUE(*registered);
  EXPECT_TRUE(**registered);
  expectResponse(component.handleMessage("replacement", true, {19}), Json(19));
  EXPECT_EQ(calls, 5u);
});

const GameTestRegistration threadMessages("LuaComponentLifecycle", "ThreadPromisesSettleOnUpdateUninitAndDestruction", [](GameTest& fixture) {
  auto luaRoot = LuaRoot::create(fixture.root());
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  std::optional<RpcPromise<Json>> destroyed;
  {
    // Heap ownership fixes callback targets' addresses through queue teardown.
    auto component = std::make_unique<LuaMessageScript>(fixture.root());
    ASSERT_TRUE(component->setLuaRoot(*luaRoot));
    auto initialized = component->init();
    ASSERT_TRUE(initialized) << initialized.error().what();
    ASSERT_TRUE(*initialized);
    auto defined = component->eval<bool>(R"LUA(
      threads.setMessageHandler("echo", function(origin, name, value)
        return {origin, name, value}
      end)
      return true
    )LUA");
    ASSERT_TRUE(defined) << defined.error().what();
    ASSERT_TRUE(*defined);
    EXPECT_TRUE(**defined);
    auto handled = component->threadPassMessage("worker", "echo", {23});
    auto missing = component->threadPassMessage("worker", "missing", {});
    expectPending(handled);
    expectPending(missing);
    auto updated = component->update<Json>();
    ASSERT_TRUE(updated) << updated.error().what();
    EXPECT_FALSE(*updated);
    auto result = handled.result();
    ASSERT_TRUE(result) << result.error().what();
    ASSERT_TRUE(*result);
    EXPECT_EQ(**result, Json(JsonArray{"worker", "echo", 23}));
    expectPromiseFailure(missing);

    auto pending = component->threadPassMessage("worker", "echo", {29});
    expectPending(pending);
    ASSERT_TRUE(component->uninit());
    expectPromiseFailure(pending);
    auto reinitialized = component->init();
    ASSERT_TRUE(reinitialized) << reinitialized.error().what();
    ASSERT_TRUE(*reinitialized);
    auto oldHandler = component->threadPassMessage("worker", "echo", {31});
    auto afterReinit = component->update<Json>();
    ASSERT_TRUE(afterReinit) << afterReinit.error().what();
    EXPECT_FALSE(*afterReinit);
    expectPromiseFailure(oldHandler);
    destroyed.emplace(component->threadPassMessage("worker", "echo", {37}));
    expectPending(*destroyed);
  }
  ASSERT_TRUE(destroyed);
  expectPromiseFailure(*destroyed);
});

}
