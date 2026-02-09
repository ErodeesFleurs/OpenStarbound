#include "StarInput.hpp"

#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

const char* InputBindingConfigRoot = "modBindings";

BiMap<Key, KeyMod> const KeysToMods{
  {Key::LShift, KeyMod::LShift},
  {Key::RShift, KeyMod::RShift},
  {Key::LCtrl, KeyMod::LCtrl},
  {Key::RCtrl, KeyMod::RCtrl},
  {Key::LAlt, KeyMod::LAlt},
  {Key::RAlt, KeyMod::RAlt},
  {Key::LGui, KeyMod::LGui},
  {Key::RGui, KeyMod::RGui},
  {Key::AltGr, KeyMod::AltGr},
  {Key::ScrollLock, KeyMod::Scroll}};

const KeyMod KeyModOptional = KeyMod::Num | KeyMod::Caps | KeyMod::Scroll;

inline auto compareKeyModLenient(KeyMod input, KeyMod test) -> bool {
  input |= KeyModOptional;
  test |= KeyModOptional;
  return (test & input) == test;
}

inline auto compareKeyMod(KeyMod input, KeyMod test) -> bool {
  return (input | (KeyModOptional & ~test)) == (test | KeyModOptional);
}

auto keyModsToJson(KeyMod mod) -> Json {
  JsonArray array;

  if (bool(mod & KeyMod::LShift))
    array.emplace_back("LShift");
  if (bool(mod & KeyMod::RShift))
    array.emplace_back("RShift");
  if (bool(mod & KeyMod::LCtrl))
    array.emplace_back("LCtrl");
  if (bool(mod & KeyMod::RCtrl))
    array.emplace_back("RCtrl");
  if (bool(mod & KeyMod::LAlt))
    array.emplace_back("LAlt");
  if (bool(mod & KeyMod::RAlt))
    array.emplace_back("RAlt");
  if (bool(mod & KeyMod::LGui))
    array.emplace_back("LGui");
  if (bool(mod & KeyMod::RGui))
    array.emplace_back("RGui");
  if (bool(mod & KeyMod::Num))
    array.emplace_back("Num");
  if (bool(mod & KeyMod::Caps))
    array.emplace_back("Caps");
  if (bool(mod & KeyMod::AltGr))
    array.emplace_back("AltGr");
  if (bool(mod & KeyMod::Scroll))
    array.emplace_back("Scroll");

  return array.empty() ? Json() : std::move(array);
}

// Optional pointer argument to output calculated priority
auto keyModsFromJson(Json const& json, std::uint8_t* priority = nullptr) -> KeyMod {
  KeyMod mod = KeyMod::NoMod;
  if (!json.isType(Json::Type::Array))
    return mod;

  std::uint8_t modPriority = 0;
  for (Json const& jMod : json.toArray()) {
    KeyMod changedMod = mod | KeyModNames.getLeft(jMod.toString());
    if (mod != changedMod) {
      mod = changedMod;
      ++modPriority;
    }
  }
  if (priority)
    *priority = modPriority;

  return mod;
}

auto hash<InputVariant>::operator()(InputVariant const& v) const -> size_t {
  size_t indexHash = hashOf(v.typeIndex());
  if (auto key = v.ptr<Key>())
    hashCombine(indexHash, hashOf(*key));
  else if (auto mButton = v.ptr<MouseButton>())
    hashCombine(indexHash, hashOf(*mButton));
  else if (auto cButton = v.ptr<ControllerButton>())
    hashCombine(indexHash, hashOf(*cButton));

  return indexHash;
}

