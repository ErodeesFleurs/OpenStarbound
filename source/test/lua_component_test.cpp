#include "gtest/gtest.h"

import std;
import star.format;
import star.algorithm;
import star.file;
import star.game_test;
import star.game_types;
import star.json;
import star.input;
import star.input_event;
import star.map;
import star.unicode;
import star.lua;
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_converters;
import star.lua_game_converters;
import star.lua_root;
import star.npc;
import star.npc_database;
import star.root;
import star.rpc_promise;
import star.string;
import star.vector;
import star.world_server;
import star.world_lua_bindings;

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
  ASSERT_TRUE(std::holds_alternative<Json>(**response));
  EXPECT_EQ(std::get<Json>(**response), expected);
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
void expectStorageLifecycle(Root& root, WorldServer& world) {
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
    expectStorageLifecycle<LuaStoredEntityScript>(fixture.root(), *world);
  }
  {
    SCOPED_TRACE("LuaActorScript");
    expectStorageLifecycle<LuaActorScript>(fixture.root(), *world);
  }
  {
    SCOPED_TRACE("LuaActionScript");
    expectStorageLifecycle<LuaActionScript>(fixture.root(), *world);
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
  for (std::string_view name : {"legacy", "localUnnamed", "localNamed", "remoteUnnamed"})
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

const GameTestRegistration inputEventNames("InputNameSerialization", "EventJsonPreservesCanonicalNamesAndUnknownEnumErrors", [](GameTest&) {
  auto key = Input::inputEventToJson(KeyDownEvent{Key::F16, KeyMod::LGui | KeyMod::AltGr});
  ASSERT_TRUE(key) << key.error().what();
  EXPECT_EQ(*key, Json(JsonObject{{"type", "KeyDown"}, {"data", JsonObject{
      {"key", "F16"}, {"mods", JsonArray{"LGui", "AltGr"}}}}}));
  auto mouse = Input::inputEventToJson(MouseButtonUpEvent{MouseButton::FourthButton, Vec2F(3, 4)});
  ASSERT_TRUE(mouse) << mouse.error().what();
  EXPECT_EQ(*mouse, Json(JsonObject{{"type", "MouseButtonUp"}, {"data", JsonObject{
      {"mouseButton", "MouseFourth"}, {"mousePosition", JsonArray{3.0, 4.0}}}}}));
  auto controller = Input::inputEventToJson(ControllerButtonDownEvent{7, ControllerButton::Invalid});
  ASSERT_TRUE(controller) << controller.error().what();
  EXPECT_EQ(*controller, Json(JsonObject{{"type", "ControllerButtonDown"}, {"data", JsonObject{
      {"controllerButton", "Invalid"}, {"controller", 7}}}}));
  auto axis = Input::inputEventToJson(ControllerAxisEvent{2, ControllerAxis::TriggerRight, 0.5f});
  ASSERT_TRUE(axis) << axis.error().what();
  EXPECT_EQ(*axis, Json(JsonObject{{"type", "ControllerAxis"}, {"data", JsonObject{
      {"controllerAxis", "TriggerRight"}, {"controllerAxisValue", 0.5}, {"controller", 2}}}}));
  auto unknownKey = Input::inputEventToJson(KeyUpEvent{static_cast<Key>(65535)});
  ASSERT_FALSE(unknownKey);
  EXPECT_TRUE(unknownKey.error().is<MapException>());
  auto unknownAxis = Input::inputEventToJson(ControllerAxisEvent{2, ControllerAxis::Invalid, 0.5f});
  ASSERT_FALSE(unknownAxis);
  EXPECT_TRUE(unknownAxis.error().is<MapException>());
});

const GameTestRegistration inputBindNames("InputNameSerialization", "BindJsonPreservesCaseFirstErrorsAndUnknownNames", [](GameTest&) {
  auto key = Input::bindFromJson(JsonObject{{"type", "key"}, {"value", "eSc"},
      {"mods", JsonArray{"rShIfT", "lCtRl", "RSHIFT"}}});
  ASSERT_TRUE(key) << key.error().what();
  ASSERT_TRUE(std::holds_alternative<Input::KeyBind>(*key));
  auto const& keyBind = std::get<Input::KeyBind>(*key);
  EXPECT_EQ(keyBind.key, Key::Escape);
  EXPECT_EQ(keyBind.mods, KeyMod::RShift | KeyMod::LCtrl);
  EXPECT_EQ(keyBind.priority, 2);
  auto serializedKey = Input::bindToJson(*key);
  ASSERT_TRUE(serializedKey) << serializedKey.error().what();
  EXPECT_EQ(*serializedKey, Json(JsonObject{{"type", "key"}, {"value", "Esc"},
      {"mods", JsonArray{"RShift", "LCtrl"}}}));
  auto reparsedKey = Input::bindFromJson(*serializedKey);
  ASSERT_TRUE(reparsedKey) << reparsedKey.error().what();
  ASSERT_TRUE(std::holds_alternative<Input::KeyBind>(*reparsedKey));
  auto const& reparsedKeyBind = std::get<Input::KeyBind>(*reparsedKey);
  EXPECT_EQ(reparsedKeyBind.key, keyBind.key);
  EXPECT_EQ(reparsedKeyBind.mods, keyBind.mods);
  EXPECT_EQ(reparsedKeyBind.priority, keyBind.priority);

  for (Json const& json : JsonArray{
      JsonObject{{"type", "mouse"}, {"value", "mOuSeFiFtH"}},
      JsonObject{{"type", "controller"}, {"value", "tOuChPaD"}, {"controller", 7}}}) {
    auto bind = Input::bindFromJson(json);
    ASSERT_TRUE(bind) << bind.error().what();
    auto serialized = Input::bindToJson(*bind);
    ASSERT_TRUE(serialized) << serialized.error().what();
    auto type = json.getString("type");
    ASSERT_TRUE(type);
    EXPECT_EQ(*serialized, *type == "mouse"
        ? Json(JsonObject{{"type", "mouse"}, {"value", "MouseFifth"}})
        : Json(JsonObject{{"type", "controller"}, {"value", "Touchpad"}, {"controller", 7}}));
  }
  String malformed(std::string_view("zzzz\xff", 5));
  auto unknown = Input::bindFromJson(JsonObject{{"type", "key"}, {"value", "not a key"},
      {"mods", JsonArray{malformed}}});
  ASSERT_TRUE(unknown) << unknown.error().what();
  EXPECT_TRUE(std::holds_alternative<std::monostate>(*unknown));
  auto invalidName = Input::bindFromJson(JsonObject{{"type", "key"}, {"value", malformed}});
  ASSERT_FALSE(invalidName);
  EXPECT_TRUE(invalidName.error().is<UnicodeException>());
  auto firstUnknown = Input::bindFromJson(JsonObject{{"type", "key"}, {"value", "A"},
      {"mods", JsonArray{"unknown modifier", malformed}}});
  ASSERT_FALSE(firstUnknown);
  EXPECT_TRUE(firstUnknown.error().is<MapException>());
  auto firstInvalid = Input::bindFromJson(JsonObject{{"type", "key"}, {"value", "A"},
      {"mods", JsonArray{malformed, "unknown modifier"}}});
  ASSERT_FALSE(firstInvalid);
  EXPECT_TRUE(firstInvalid.error().is<UnicodeException>());
  auto invalidValue = Input::bindToJson(Input::KeyBind{static_cast<Key>(65535)});
  ASSERT_FALSE(invalidValue);
  EXPECT_TRUE(invalidValue.error().is<MapException>());
});

const GameTestRegistration inputModifierPresence("InputNameSerialization",
                                                 "ModifierPresencePreservesTypeErrorsAndPriority", [](GameTest&) {
                                                   Json plain = JsonObject{{"type", "key"}, {"value", "A"}};
                                                   auto nullModifiers = plain.set("mods", Json());
                                                   ASSERT_TRUE(nullModifiers);
                                                   for (Json const& config : JsonArray{plain, *nullModifiers}) {
                                                     auto decoded = Input::bindFromJson(config);
                                                     ASSERT_TRUE(decoded);
                                                     ASSERT_TRUE(std::holds_alternative<Input::KeyBind>(*decoded));
                                                     auto const& binding = std::get<Input::KeyBind>(*decoded);
                                                     EXPECT_EQ(binding.mods, KeyMod::NoMod);
                                                     EXPECT_EQ(binding.priority, 0);
                                                   }
                                                   auto badModifiers = plain.set("mods", JsonObject{});
                                                   ASSERT_TRUE(badModifiers);
                                                   auto invalid = Input::bindFromJson(*badModifiers);
                                                   ASSERT_FALSE(invalid);
                                                   EXPECT_TRUE(invalid.error().is<JsonException>());
                                                 });
const GameTestRegistration luaEntityIdentifiers(
  "LuaEntityBindings", "EmbeddedNullIdentifiersSurviveFreshAndRestartedEngines", [](GameTest& fixture) {
    auto storage = File::ephemeralFile(fixture.root().random());
    ASSERT_TRUE(storage);
    auto world = WorldServer::create(fixture.root(), Vec2U(2048, 2048), *storage);
    ASSERT_TRUE(world);
    auto close = finally([&] { EXPECT_TRUE((*world)->close()); });
    auto database = fixture.root().npcDatabase();
    ASSERT_TRUE(database);
    auto variant = (*database)->generateNpcVariant("human", "villager", 1.0f, 1234, JsonObject());
    ASSERT_TRUE(variant);
    auto npc = (*database)->createNpc(*variant);
    ASSERT_TRUE(npc);
    ASSERT_TRUE((*world)->addEntity(*npc));
    String identifier(std::string_view("\0before\0\0after\0", 15));
    (*npc)->setUniqueId(identifier);
    auto luaRoot = LuaRoot::create(fixture.root());
    ASSERT_TRUE(luaRoot);
    for (bool restart : {false, true}) {
      SCOPED_TRACE(restart ? "restarted engine" : "fresh engine");
      if (restart)
        ASSERT_TRUE((*luaRoot)->restart());
      auto context = (*luaRoot)->luaEngine().createContext();
      ASSERT_TRUE(context);
      auto callbacks = LuaBindings::makeWorldCallbacks(fixture.root(), **world);
      ASSERT_TRUE(callbacks);
      ASSERT_TRUE(context->setCallbacks("world", *callbacks));
      ASSERT_TRUE(context->set("id", (*npc)->entityId()));
      auto identifiers = context->eval<Json>("{world.entityUniqueId(id), world.entity(id):uniqueId()}");
      ASSERT_TRUE(identifiers) << identifiers.error().what();
      EXPECT_EQ(*identifiers, Json(JsonArray{identifier, identifier}));
      auto lengths = context->eval<Json>("{#world.entityUniqueId(id), #world.entity(id):uniqueId()}");
      ASSERT_TRUE(lengths);
      EXPECT_EQ(*lengths, Json(JsonArray{15, 15}));
    }
  });
}
