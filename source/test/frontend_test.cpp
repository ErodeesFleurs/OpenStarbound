import std;
import star.chat_bubble_separation;
import star.format;
import star.game_test;
import star.input_event;
import star.json;
import star.key_bindings;
import star.list;
import star.rect;
import star.vector;
import star.set;
import star.string;
import star.root;
import star.configuration;
import star.gui_context;
import star.widget;
import star.button_widget;
import star.button_group;
import star.text_box_widget;
import star.base_script_pane;
import star.lua;
import star.lua_root;
import star.lua_components;
import star.lua_converters;
import star.lua_game_converters;
import star.map;
import star.pane;
import star.pane_manager;
import star.main_interface;

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

using namespace Star;

namespace {

std::optional<RectF> separateAgainst(List<RectF> const& obstacles, RectF box) {
  BubbleSeparator<int> separator(1.0f);
  int id = 0;
  for (auto const& obstacle : obstacles)
    separator.addBubble(obstacle.center(), obstacle, id++);
  auto prepared = separator.forEach([](BubbleState<int>&, int&) -> std::expected<void, StarException> {
    return {};
  });
  EXPECT_TRUE(prepared);
  separator.addBubble(box.center(), box, -1);
  auto matches = separator.filtered([](BubbleState<int> const&, int id) { return id == -1; });
  EXPECT_EQ(matches.size(), 1u);
  std::optional<RectF> result;
  for (auto const& match : matches)
    result = match.separatedBox;
  return result;
}

TEST(ChatBubbleSeparation, ContainingProjectionsNeedBothEdgeQueries) {
  RectF box{0, 2, 10, 8};
  RectF containing{-100, 0, 100, 10};
  // A nonoverlapping neighbor hides the containing rectangle in one ordering.
  EXPECT_EQ(separateAgainst({containing, {-20, 20, -10, 30}}, box), (RectF{0, 10, 10, 16}));
  EXPECT_EQ(separateAgainst({containing, {20, 20, 30, 30}}, box), (RectF{0, 10, 10, 16}));
}

TEST(ChatBubbleSeparation, EqualSizesRespectVerticalPriorityAndTouchingEdges) {
  RectF box{0, 0, 10, 10};
  EXPECT_EQ(separateAgainst({{0, 5, 10, 15}}, box), box);
  EXPECT_EQ(separateAgainst({{0, -5, 10, 5}}, box), (RectF{0, 10, 10, 20}));
  EXPECT_EQ(separateAgainst({box}, box), (RectF{0, 10, 10, 20}));
  EXPECT_EQ(separateAgainst({{10, 0, 20, 10}, {0, 10, 10, 20}}, box), box);
}

TEST(ChatBubbleSeparation, MixedSizesChainWithoutChangingDimensions) {
  RectF box{0, 0, 10, 10};
  auto result = separateAgainst({{-5, 0, 15, 10}, {-2, 12, 13, 22}, {0, 24, 10, 44}}, RectF{0, 4, 10, 14});
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, (RectF{0, 44, 10, 54}));
  EXPECT_EQ(result->size(), box.size());
  EXPECT_EQ(separateAgainst({{1, -5, 9, 50}}, box), box);
}

const GameTestRegistration keybindingSpecificity("KeyBindings", "MostSpecificModifiersAndEventTypes", [](GameTest& fixture) {
  auto config = Json::parseJson(R"JSON({
    "PlayerLeft": [{"type":"key","value":"A","mods":[]}],
    "PlayerRight": [{"type":"key","value":"A","mods":["LShift"]}],
    "PlayerUp": [{"type":"key","value":"A","mods":["LCtrl"]}],
    "PlayerDown": [{"type":"key","value":"A","mods":["LShift","LCtrl"]}],
    "PlayerJump": [{"type":"key","value":"A","mods":["LShift","LCtrl"]}]
  })JSON");
  ASSERT_TRUE(config) << config.error().what();
  auto bindings = KeyBindings::create(fixture.root(), *config);
  ASSERT_TRUE(bindings) << bindings.error().what();
  EXPECT_EQ(bindings->actions(Key::A), (Set<InterfaceAction>{InterfaceAction::PlayerLeft}));
  EXPECT_EQ(bindings->actions(KeyChord{Key::A, KeyMod::LShift}), (Set<InterfaceAction>{InterfaceAction::PlayerRight}));
  EXPECT_EQ(bindings->actions(KeyChord{Key::A, KeyMod::LCtrl}), (Set<InterfaceAction>{InterfaceAction::PlayerUp}));
  auto both = KeyMod::LShift | KeyMod::LCtrl;
  EXPECT_EQ(bindings->actions(KeyChord{Key::A, both}),
      (Set<InterfaceAction>{InterfaceAction::PlayerDown, InterfaceAction::PlayerJump}));
  EXPECT_EQ(bindings->actions(InputEvent{KeyDownEvent{Key::A, both | KeyMod::LAlt}}),
      (Set<InterfaceAction>{InterfaceAction::PlayerDown, InterfaceAction::PlayerJump}));
  EXPECT_TRUE(bindings->actions(InputEvent{KeyUpEvent{Key::A}}).empty());
  EXPECT_TRUE(bindings->actions(Key::B).empty());
  EXPECT_TRUE(bindings->actionsForKey(Key::B).empty());
  EXPECT_EQ(bindings->actionsForKey(Key::A), (Set<InterfaceAction>{InterfaceAction::PlayerLeft,
      InterfaceAction::PlayerRight, InterfaceAction::PlayerUp, InterfaceAction::PlayerDown, InterfaceAction::PlayerJump}));

  auto tiedConfig = Json::parseJson(R"JSON({
    "PlayerRight": [{"type":"key","value":"A","mods":["LShift"]}],
    "PlayerUp": [{"type":"key","value":"A","mods":["LCtrl"]}]
  })JSON");
  ASSERT_TRUE(tiedConfig) << tiedConfig.error().what();
  auto tied = KeyBindings::create(fixture.root(), *tiedConfig);
  ASSERT_TRUE(tied) << tied.error().what();
  EXPECT_EQ(tied->actions(KeyChord{Key::A, both}),
      (Set<InterfaceAction>{InterfaceAction::PlayerRight, InterfaceAction::PlayerUp}));
});

const GameTestRegistration widgetParentLifetime("WidgetBorrow", "ReparentAndDestructionDetachEscapedChild", [](GameTest& fixture) {
  auto configuration = fixture.root().configuration();
  ASSERT_TRUE(configuration);
  ASSERT_TRUE((*configuration)->set("bindings", JsonObject{}));
  auto context = GuiContext::create(fixture.root(), {}, {});
  ASSERT_TRUE(context) << context.error().what();

  auto first = std::make_shared<Widget>(**context);
  auto second = std::make_shared<Widget>(**context);
  auto child = std::make_shared<Widget>(**context);
  ASSERT_TRUE(first->addChild("old", child));
  ASSERT_TRUE(second->addChild("new", child));
  auto oldName = first->containsChild("old");
  ASSERT_TRUE(oldName);
  EXPECT_FALSE(*oldName);
  EXPECT_EQ(second->childPtr(*child), child);
  ASSERT_TRUE(child->parent());
  EXPECT_EQ(child->parent()->get().childPtr(*child), child);

  first.reset();
  EXPECT_TRUE(child->parent());
  second.reset();
  EXPECT_FALSE(child->parent());
  EXPECT_FALSE(child->window());
});