auto Input::inputEventToJson(InputEvent const& input) -> Json {
  String type;
  Json data;

  if (auto keyDown = input.ptr<KeyDownEvent>()) {
    type = "KeyDown";
    data = JsonObject{
      {"key", KeyNames.getRight(keyDown->key)},
      {"mods", keyModsToJson(keyDown->mods)}};
  } else if (auto keyUp = input.ptr<KeyUpEvent>()) {
    type = "KeyUp";
    data = JsonObject{
      {"key", KeyNames.getRight(keyUp->key)}};
  } else if (auto mouseDown = input.ptr<MouseButtonDownEvent>()) {
    type = "MouseButtonDown";
    data = JsonObject{
      {"mouseButton", MouseButtonNames.getRight(mouseDown->mouseButton)},
      {"mousePosition", jsonFromVec2F(mouseDown->mousePosition)}};
  } else if (auto mouseUp = input.ptr<MouseButtonUpEvent>()) {
    type = "MouseButtonUp";
    data = JsonObject{
      {"mouseButton", MouseButtonNames.getRight(mouseUp->mouseButton)},
      {"mousePosition", jsonFromVec2F(mouseUp->mousePosition)}};
  } else if (auto mouseWheel = input.ptr<MouseWheelEvent>()) {
    type = "MouseWheel";
    data = JsonObject{
      {"mouseWheel", mouseWheel->mouseWheel == MouseWheel::Up ? 1 : -1},
      {"mousePosition", jsonFromVec2F(mouseWheel->mousePosition)}};
  } else if (auto mouseMove = input.ptr<MouseMoveEvent>()) {
    type = "MouseMove";
    data = JsonObject{
      {"mouseMove", jsonFromVec2F(mouseMove->mouseMove)},
      {"mousePosition", jsonFromVec2F(mouseMove->mousePosition)}};
  } else if (auto controllerDown = input.ptr<ControllerButtonDownEvent>()) {
    type = "ControllerButtonDown";
    data = JsonObject{
      {"controllerButton", ControllerButtonNames.getRight(controllerDown->controllerButton)},
      {"controller", controllerDown->controller}};
  } else if (auto controllerUp = input.ptr<ControllerButtonUpEvent>()) {
    type = "ControllerButtonUp";
    data = JsonObject{
      {"controllerButton", ControllerButtonNames.getRight(controllerUp->controllerButton)},
      {"controller", controllerUp->controller}};
  } else if (auto controllerAxis = input.ptr<ControllerAxisEvent>()) {
    type = "ControllerAxis";
    data = JsonObject{
      {"controllerAxis", ControllerAxisNames.getRight(controllerAxis->controllerAxis)},
      {"controllerAxisValue", controllerAxis->controllerAxisValue},
      {"controller", controllerAxis->controller}};
  }

  if (data) {
    return JsonObject{
      {"type", type},
      {"data", data}};
  }

  return data;
}

auto Input::bindFromJson(Json const& json) -> Input::Bind {
  Bind bind;
  if (json.isNull())
    return bind;

  String type = json.getString("type");
  Json value = json.get("value", {});

  if (type == "key") {
    KeyBind keyBind;
    if (auto key = KeyNames.maybeLeft(value.toString()))
      keyBind.key = *key;
    else
      return bind;
    keyBind.mods = keyModsFromJson(json.getArray("mods", {}), &keyBind.priority);
    bind = std::move(keyBind);
  } else if (type == "mouse") {
    MouseBind mouseBind;
    if (auto button = MouseButtonNames.maybeLeft(value.toString()))
      mouseBind.button = *button;
    else
      return bind;
    mouseBind.mods = keyModsFromJson(json.getArray("mods", {}), &mouseBind.priority);
    bind = std::move(mouseBind);
  } else if (type == "controller") {
    ControllerBind controllerBind;
    if (auto button = ControllerButtonNames.maybeLeft(value.toString()))
      controllerBind.button = *button;
    else
      return bind;
    controllerBind.controller = json.getUInt("controller", 0);
    bind = std::move(controllerBind);
  }

  return bind;
}

auto Input::bindToJson(Bind const& bind) -> Json {
  if (auto keyBind = bind.ptr<KeyBind>()) {
    auto obj = JsonObject{
      {"type", "key"},
      {"value", KeyNames.getRight(keyBind->key)}};// don't want empty mods to exist as null entry
    if (auto mods = keyModsToJson(keyBind->mods))
      obj.emplace("mods", std::move(mods));
    return obj;
  } else if (auto mouseBind = bind.ptr<MouseBind>()) {
    auto obj = JsonObject{
      {"type", "mouse"},
      {"value", MouseButtonNames.getRight(mouseBind->button)}};
    if (auto mods = keyModsToJson(mouseBind->mods))
      obj.emplace("mods", std::move(mods));
    return obj;
  } else if (auto controllerBind = bind.ptr<ControllerBind>()) {
    return JsonObject{
      {"type", "controller"},
      {"value", ControllerButtonNames.getRight(controllerBind->button)},
      {"controller", controllerBind->controller}};
  }

  return {};
}

Input::BindEntry::BindEntry(String entryId, Json const& config, BindCategory const& parentCategory) {
  category = &parentCategory;
  id = entryId;
  name = config.getString("name", id);
  tags = jsonToStringList(config.get("tags", JsonArray()));
  for (Json const& jBind : config.getArray("default", {})) {
    try {
      defaultBinds.emplace_back(bindFromJson(jBind));
    } catch (JsonException const& e) { Logger::error("Binds: Error loading default bind in {}.{}: {}", parentCategory.id, id, e.what()); }
  }
}

void Input::BindEntry::updated() {
  auto config = Root::singleton().configuration();

  JsonArray array;
  for (auto const& bind : customBinds)
    array.emplace_back(bindToJson(bind));

  if (!config->get(InputBindingConfigRoot).isType(Json::Type::Object))
    config->set(InputBindingConfigRoot, JsonObject());

  String path = strf("{}.{}", InputBindingConfigRoot, category->id);
  if (!config->getPath(path).isType(Json::Type::Object)) {
    config->setPath(path, JsonObject{{id, std::move(array)}});
  } else {
    path = strf("{}.{}", path, id);
    config->setPath(path, array);
  }

  Input::singleton().rebuildMappings();
}

Input::BindRef::BindRef(BindEntry& bindEntry, KeyBind& keyBind) {
  entry = &bindEntry;
  priority = keyBind.priority;
  mods = keyBind.mods;
}

Input::BindRef::BindRef(BindEntry& bindEntry, MouseBind& mouseBind) {
  entry = &bindEntry;
  priority = mouseBind.priority;
  mods = mouseBind.mods;
}

Input::BindRef::BindRef(BindEntry& bindEntry) {
  entry = &bindEntry;
  priority = 0;
  mods = KeyMod::NoMod;
}

Input::BindCategory::BindCategory(String categoryId, Json const& categoryConfig) {
  id = categoryId;
  config = categoryConfig;
  name = config.getString("name", id);

  Ptr<Configuration> userConfig = Root::singletonPtr()->configuration();
  auto userBindings = userConfig->get(InputBindingConfigRoot);

  for (auto& pair : config.getObject("binds", {})) {
    String const& bindId = pair.first;
    Json const& bindConfig = pair.second;
    if (!bindConfig.isType(Json::Type::Object))
      continue;

    BindEntry& entry = entries.try_emplace(bindId, bindId, bindConfig, *this).first->second;

    if (userBindings.isType(Json::Type::Object)) {
      for (auto& jBind : userBindings.queryArray(strf("{}.{}", id, bindId), {})) {
        try {
          entry.customBinds.emplace_back(bindFromJson(jBind));
        } catch (JsonException const& e) { Logger::error("Binds: Error loading user bind in {}.{}: {}", id, bindId, e.what()); }
      }
    }

    if (entry.customBinds.empty())
      entry.customBinds = entry.defaultBinds;
  }
}

auto Input::filterBindEntries(List<Input::BindRef> const& binds, KeyMod mods) const -> List<Input::BindEntry*> {
  std::uint8_t maxPriority = 0;
  List<BindEntry*> result{};
  for (const BindRef& bind : binds) {
    if (bind.priority < maxPriority)
      break;
    else if (compareKeyModLenient(mods, bind.mods)) {
      maxPriority = bind.priority;
      result.emplace_back(bind.entry);
    }
  }
  return result;
}