const GameTestRegistration buttonGroupLifetime("WidgetBorrow", "ButtonDestructionRemovesWeakGroupEntries", [](GameTest& fixture) {
  auto configuration = fixture.root().configuration();
  ASSERT_TRUE(configuration);
  ASSERT_TRUE((*configuration)->set("bindings", JsonObject{}));
  auto context = GuiContext::create(fixture.root(), {}, {});
  ASSERT_TRUE(context) << context.error().what();
  auto first = ButtonWidget::create(**context);
  auto second = ButtonWidget::create(**context);
  ASSERT_TRUE(first) << first.error().what();
  ASSERT_TRUE(second) << second.error().what();
  auto group = std::make_shared<ButtonGroup>();
  (*first)->setButtonGroup(group, 10);
  (*second)->setButtonGroup(group, 20);
  ASSERT_TRUE(group->select(10));
  EXPECT_TRUE((*first)->isChecked());
  EXPECT_FALSE((*second)->isChecked());

  first->reset();
  EXPECT_FALSE(group->button(10));
  EXPECT_EQ(1u, group->buttonCount());
  EXPECT_EQ(group->checkedId(), ButtonGroup::NoButton);
  ASSERT_TRUE(group->select(20));
  EXPECT_TRUE((*second)->isChecked());
  second->reset();
  EXPECT_TRUE(group->buttons().empty());
  EXPECT_EQ(0u, group->buttonCount());
  EXPECT_EQ(group->checkedId(), ButtonGroup::NoButton);
});

std::expected<std::shared_ptr<GuiContext>, StarException> textEditorContext(GameTest& fixture) {
  auto configuration = fixture.root().configuration();
  if (!configuration)
    return std::unexpected(std::move(configuration.error()));
  auto configured = (*configuration)->set("bindings", JsonObject{});
  if (!configured)
    return std::unexpected(std::move(configured.error()));
  return GuiContext::create(fixture.root(), {}, {});
}

std::expected<std::shared_ptr<TextBoxWidget>, StarException> textEditor(GuiContext& context, String const& text) {
  return TextBoxWidget::create(context, text, "", [](Widget&) -> std::expected<void, StarException> {
    return {};
  });
}

void sendEditorEvent(TextBoxWidget& editor, InputEvent const& event) {
  auto handled = editor.sendEvent(event);
  ASSERT_TRUE(handled) << handled.error().what();
  EXPECT_TRUE(*handled);
}

const GameTestRegistration textEditorUnicode("TextEditor", "CommittedUnicodeReplacesSelectionWithoutDrawing", [](GameTest& fixture) {
  auto context = textEditorContext(fixture);
  ASSERT_TRUE(context) << context.error().what();
  auto created = textEditor(**context, "A中🙂Z");
  ASSERT_TRUE(created) << created.error().what();
  auto& editor = **created;
  editor.setEditorRendering(false);
  ASSERT_TRUE(editor.focus());
  auto capture = editor.keyboardCaptureMode();
  ASSERT_TRUE(capture) << capture.error().what();
  EXPECT_EQ(*capture, KeyboardCaptureMode::TextInput);
  EXPECT_EQ(editor.getCursorPosition(), 4);

  // Reversed endpoints are codepoint positions, not UTF-8 byte offsets.
  ASSERT_TRUE(editor.setSelection(3, 1));
  sendEditorEvent(editor, TextInputEvent{"é界"});
  EXPECT_EQ(editor.getText(), String("Aé界Z"));
  EXPECT_EQ(editor.getCursorPosition(), 3);
  // A second commit inserts rather than replacing the former selection.
  sendEditorEvent(editor, TextInputEvent{"🙂"});
  EXPECT_EQ(editor.getText(), String("Aé界🙂Z"));
  EXPECT_EQ(editor.getCursorPosition(), 4);
});

const GameTestRegistration textEditorSelectionDelete("TextEditor", "RealShiftNavigationAndBothDeletionKeys", [](GameTest& fixture) {
  auto context = textEditorContext(fixture);
  ASSERT_TRUE(context) << context.error().what();
  auto created = textEditor(**context, "A中🙂Z");
  ASSERT_TRUE(created) << created.error().what();
  auto& editor = **created;
  ASSERT_TRUE(editor.focus());
  sendEditorEvent(editor, KeyDownEvent{Key::Home, KeyMod::NoMod});
  sendEditorEvent(editor, KeyDownEvent{Key::Right, KeyMod::NoMod});
  sendEditorEvent(editor, KeyDownEvent{Key::Right, KeyMod::LShift});
  sendEditorEvent(editor, KeyDownEvent{Key::Right, KeyMod::LShift});
  sendEditorEvent(editor, KeyDownEvent{Key::Delete, KeyMod::NoMod});
  EXPECT_EQ(editor.getText(), String("AZ"));
  EXPECT_EQ(editor.getCursorPosition(), 1);
  sendEditorEvent(editor, TextInputEvent{"界"});
  EXPECT_EQ(editor.getText(), String("A界Z"));

  auto reset = editor.setText("A中🙂Z");
  ASSERT_TRUE(reset) << reset.error().what();
  ASSERT_TRUE(*reset);
  ASSERT_TRUE(editor.setSelection(3, 1));
  sendEditorEvent(editor, KeyDownEvent{Key::Backspace, KeyMod::NoMod});
  EXPECT_EQ(editor.getText(), String("AZ"));
  EXPECT_EQ(editor.getCursorPosition(), 1);
  sendEditorEvent(editor, TextInputEvent{"🙂"});
  EXPECT_EQ(editor.getText(), String("A🙂Z"));
});