auto Input::bindEntryPtr(String const& categoryId, String const& bindId) -> Input::BindEntry* {
  if (auto category = m_bindCategories.ptr(categoryId)) {
    if (auto entry = category->entries.ptr(bindId)) {
      return entry;
    }
  }

  return nullptr;
}

auto Input::bindEntry(String const& categoryId, String const& bindId) -> Input::BindEntry& {
  if (auto ptr = bindEntryPtr(categoryId, bindId))
    return *ptr;
  else
    throw InputException::format("Could not find bind entry {}.{}", categoryId, bindId);
}

auto Input::bindStatePtr(String const& categoryId, String const& bindId) -> Input::InputState* {
  if (auto ptr = bindEntryPtr(categoryId, bindId))
    return m_bindStates.ptr(ptr);
  else
    return nullptr;
}

auto Input::addBindState(BindEntry const* bindEntry) -> Input::InputState& {
  auto insertion = m_bindStates.insert(bindEntry, InputState());
  if (insertion.second) {
    for (auto& tag : bindEntry->tags)
      ++m_activeTags[tag];
  }
  return insertion.first->second;
}

Input* Input::s_singleton;

auto Input::singletonPtr() -> Input* {
  return s_singleton;
}

auto Input::singleton() -> Input& {
  if (!s_singleton)
    throw InputException("Input::singleton() called with no Input instance available");
  else
    return *s_singleton;
}

Input::Input() {
  if (s_singleton)
    throw InputException("Singleton Input has been constructed twice");

  s_singleton = this;

  m_pressedMods = KeyMod::NoMod;

  reload();

  m_rootReloadListener = std::make_shared<CallbackListener>([&]() -> void {
    reload();
  });

  Root::singletonPtr()->registerReloadListener(m_rootReloadListener);
}

Input::~Input() {
  s_singleton = nullptr;
}

auto Input::inputEventsThisFrame() const -> List<std::pair<InputEvent, bool>> const& {
  return m_inputEvents;
}

void Input::reset(bool clear) {
  m_inputEvents.clear();
  if (clear) {
    m_keyStates.clear();
    m_mouseStates.clear();
    m_controllerStates.clear();
    m_bindStates.clear();
  } else {
    auto eraseCond = [](auto& p) -> auto {
      if (p.second.held)
        p.second.reset();
      return !p.second.held;
    };

    eraseWhere(m_keyStates, eraseCond);
    eraseWhere(m_mouseStates, eraseCond);
    eraseWhere(m_controllerStates, eraseCond);
    eraseWhere(m_bindStates, [&](auto& p) -> auto {
      if (p.second.held)
        p.second.reset();
      else {
        for (auto& tag : p.first->tags) {
          auto find = m_activeTags.find(tag);
          if (find != m_activeTags.end() && !--find->second)
            m_activeTags.erase(find);
        }
        return true;
      }
      return false;
    });
  }
}

void Input::update() {
  reset();
}

auto Input::handleInput(InputEvent const& input, bool gameProcessed) -> bool {
  m_inputEvents.emplace_back(input, gameProcessed);
  if (auto keyDown = input.ptr<KeyDownEvent>()) {
    auto keyToMod = KeysToMods.rightPtr(keyDown->key);
    if (keyToMod)
      m_pressedMods |= *keyToMod;

    if (!gameProcessed && !m_textInputActive) {
      auto& state = m_keyStates[keyDown->key];
      if (keyToMod)
        state.mods |= *keyToMod;
      state.press();

      if (auto binds = m_bindMappings.ptr(keyDown->key)) {
        for (auto bind : filterBindEntries(*binds, keyDown->mods))
          addBindState(bind).press();
      }
    }
  } else if (auto keyUp = input.ptr<KeyUpEvent>()) {
    auto keyToMod = KeysToMods.rightPtr(keyUp->key);
    if (keyToMod)
      m_pressedMods &= ~*keyToMod;

    // We need to be able to release input even when gameProcessed is true, but only if it's already down.
    if (auto state = m_keyStates.ptr(keyUp->key)) {
      if (keyToMod)
        state->mods &= ~*keyToMod;
      state->release();
    }

    if (auto binds = m_bindMappings.ptr(keyUp->key)) {
      for (auto& bind : *binds) {
        if (auto state = m_bindStates.ptr(bind.entry))
          state->release();
      }
    }
  } else if (auto mouseDown = input.ptr<MouseButtonDownEvent>()) {
    m_mousePosition = mouseDown->mousePosition;
    if (!gameProcessed) {
      auto& state = m_mouseStates[mouseDown->mouseButton];
      state.pressPositions.append(mouseDown->mousePosition);
      state.press();

      if (auto binds = m_bindMappings.ptr(mouseDown->mouseButton)) {
        for (auto bind : filterBindEntries(*binds, m_pressedMods))
          addBindState(bind).press();
      }
    }
  } else if (auto mouseUp = input.ptr<MouseButtonUpEvent>()) {
    m_mousePosition = mouseUp->mousePosition;
    if (auto state = m_mouseStates.ptr(mouseUp->mouseButton)) {
      state->releasePositions.append(mouseUp->mousePosition);
      state->release();
    }

    if (auto binds = m_bindMappings.ptr(mouseUp->mouseButton)) {
      for (auto& bind : *binds) {
        if (auto state = m_bindStates.ptr(bind.entry))
          state->release();
      }
    }
  } else if (auto mouseMove = input.ptr<MouseMoveEvent>()) {
    m_mousePosition = mouseMove->mousePosition;
  } else if (auto controllerDown = input.ptr<ControllerButtonDownEvent>()) {
    if (!gameProcessed) {
      auto& state = m_controllerStates[controllerDown->controllerButton];
      state.press();

      if (auto binds = m_bindMappings.ptr(controllerDown->controllerButton)) {
        for (auto bind : filterBindEntries(*binds, m_pressedMods))
          addBindState(bind).press();
      }
    }
  } else if (auto controllerUp = input.ptr<ControllerButtonUpEvent>()) {
    if (auto state = m_controllerStates.ptr(controllerUp->controllerButton))
      state->release();

    if (auto binds = m_bindMappings.ptr(controllerUp->controllerButton)) {
      for (auto& bind : *binds) {
        if (auto state = m_bindStates.ptr(bind.entry))
          state->release();
      }
    }
  }

  return false;
}

void Input::rebuildMappings() {
  reset(true);
  m_bindMappings.clear();

  for (auto& category : m_bindCategories) {
    for (auto& pair : category.second.entries) {
      auto& entry = pair.second;
      for (auto& bind : entry.customBinds) {
        if (auto keyBind = bind.ptr<KeyBind>())
          m_bindMappings[keyBind->key].emplace_back(entry, *keyBind);
        if (auto mouseBind = bind.ptr<MouseBind>())
          m_bindMappings[mouseBind->button].emplace_back(entry, *mouseBind);
        if (auto controllerBind = bind.ptr<ControllerBind>())
          m_bindMappings[controllerBind->button].emplace_back(entry);
      }
    }
  }

  for (auto& pair : m_bindMappings) {
    pair.second.sort([](BindRef const& a, BindRef const& b)
                       -> bool { return a.priority > b.priority; });
  }
}

void Input::reload() {
  ;
  m_bindCategories.clear();

  auto assets = Root::singleton().assets();

  for (auto& bindPath : assets->scanExtension("binds")) {
    for (auto& pair : assets->json(bindPath).toObject()) {
      String const& categoryId = pair.first;
      Json const& categoryConfig = pair.second;
      if (!categoryConfig.isType(Json::Type::Object))
        continue;

      m_bindCategories.try_emplace(categoryId, categoryId, categoryConfig);
    }
  }

  size_t count = 0;
  for (auto& pair : m_bindCategories)
    count += pair.second.entries.size();

  Logger::info("Binds: Loaded {} bind{}", count, count == 1 ? "" : "s");

  rebuildMappings();
}