const GameTestRegistration textEditorRejectedEdit("TextEditor", "RejectedInputAndDeletionRetainSelection", [](GameTest& fixture) {
  auto context = textEditorContext(fixture);
  ASSERT_TRUE(context) << context.error().what();
  auto created = textEditor(**context, "A中🙂Z");
  ASSERT_TRUE(created) << created.error().what();
  auto& editor = **created;
  ASSERT_TRUE(editor.focus());
  editor.setRegex("[A-Z中🙂]*");
  ASSERT_TRUE(editor.setSelection(3, 1));
  sendEditorEvent(editor, TextInputEvent{"1"});
  EXPECT_EQ(editor.getText(), String("A中🙂Z"));
  EXPECT_EQ(editor.getCursorPosition(), 1);
  // This succeeds only if the rejected commit retained both selection endpoints.
  sendEditorEvent(editor, TextInputEvent{"Q"});
  EXPECT_EQ(editor.getText(), String("AQZ"));
  EXPECT_EQ(editor.getCursorPosition(), 2);

  editor.setRegex("[A-Z中🙂]{3}");
  ASSERT_TRUE(editor.setSelection(1, 2));
  sendEditorEvent(editor, KeyDownEvent{Key::Delete, KeyMod::NoMod});
  EXPECT_EQ(editor.getText(), String("AQZ"));
  EXPECT_EQ(editor.getCursorPosition(), 2);
  // A rejected deletion must not turn this replacement into an insertion.
  sendEditorEvent(editor, TextInputEvent{"中"});
  EXPECT_EQ(editor.getText(), String("A中Z"));
  EXPECT_EQ(editor.getCursorPosition(), 2);

  // A rejected whole-field removal must not create a phantom selection.
  // The renderer-free fixture has no platform clipboard controller.
  editor.setRegex("[A-Z中🙂]{3,4}");
  ASSERT_TRUE(editor.setCursorPosition(1));
  auto rejected = editor.setText("");
  ASSERT_TRUE(rejected) << rejected.error().what();
  EXPECT_FALSE(*rejected);
  EXPECT_EQ(editor.getText(), String("A中Z"));
  EXPECT_EQ(editor.getCursorPosition(), 1);
  sendEditorEvent(editor, TextInputEvent{"Q"});
  EXPECT_EQ(editor.getText(), String("AQ中Z"));
  EXPECT_EQ(editor.getCursorPosition(), 2);
});

const GameTestRegistration textEditorLegacy("TextEditor", "UnselectedEditingFocusAndSubmitRemainUnchanged", [](GameTest& fixture) {
  auto context = textEditorContext(fixture);
  ASSERT_TRUE(context) << context.error().what();
  auto created = textEditor(**context, "ab");
  ASSERT_TRUE(created) << created.error().what();
  auto& editor = **created;
  auto unfocused = editor.sendEvent(TextInputEvent{"ignored"});
  ASSERT_TRUE(unfocused) << unfocused.error().what();
  EXPECT_FALSE(*unfocused);
  EXPECT_EQ(editor.getText(), String("ab"));
  ASSERT_TRUE(editor.focus());

  sendEditorEvent(editor, TextInputEvent{"中"});
  sendEditorEvent(editor, KeyDownEvent{Key::Left, KeyMod::NoMod});
  sendEditorEvent(editor, KeyDownEvent{Key::Backspace, KeyMod::NoMod});
  EXPECT_EQ(editor.getText(), String("a中"));
  EXPECT_EQ(editor.getCursorPosition(), 1);
  sendEditorEvent(editor, KeyDownEvent{Key::Delete, KeyMod::NoMod});
  EXPECT_EQ(editor.getText(), String("a"));
  EXPECT_EQ(editor.getCursorPosition(), 1);
  sendEditorEvent(editor, KeyDownEvent{Key::Home, KeyMod::NoMod});
  sendEditorEvent(editor, TextInputEvent{"b"});
  EXPECT_EQ(editor.getText(), String("ba"));
  sendEditorEvent(editor, KeyDownEvent{Key::End, KeyMod::NoMod});
  EXPECT_EQ(editor.getCursorPosition(), 2);

  int submitted = 0;
  editor.setOnEnterKeyCallback([&](Widget&) -> std::expected<void, StarException> {
    ++submitted;
    return {};
  });
  sendEditorEvent(editor, KeyDownEvent{Key::Return, KeyMod::NoMod});
  EXPECT_EQ(submitted, 1);
  EXPECT_EQ(editor.getText(), String("ba"));
  sendEditorEvent(editor, KeyDownEvent{Key::Return, KeyMod::LShift});
  EXPECT_EQ(submitted, 1);
  EXPECT_EQ(editor.getText(), String("ba\n"));
  EXPECT_EQ(editor.getCursorPosition(), 3);
});

class ScriptTestPane : public BaseScriptPane {
public:
  static std::expected<std::shared_ptr<ScriptTestPane>, StarException> create(GuiContext& context) {
    auto pane = std::shared_ptr<ScriptTestPane>(new ScriptTestPane(context));
    auto configured = pane->init(JsonObject{{"gui", JsonObject{}}});
    if (!configured)
      return std::unexpected(std::move(configured.error()));
    return pane;
  }
  std::expected<std::optional<Json>, StarException> evaluate(String const& code) {
    return m_script.eval<Json>(code);
  }
private:
  explicit ScriptTestPane(GuiContext& context) : BaseScriptPane(context) {}
};

const GameTestRegistration scriptStorage("ScriptPane", "StorageSurvivesLifecycleReloadAndEngineReplacement", [](GameTest& fixture) {
  auto context = textEditorContext(fixture);
  ASSERT_TRUE(context) << context.error().what();
  auto luaRoot = LuaRoot::create(fixture.root());
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  auto created = ScriptTestPane::create(**context);
  ASSERT_TRUE(created) << created.error().what();
  auto& pane = **created;
  ASSERT_TRUE(pane.setLuaRoot(*luaRoot));
  ASSERT_TRUE(pane.displayed());
  ASSERT_TRUE(pane.evaluate("storage.count = 17; storage.events = jarray(); table.insert(storage.events, false)"));
  ASSERT_TRUE(pane.dismissed());
  ASSERT_TRUE(pane.displayed());
  auto saved = pane.evaluate("return {storage.count, #storage.events, storage.events[1]}");
  ASSERT_TRUE(saved) << saved.error().what();
  ASSERT_TRUE(*saved);
  EXPECT_EQ(**saved, (Json(JsonArray{17, 1, false})));

  ASSERT_TRUE(fixture.root().reload());
  auto reloaded = pane.evaluate("return {storage.count, #storage.events, storage.events[1]}");
  ASSERT_TRUE(reloaded) << reloaded.error().what();
  ASSERT_TRUE(*reloaded);
  EXPECT_EQ(**reloaded, **saved);
  auto previousGeneration = (*luaRoot)->generation();
  ASSERT_TRUE((*luaRoot)->restart());
  EXPECT_GT((*luaRoot)->generation(), previousGeneration);
  auto restarted = pane.evaluate("return {storage.count, #storage.events, storage.events[1]}");
  ASSERT_TRUE(restarted) << restarted.error().what();
  ASSERT_TRUE(*restarted);
  EXPECT_EQ(**restarted, **saved);
  ASSERT_TRUE(pane.dismissed());
  auto stopped = pane.call("missing", {});
  ASSERT_TRUE(stopped) << stopped.error().what();
  EXPECT_FALSE(*stopped);
});

const GameTestRegistration componentCallbackOwnership("ScriptPane", "ComponentCallbacksReplaceInheritedTargetsWithoutRevivingRetiredOnes", [](GameTest& fixture) {
  auto luaRoot = LuaRoot::create(fixture.root());
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  std::stop_source previousLifetime;
  LuaCallbacks previous;
  ASSERT_TRUE(previous.registerCallback("session",
      [lifetime = previousLifetime.get_token()]() -> std::expected<String, StarException> {
    if (lifetime.stop_requested())
      return std::unexpected(StarException("Previous interface is detached"));
    return String("previous");
  }));
  (*luaRoot)->addCallbacks("interface", previous);
  (*luaRoot)->addCallbacks("world", previous);
  (*luaRoot)->addCallbacks("inheritedOnly", previous);

  LuaBaseComponent first(fixture.root());
  ASSERT_TRUE(first.addCallbacks("interface", previous));
  ASSERT_TRUE(first.setLuaRoot(*luaRoot));
  ASSERT_TRUE(first.init());
  auto firstSession = first.eval<String>("return interface.session()");
  ASSERT_TRUE(firstSession) << firstSession.error().what();
  ASSERT_TRUE(*firstSession);
  EXPECT_EQ(**firstSession, String("previous"));
  auto retainedSession = first.context()->getPath<LuaFunction>("interface.session");
  ASSERT_TRUE(retainedSession) << retainedSession.error().what();
  ASSERT_TRUE(first.uninit());
  ASSERT_TRUE(previousLifetime.request_stop());
  auto retired = retainedSession->invoke<String>();
  ASSERT_FALSE(retired);
  EXPECT_NE(std::string_view(retired.error().what()).find("Previous interface is detached"), std::string_view::npos);

  std::stop_source currentLifetime;
  LuaCallbacks current;
  ASSERT_TRUE(current.registerCallback("session",
      [lifetime = currentLifetime.get_token()]() -> std::expected<String, StarException> {
    if (lifetime.stop_requested())
      return std::unexpected(StarException("Current interface is detached"));
    return String("current");
  }));
  LuaBaseComponent second(fixture.root());
  ASSERT_TRUE(second.addCallbacks("interface", current));
  ASSERT_TRUE(second.addCallbacks("world", LuaCallbacks{}));
  ASSERT_TRUE(second.setLuaRoot(*luaRoot));
  ASSERT_TRUE(second.init());
  auto secondSession = second.eval<String>("return interface.session()");
  ASSERT_TRUE(secondSession) << secondSession.error().what();
  ASSERT_TRUE(*secondSession);
  EXPECT_EQ(**secondSession, String("current"));
  auto unavailableWorld = second.eval<bool>("return type(world) == 'table' and world.session == nil");
  ASSERT_TRUE(unavailableWorld) << unavailableWorld.error().what();
  EXPECT_TRUE(unavailableWorld->value_or(false));
  auto inherited = second.eval<bool>("return not pcall(inheritedOnly.session)");
  ASSERT_TRUE(inherited) << inherited.error().what();
  EXPECT_TRUE(inherited->value_or(false));
  ASSERT_TRUE(second.init());
  auto reinitialized = second.eval<String>("return interface.session()");
  ASSERT_TRUE(reinitialized) << reinitialized.error().what();
  ASSERT_TRUE(*reinitialized);
  EXPECT_EQ(**reinitialized, String("current"));
  ASSERT_TRUE(second.uninit());

  // Adding an explicit group to an already-running component has the same
  // precedence, without altering inherited tables in sibling contexts.
  LuaBaseComponent live(fixture.root());
  ASSERT_TRUE(live.setLuaRoot(*luaRoot));
  ASSERT_TRUE(live.init());
  auto inheritedBeforeBinding = live.eval<bool>("return not pcall(interface.session)");
  ASSERT_TRUE(inheritedBeforeBinding) << inheritedBeforeBinding.error().what();
  EXPECT_TRUE(inheritedBeforeBinding->value_or(false));
  ASSERT_TRUE(live.addCallbacks("interface", current));
  auto liveSession = live.eval<String>("return interface.session()");
  ASSERT_TRUE(liveSession) << liveSession.error().what();
  ASSERT_TRUE(*liveSession);
  EXPECT_EQ(**liveSession, String("current"));
  ASSERT_TRUE(live.addCallbacks("world", LuaCallbacks{}));
  auto shadowedWorld = live.eval<bool>("return type(world) == 'table' and world.session == nil");
  ASSERT_TRUE(shadowedWorld) << shadowedWorld.error().what();
  EXPECT_TRUE(shadowedWorld->value_or(false));
  ASSERT_TRUE(live.uninit());

  auto rootContext = (*luaRoot)->createContext(StringList{});
  ASSERT_TRUE(rootContext) << rootContext.error().what();
  auto stillRetired = rootContext->eval<bool>("return not pcall(interface.session) and not pcall(world.session)");
  ASSERT_TRUE(stillRetired) << stillRetired.error().what();
  EXPECT_TRUE(*stillRetired);
  auto retainedStillRetired = retainedSession->invoke<String>();
  EXPECT_FALSE(retainedStillRetired);
});