void Input::setTextInputActive(bool active) {
  m_textInputActive = active;
}

auto Input::bindDown(String const& categoryId, String const& bindId) -> std::optional<unsigned> {
  if (auto state = bindStatePtr(categoryId, bindId)) {
    if (state->presses)
      return state->presses;
  }
  return {};
}

auto Input::bindHeld(String const& categoryId, String const& bindId) -> bool {
  if (auto state = bindStatePtr(categoryId, bindId))
    return state->held;
  else
    return false;
}

auto Input::bindUp(String const& categoryId, String const& bindId) -> std::optional<unsigned> {
  if (auto state = bindStatePtr(categoryId, bindId)) {
    if (state->releases)
      return state->releases;
  }
  return {};
}

auto Input::keyDown(Key key, std::optional<KeyMod> keyMod) -> std::optional<unsigned> {
  if (auto state = m_keyStates.ptr(key)) {
    if (state->presses && (!keyMod || compareKeyMod(*keyMod, state->mods)))
      return state->presses;
  }
  return {};
}

auto Input::keyHeld(Key key) -> bool {
  auto state = m_keyStates.ptr(key);
  return state && state->held;
}

auto Input::keyUp(Key key) -> std::optional<unsigned> {
  if (auto state = m_keyStates.ptr(key)) {
    if (state->releases)
      return state->releases;
  }
  return {};
}

auto Input::mouseDown(MouseButton button) -> std::optional<List<Vec2F>> {
  if (auto state = m_mouseStates.ptr(button)) {
    if (state->presses)
      return state->pressPositions;
  }
  return {};
}

auto Input::mouseHeld(MouseButton button) -> bool {
  auto state = m_mouseStates.ptr(button);
  return state && state->held;
}

auto Input::mouseUp(MouseButton button) -> std::optional<List<Vec2F>> {
  if (auto state = m_mouseStates.ptr(button)) {
    if (state->releases)
      return state->releasePositions;
  }
  return {};
}

auto Input::mousePosition() const -> Vec2F {
  return m_mousePosition;
}

void Input::resetBinds(String const& categoryId, String const& bindId) {
  auto& entry = bindEntry(categoryId, bindId);

  entry.customBinds = entry.defaultBinds;
  entry.updated();
}

auto Input::getDefaultBinds(String const& categoryId, String const& bindId) -> Json {
  JsonArray array;
  for (Bind const& bind : bindEntry(categoryId, bindId).defaultBinds)
    array.emplace_back(bindToJson(bind));

  return array;
}

auto Input::getBinds(String const& categoryId, String const& bindId) -> Json {
  JsonArray array;
  for (Bind const& bind : bindEntry(categoryId, bindId).customBinds)
    array.emplace_back(bindToJson(bind));

  return array;
}

void Input::setBinds(String const& categoryId, String const& bindId, Json const& jBinds) {
  auto& entry = bindEntry(categoryId, bindId);

  List<Bind> binds;
  for (Json const& jBind : jBinds.toArray())
    binds.emplace_back(bindFromJson(jBind));

  entry.customBinds = std::move(binds);
  entry.updated();
}

auto Input::getTag(String const& tagName) const -> unsigned {
  if (auto tag = m_activeTags.ptr(tagName))
    return *tag;
  else
    return 0;
}

Input::ClipboardUnlock::ClipboardUnlock(Input& input)
    : m_input(&input) { ++m_input->m_clipboardAllowed; };

Input::ClipboardUnlock::ClipboardUnlock(ClipboardUnlock&& other) { m_input = take(other.m_input); };

Input::ClipboardUnlock::~ClipboardUnlock() {
  if (m_input)
    --m_input->m_clipboardAllowed;
};

auto Input::unlockClipboard() -> Input::ClipboardUnlock {
  return {*this};
}

auto Input::clipboardAllowed() const -> bool {
  return m_clipboardAllowed > 0 ? true : getTag("clipboard") > 0;
}

}// namespace Star