class ReentrantReloadScript : public LuaBaseComponent {
public:
  explicit ReentrantReloadScript(Root& root) : LuaBaseComponent(root) {}
protected:
  std::expected<void, StarException> contextSetup() override {
    if (auto result = LuaBaseComponent::contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    auto defined = context()->eval<LuaValue>(R"LUA(
      function probe() return true end
      function init() host.reenter("init") end
      function uninit() host.reenter("uninit") end
      function dismissed() host.reenter("dismissed") end
    )LUA");
    if (!defined)
      return std::unexpected(std::move(defined.error()));
    return {};
  }
};

const GameTestRegistration scriptReloadReentry("ScriptPane", "ExplicitInitAndAutomaticReloadAllowHookReentry", [](GameTest& fixture) {
  auto luaRoot = LuaRoot::create(fixture.root());
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  ReentrantReloadScript script(fixture.root());
  ASSERT_TRUE(script.setLuaRoot(*luaRoot));
  script.setAutoReInit(true);
  int initialized = 0;
  int uninitialized = 0;
  int dismissed = 0;
  LuaCallbacks callbacks;
  ASSERT_TRUE(callbacks.registerCallback("reenter", [&](String const& hook) -> std::expected<void, StarException> {
    if (hook == "init")
      ++initialized;
    else if (hook == "uninit")
      ++uninitialized;
    else
      ++dismissed;
    auto result = script.invoke<bool>("probe");
    if (!result)
      return std::unexpected(std::move(result.error()));
    if (!result->value_or(false))
      return std::unexpected(StarException("Reentrant hook lost its script context"));
    return {};
  }));
  ASSERT_TRUE(script.addCallbacks("host", std::move(callbacks)));
  ASSERT_TRUE(script.init());
  EXPECT_EQ(initialized, 1);
  ASSERT_TRUE(fixture.root().reload());
  ASSERT_TRUE(script.init());
  EXPECT_EQ(initialized, 2);
  EXPECT_EQ(uninitialized, 1);
  ASSERT_TRUE(fixture.root().reload());
  auto reloaded = script.invoke<bool>("probe");
  ASSERT_TRUE(reloaded) << reloaded.error().what();
  EXPECT_TRUE(reloaded->value_or(false));
  EXPECT_EQ(initialized, 3);
  EXPECT_EQ(uninitialized, 2);
  ASSERT_TRUE((*luaRoot)->restart());
  auto restarted = script.invoke<bool>("probe");
  ASSERT_TRUE(restarted) << restarted.error().what();
  EXPECT_TRUE(restarted->value_or(false));
  EXPECT_EQ(initialized, 4);
  EXPECT_EQ(uninitialized, 3);
  ASSERT_TRUE(fixture.root().reload());
  ASSERT_TRUE((*luaRoot)->restart());
  ASSERT_TRUE(script.uninit("dismissed"));
  EXPECT_EQ(initialized, 4);
  EXPECT_EQ(dismissed, 1);
  EXPECT_EQ(uninitialized, 4);
});

class ShutdownFailureScript : public LuaBaseComponent {
public:
  explicit ShutdownFailureScript(Root& root) : LuaBaseComponent(root) {}
  unsigned shutdowns = 0;
protected:
  std::expected<void, StarException> contextSetup() override {
    if (auto result = LuaBaseComponent::contextSetup(); !result)
      return std::unexpected(std::move(result.error()));
    auto defined = context()->eval<LuaValue>(R"LUA(
      function fail() error("shutdown reentry failure") end
      function dismissed() host.reenterFailure() end
      function uninit() host.uninitReached() end
    )LUA");
    if (!defined)
      return std::unexpected(std::move(defined.error()));
    return {};
  }
  std::expected<void, StarException> contextShutdown() override {
    ++shutdowns;
    return LuaBaseComponent::contextShutdown();
  }
};

const GameTestRegistration scriptShutdownFailure("ScriptPane", "ReentrantOwnerHookFailureShutsContextDownOnce", [](GameTest& fixture) {
  auto luaRoot = LuaRoot::create(fixture.root());
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  ShutdownFailureScript script(fixture.root());
  ASSERT_TRUE(script.setLuaRoot(*luaRoot));
  unsigned uninitReached = 0;
  LuaCallbacks callbacks;
  ASSERT_TRUE(callbacks.registerCallback("reenterFailure", [&]() -> std::expected<void, StarException> {
    auto result = script.invoke("fail");
    if (!result)
      return std::unexpected(std::move(result.error()));
    return {};
  }));
  ASSERT_TRUE(callbacks.registerCallback("uninitReached", [&]() { ++uninitReached; }));
  ASSERT_TRUE(script.addCallbacks("host", std::move(callbacks)));
  ASSERT_TRUE(script.init());
  std::expected<void, StarException> stopped;
  testing::TestPartResultArray expectedErrors;
  {
    // Keep the fixture's ErrorLogSink active and assert its two expected
    // failures, instead of disabling production logging for this error path.
    testing::ScopedFakeTestPartResultReporter capture(
        testing::ScopedFakeTestPartResultReporter::INTERCEPT_ONLY_CURRENT_THREAD, &expectedErrors);
    stopped = script.uninit("dismissed");
  }
  ASSERT_EQ(expectedErrors.size(), 2);
  std::array<std::string_view, 2> prefixes{
      "Error was logged: Exception while invoking lua function 'fail'.",
      "Error was logged: Exception while calling script shutdown hook:"};
  for (int i = 0; i < expectedErrors.size(); ++i) {
    auto const& result = expectedErrors.GetTestPartResult(i);
    EXPECT_EQ(result.type(), testing::TestPartResult::kNonFatalFailure);
    auto message = std::string_view(result.message());
    EXPECT_NE(message.find(prefixes[i]), std::string_view::npos);
    EXPECT_NE(message.find("shutdown reentry failure"), std::string_view::npos);
  }
  EXPECT_FALSE(stopped);
  EXPECT_FALSE(script.initialized());
  EXPECT_EQ(script.shutdowns, 1u);
  EXPECT_EQ(uninitReached, 0u);
});

const GameTestRegistration scriptHiddenInput("ScriptPane", "HiddenDisplayedPaneRejectsInputBeforeScriptHook", [](GameTest& fixture) {
  auto context = textEditorContext(fixture);
  ASSERT_TRUE(context) << context.error().what();
  auto luaRoot = LuaRoot::create(fixture.root());
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  auto created = ScriptTestPane::create(**context);
  ASSERT_TRUE(created) << created.error().what();
  auto& pane = **created;
  ASSERT_TRUE(pane.setLuaRoot(*luaRoot));
  ASSERT_TRUE(pane.displayed());
  auto editor = textEditor(**context, "A中");
  ASSERT_TRUE(editor) << editor.error().what();
  (*editor)->setEditorRendering(false);
  ASSERT_TRUE(pane.addChild("editor", *editor));
  ASSERT_TRUE((*editor)->focus());
  ASSERT_TRUE(pane.evaluate(R"LUA(
    storage.inputs = 0
    function onInput(event)
      storage.inputs = storage.inputs + 1
      return true
    end
  )LUA"));
  ASSERT_TRUE(pane.hide());
  EXPECT_TRUE(pane.isDisplayed());
  for (InputEvent const& event : {
      InputEvent{MouseButtonDownEvent{MouseButton::Left, {1, 1}}},
      InputEvent{KeyDownEvent{Key::A, KeyMod::NoMod}},
      InputEvent{TextInputEvent{"🙂"}}}) {
    auto consumed = pane.sendEvent(event);
    ASSERT_TRUE(consumed) << consumed.error().what();
    EXPECT_FALSE(*consumed);
  }
  auto inputs = pane.evaluate("return storage.inputs");
  ASSERT_TRUE(inputs) << inputs.error().what();
  ASSERT_TRUE(*inputs);
  EXPECT_EQ(**inputs, Json(0));
  EXPECT_EQ((*editor)->getText(), String("A中"));
  ASSERT_TRUE(pane.dismissed());
});

const GameTestRegistration scriptInputEditor("ScriptPane", "ConsumingInputPrecedesFocusedNativeEditor", [](GameTest& fixture) {
  auto configuration = fixture.root().configuration();
  ASSERT_TRUE(configuration) << configuration.error().what();
  auto bindings = Json::parseJson(R"JSON({
    "ChatStop":[{"type":"key","value":"Backspace","mods":[]}],
    "GuiClose":[{"type":"key","value":"Esc","mods":[]}]
  })JSON");
  ASSERT_TRUE(bindings) << bindings.error().what();
  ASSERT_TRUE((*configuration)->set("bindings", *bindings));
  auto context = GuiContext::create(fixture.root(), {}, {});
  ASSERT_TRUE(context) << context.error().what();
  auto luaRoot = LuaRoot::create(fixture.root());
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  auto created = ScriptTestPane::create(**context);
  ASSERT_TRUE(created) << created.error().what();
  auto& pane = **created;
  ASSERT_TRUE(pane.setLuaRoot(*luaRoot));
  ASSERT_TRUE(pane.displayed());
  ASSERT_TRUE(pane.evaluate(R"LUA(
    function onInput(event)
      storage.event = event
      return event.actions[1] == "ChatStop"
    end
    function read() return storage.event end
  )LUA"));
  auto editor = textEditor(**context, "A中");
  ASSERT_TRUE(editor) << editor.error().what();
  (*editor)->setEditorRendering(false);
  ASSERT_TRUE(pane.addChild("editor", *editor));
  ASSERT_TRUE((*editor)->focus());
  auto consumed = pane.sendEvent(KeyDownEvent{Key::Backspace, KeyMod::NoMod});
  ASSERT_TRUE(consumed) << consumed.error().what();
  EXPECT_TRUE(*consumed);
  EXPECT_EQ((*editor)->getText(), String("A中"));
  auto action = pane.call("read", {});
  ASSERT_TRUE(action) << action.error().what();
  ASSERT_TRUE(*action);
  auto actions = (**action).getArray("actions");
  ASSERT_TRUE(actions) << actions.error().what();
  EXPECT_EQ(*actions, (JsonArray{"ChatStop"}));
  auto release = pane.inputJson(KeyUpEvent{Key::Backspace});
  ASSERT_TRUE(release) << release.error().what();
  auto releaseActions = release->getArray("actions");
  ASSERT_TRUE(releaseActions) << releaseActions.error().what();
  EXPECT_TRUE(releaseActions->empty());
  auto committed = pane.sendEvent(TextInputEvent{"🙂"});
  ASSERT_TRUE(committed) << committed.error().what();
  EXPECT_TRUE(*committed);
  EXPECT_EQ((*editor)->getText(), String("A中🙂"));
  auto forwarded = pane.forwardEvent(KeyDownEvent{Key::Backspace, KeyMod::NoMod});
  ASSERT_TRUE(forwarded) << forwarded.error().what();
  EXPECT_TRUE(*forwarded);
  EXPECT_EQ((*editor)->getText(), String("A中"));
  ASSERT_TRUE(pane.evaluate("onInput = nil"));
  EXPECT_FALSE(pane.isDismissed());
  auto closed = pane.sendEvent(KeyDownEvent{Key::Escape, KeyMod::NoMod});
  ASSERT_TRUE(closed) << closed.error().what();
  EXPECT_TRUE(*closed);
  EXPECT_TRUE(pane.isDismissed());
  ASSERT_TRUE(pane.dismissed());
});


const GameTestRegistration scriptPaneBounds("ScriptPane", "WidgetOnlyPaneHasAutomaticOrExplicitRootBounds", [](GameTest& fixture) {
  auto context = textEditorContext(fixture);
  ASSERT_TRUE(context) << context.error().what();
  JsonObject config{{"gui", JsonObject{{"canvas", JsonObject{
      {"type", "canvas"}, {"position", JsonArray{7, 9}}, {"size", JsonArray{23, 11}}}}}}};
  auto automatic = BaseScriptPane::create(**context, config, true);
  ASSERT_TRUE(automatic) << automatic.error().what();
  EXPECT_EQ((*automatic)->size(), (Vec2I{30, 20}));
  config.set("size", JsonArray{100, 40});
  auto explicitSize = BaseScriptPane::create(**context, config, true);
  ASSERT_TRUE(explicitSize) << explicitSize.error().what();
  EXPECT_EQ((*explicitSize)->size(), (Vec2I{100, 40}));
});

class InputOrderPane : public Pane {
public:
  static std::expected<std::shared_ptr<InputOrderPane>, StarException> create(GuiContext& context, bool consumes) {
    auto pane = std::shared_ptr<InputOrderPane>(new InputOrderPane(context, consumes));
    if (auto result = pane->initializePane(); !result)
      return std::unexpected(std::move(result.error()));
    pane->setAnchor(PaneAnchor::BottomLeft);
    pane->setSize({10, 10});
    return pane;
  }
  std::expected<bool, StarException> sendEvent(InputEvent const& event) override {
    return m_consumes && std::holds_alternative<MouseButtonDownEvent>(event);
  }
private:
  InputOrderPane(GuiContext& context, bool consumes) : Pane(context), m_consumes(consumes) {}
  bool m_consumes;
};

const GameTestRegistration stableHudOrder("ScriptedHud", "PointerConsumptionRaisesWindowsButNotFixedLayers", [](GameTest& fixture) {
  auto context = textEditorContext(fixture);
  ASSERT_TRUE(context) << context.error().what();
  for (auto layer : {PaneLayer::Hud, PaneLayer::World, PaneLayer::Window}) {
    PaneManager manager(**context);
    auto back = InputOrderPane::create(**context, true);
    auto front = InputOrderPane::create(**context, false);
    ASSERT_TRUE(back) << back.error().what();
    ASSERT_TRUE(front) << front.error().what();
    ASSERT_TRUE(manager.displayPane(layer, *back, {}));
    ASSERT_TRUE(manager.displayPane(layer, *front, {}));
    EXPECT_EQ(manager.topPane({layer}), *front);
    auto consumed = manager.sendInputEvent(MouseButtonDownEvent{MouseButton::Left, {1, 1}});
    ASSERT_TRUE(consumed) << consumed.error().what();
    EXPECT_TRUE(*consumed);
    EXPECT_EQ(manager.topPane({layer}), layer == PaneLayer::Window ? *back : *front);
    ASSERT_TRUE(manager.dismissAllPanes());
  }
});

const GameTestRegistration uiWorldScope("LuaUiManager", "WorldTransitionDropsScopedEventsBeforeReleasingNativePane", [](GameTest& fixture) {
  auto context = textEditorContext(fixture);
  ASSERT_TRUE(context) << context.error().what();
  auto luaRoot = LuaRoot::create(fixture.root());
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  auto script = (*luaRoot)->createContext("/interface/opensb/ui/manager.lua");
  ASSERT_TRUE(script) << script.error().what();
  PaneManager panes(**context);
  auto created = ScriptTestPane::create(**context);
  ASSERT_TRUE(created) << created.error().what();
  auto worldPane = std::move(*created);
  ASSERT_TRUE(worldPane->setLuaRoot(*luaRoot));
  worldPane->setAnchor(PaneAnchor::BottomLeft);
  auto displayed = panes.displayPane(PaneLayer::Window, worldPane, {});
  ASSERT_TRUE(displayed) << displayed.error().what();
  std::weak_ptr<Pane> released = worldPane;
  auto retained = worldPane->makePaneCallbacks();
  ASSERT_TRUE(retained) << retained.error().what();
  ASSERT_TRUE(script->setCallbacks("retained", *retained));
  LuaCallbacks callbacks;
  ASSERT_TRUE(callbacks.registerCallback("paneIdAvailable", &MainInterface::paneIdAvailable));
  unsigned destroyed = 0;
  ASSERT_TRUE(callbacks.registerCallback("destroyPane", [&](String const& id) -> std::expected<void, StarException> {
    if (id == "worldOpen" && worldPane) {
      if (auto dismissed = panes.dismissPane(worldPane); !dismissed)
        return std::unexpected(std::move(dismissed.error()));
      worldPane.reset();
      ++destroyed;
    }
    return {};
  }));
  ASSERT_TRUE(script->setCallbacks("interface", callbacks));
  // Only the native destroy operation is needed here, and it releases a real
  // displayed Pane. No callPane echo exists: dispatch before teardown fails.
  auto suspended = script->eval<Json>(R"LUA(
    local state = {
      session = {inWorld = true, worldGeneration = 1},
      open = {worldOpen = true, worldClosed = false, sessionPane = true}
    }
    local definitions = {
      worldOpen = {config = {}, scope = "world"},
      worldClosed = {config = {}, scope = "world"},
      sessionPane = {config = {}, scope = "session"}
    }
    manager = UiManager.new(state, definitions, state.session)
    manager:enqueue("worldOpen", "message", {old = true})
    manager:enqueue("worldClosed", "message", {old = true})
    manager:enqueue("$ui", "open", {id = "worldOpen"})
    manager:enqueue("$ui", "close", {id = "worldClosed"})
    manager:enqueue("sessionPane", "message", {keep = true})
    manager:enqueue("$ui", "open", {id = "sessionPane"})
    manager:enqueue("$ui", "register", {id = "worldOpen", definition = definitions.worldOpen})
    manager:suspend({inWorld = true, worldGeneration = 2})
    local alive = pcall(retained.isDisplayed)
    return {#state.queue, state.queue[1].id, state.queue[2].payload.id,
      state.queue[3].event, state.open.worldOpen == nil,
      state.open.worldClosed == nil, state.open.sessionPane, alive}
  )LUA");
  ASSERT_TRUE(suspended) << suspended.error().what();
  EXPECT_EQ(*suspended, (Json(JsonArray{3, "sessionPane", "sessionPane", "register", true, true, true, false})));
  EXPECT_EQ(destroyed, 1u);
  EXPECT_TRUE(released.expired());
  EXPECT_FALSE(panes.topPane());

  created = ScriptTestPane::create(**context);
  ASSERT_TRUE(created) << created.error().what();
  worldPane = std::move(*created);
  ASSERT_TRUE(worldPane->setLuaRoot(*luaRoot));
  worldPane->setAnchor(PaneAnchor::BottomLeft);
  displayed = panes.displayPane(PaneLayer::Window, worldPane, {});
  ASSERT_TRUE(displayed) << displayed.error().what();
  released = worldPane;
  // Engine replacement can bypass suspend. Restoring the real Lua checkpoint
  // must apply the same old-world event filtering before adopting instances.
  auto restored = script->eval<Json>(R"LUA(
    local state = {
      session = {inWorld = true, worldGeneration = 1},
      open = {worldOpen = true},
      queue = jarray()
    }
    manager = UiManager.new(state, {worldOpen = {config = {}, scope = "world"}},
      {inWorld = true, worldGeneration = 2})
    manager:enqueue("worldOpen", "message", {old = true})
    manager:restore()
    return {#state.queue, state.open.worldOpen == nil}
  )LUA");
  ASSERT_TRUE(restored) << restored.error().what();
  EXPECT_EQ(*restored, (Json(JsonArray{0, true})));
  EXPECT_EQ(destroyed, 2u);
  EXPECT_TRUE(released.expired());
  EXPECT_FALSE(panes.topPane());
});

const GameTestRegistration uiInputAndQueue("LuaUiManager", "NativePriorityReloadPairingAndDeferredHandlerEvents", [](GameTest& fixture) {
  auto context = textEditorContext(fixture);
  ASSERT_TRUE(context) << context.error().what();
  auto luaRoot = LuaRoot::create(fixture.root());
  ASSERT_TRUE(luaRoot) << luaRoot.error().what();
  PaneManager panes(**context);
  Map<String, std::shared_ptr<ScriptTestPane>> instances;
  LuaUpdateScript controller(fixture.root());
  ASSERT_TRUE(controller.setScript("/interface/opensb/ui/manager.lua"));
  ASSERT_TRUE(controller.setLuaRoot(*luaRoot));
  LuaCallbacks callbacks;
  ASSERT_TRUE(callbacks.registerCallback("paneIdAvailable", &MainInterface::paneIdAvailable));
  ASSERT_TRUE(callbacks.registerCallback("bindRegisteredPane", [&](String const& id) -> std::expected<std::optional<LuaCallbacks>, StarException> {
    auto instance = instances.maybeRef(id);
    if (!instance) return std::optional<LuaCallbacks>{};
    auto bound = instance->get()->makePaneCallbacks();
    if (!bound) return std::unexpected(std::move(bound.error()));
    return std::optional<LuaCallbacks>(std::move(*bound));
  }));
  ASSERT_TRUE(callbacks.registerCallback("callPane", [&](String const& id, String const& method, JsonArray const& args) -> std::expected<std::optional<Json>, StarException> {
    auto instance = instances.maybeRef(id);
    if (!instance) return std::unexpected(GuiException("Missing test pane"));
    return instance->get()->call(method, args);
  }));
  ASSERT_TRUE(callbacks.registerCallback("displayRegisteredPane", [&](String const& id) -> std::expected<void, StarException> {
    auto instance = instances.maybeRef(id);
    if (!instance) return std::unexpected(GuiException("Missing test pane"));
    if (panes.isDisplayed(instance->get())) return instance->get()->show();
    return panes.displayPane(PaneLayer::Window, instance->get(), {});
  }));
  ASSERT_TRUE(callbacks.registerCallback("destroyPane", [&](String const& id) -> std::expected<void, StarException> {
    auto instance = instances.maybeRef(id);
    if (!instance) return {};
    if (panes.isDisplayed(instance->get())) {
      if (auto dismissed = panes.dismissPane(instance->get()); !dismissed)
        return std::unexpected(std::move(dismissed.error()));
    }
    instances.remove(id);
    return {};
  }));
  ASSERT_TRUE(callbacks.registerCallback("paneInputTargets", [&]() -> std::expected<JsonArray, StarException> {
    auto targets = panes.inputTargetPanes();
    if (!targets) return std::unexpected(std::move(targets.error()));
    JsonArray ids;
    for (auto const& pane : *targets)
      for (auto const& entry : instances)
        if (entry.second == pane) ids.append(entry.first);
    return ids;
  }));
  ASSERT_TRUE(controller.addCallbacks("interface", callbacks));
  ASSERT_TRUE(controller.init());
  unsigned cancellations = 0;
  for (auto const& id : {String("a"), String("b")}) {
    auto created = ScriptTestPane::create(**context);
    ASSERT_TRUE(created) << created.error().what();
    auto pane = std::move(*created);
    pane->setAnchor(PaneAnchor::BottomLeft);
    ASSERT_TRUE(pane->setLuaRoot(*luaRoot));
    LuaCallbacks host;
    ASSERT_TRUE(host.registerCallback("send", [&, id](String const& event, Json payload) -> std::expected<void, StarException> {
      auto queued = controller.invoke<Json>("enqueue", id, event, std::move(payload));
      if (!queued) return std::unexpected(std::move(queued.error()));
      return {};
    }));
    ASSERT_TRUE(host.registerCallback("cancelled", [&]() { ++cancellations; }));
    ASSERT_TRUE(pane->addCallbacks("host", std::move(host)));
    auto displayed = panes.displayPane(PaneLayer::Window, pane, {});
    ASSERT_TRUE(displayed) << displayed.error().what();
    ASSERT_TRUE(pane->evaluate(R"LUA(
      function onAction(action, pressed, event)
        storage.held = pressed
        if event and event.cancelled then host.cancelled() end
        return true
      end
      function onEvent(name, payload)
        storage.events = (storage.events or 0) + 1
        host.send(name, payload)
      end
    )LUA"));
    instances.set(id, std::move(pane));
  }
  String setup = R"LUA(
    local definitions = {
      a = {config = {}, scope = "session", order = 100, actions = {Probe = "hold"}},
      b = {config = {}, scope = "session", order = 0, actions = {Probe = "hold"}}
    }
    storage.open = storage.open or {a = true, b = true}
    manager = UiManager.new(storage, definitions, {inWorld = true, worldGeneration = 1})
    manager:restore()
    function enqueue(id, event, payload) manager:enqueue(id, event, payload) end
    return true
  )LUA";
  ASSERT_TRUE(controller.eval<Json>(setup));
  auto initial = controller.eval<Json>(R"LUA(
    local configured = pcall(UiManager.new, {}, {Inventory = {config = {}}}, {})
    local registered = pcall(function() manager:register("Inventory", {config = {}, open = true}) end)
    local handled = manager:input({type = "keyDown", key = 1, actions = {"Probe"}})
    return {configured, registered, manager.definitions.Inventory == nil,
      storage.registered.Inventory == nil, handled, storage.pressed["1"].id}
  )LUA");
  ASSERT_TRUE(initial) << initial.error().what();
  ASSERT_TRUE(*initial);
  EXPECT_EQ(**initial, (Json(JsonArray{false, false, true, true, true, "b"})));

  auto a = instances.value("a");
  auto b = instances.value("b");
  ASSERT_TRUE(a);
  ASSERT_TRUE(b);
  ASSERT_TRUE(panes.dismissPane(a));
  auto redisplayed = panes.displayPane(PaneLayer::Window, a, {});
  ASSERT_TRUE(redisplayed) << redisplayed.error().what();
  // Native redisplay raises A above B despite the fixed declaration order.
  ASSERT_TRUE(a->evaluate("function onAction(action, pressed, event) storage.held = pressed; if event and event.cancelled then host.cancelled() end; return true end"));
  auto raised = controller.eval<Json>("manager:input({type='keyDown', key=2, actions={'Probe'}}); return storage.pressed['2'].id");
  ASSERT_TRUE(raised) << raised.error().what();
  ASSERT_TRUE(*raised);
  EXPECT_EQ(**raised, Json("a"));
  auto modal = InputOrderPane::create(**context, false);
  ASSERT_TRUE(modal) << modal.error().what();
  ASSERT_TRUE(panes.displayPane(PaneLayer::ModalWindow, *modal, {}));
  auto blocked = controller.eval<Json>("return manager:input({type='keyDown', key=3, actions={'Probe'}})");
  ASSERT_TRUE(blocked) << blocked.error().what();
  ASSERT_TRUE(*blocked);
  EXPECT_EQ(**blocked, Json(false));
  ASSERT_TRUE(panes.dismissPane(*modal));
  auto editor = textEditor(**context, "");
  ASSERT_TRUE(editor) << editor.error().what();
  ASSERT_TRUE(b->addChild("editor", *editor));
  ASSERT_TRUE((*editor)->focus());
  auto captured = panes.inputTargetPanes();
  ASSERT_TRUE(captured) << captured.error().what();
  EXPECT_EQ(*captured, (List<std::shared_ptr<Pane>>{b}));
  ASSERT_TRUE((*editor)->blur());

  ASSERT_TRUE(controller.uninit());
  ASSERT_TRUE(controller.init());
  ASSERT_TRUE(controller.eval<Json>(setup));
  auto released = controller.eval<Json>(R"LUA(
    local paired = manager:input({type = "keyUp", key = 1, actions = {}})
    manager:close("a")
    return {paired, storage.pressed["1"] == nil, storage.pressed["2"] == nil}
  )LUA");
  ASSERT_TRUE(released) << released.error().what();
  ASSERT_TRUE(*released);
  EXPECT_EQ(**released, (Json(JsonArray{true, true, true})));
  EXPECT_EQ(cancellations, 1u);
  auto held = b->evaluate("return storage.held");
  ASSERT_TRUE(held) << held.error().what();
  ASSERT_TRUE(*held);
  EXPECT_EQ(**held, Json(false));

  auto first = controller.eval<Json>("manager:enqueue('b','pulse',{}); manager:update(); return #storage.queue");
  ASSERT_TRUE(first) << first.error().what();
  ASSERT_TRUE(*first);
  EXPECT_EQ(**first, Json(1));
  auto events = b->evaluate("return storage.events");
  ASSERT_TRUE(events) << events.error().what();
  ASSERT_TRUE(*events);
  EXPECT_EQ(**events, Json(1));
  auto second = controller.eval<Json>("manager:update(); return #storage.queue");
  ASSERT_TRUE(second) << second.error().what();
  ASSERT_TRUE(*second);
  EXPECT_EQ(**second, Json(1));
  events = b->evaluate("return storage.events");
  ASSERT_TRUE(events) << events.error().what();
  ASSERT_TRUE(*events);
  EXPECT_EQ(**events, Json(2));
  ASSERT_TRUE(controller.eval<Json>("manager:shutdown()"));
  ASSERT_TRUE(controller.uninit());
  EXPECT_TRUE(instances.empty());
  EXPECT_FALSE(panes.topPane());
});

}
