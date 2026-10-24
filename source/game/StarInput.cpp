import star.hash;
import star.json;
import star.json_extra;
import star.input_event;
import star.thread;
import star.listener;
import star.ordered_map;
import star.bi_map;
import star.io_device;
import star.list;
import star.image_processing;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import std;
import star.string;
import star.format;
import star.version;

import star.input;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.vector;
import star.algorithm;
namespace Star {

const char* InputBindingConfigRoot = "modBindings";

std::expected<BiMap<Key, KeyMod>, StarException> const KeysToMods = BiMap<Key, KeyMod>::create({
  {Key::LShift, KeyMod::LShift},
  {Key::RShift, KeyMod::RShift},
  {Key::LCtrl, KeyMod::LCtrl},
  {Key::RCtrl, KeyMod::RCtrl},
  {Key::LAlt, KeyMod::LAlt},
  {Key::RAlt, KeyMod::RAlt},
  {Key::LGui, KeyMod::LGui},
  {Key::RGui, KeyMod::RGui},
  {Key::AltGr, KeyMod::AltGr},
  {Key::ScrollLock, KeyMod::Scroll}
});

const KeyMod KeyModOptional = KeyMod::Num | KeyMod::Caps | KeyMod::Scroll;

inline bool compareKeyModLenient(KeyMod input, KeyMod test) {
	input |= KeyModOptional;
	test |= KeyModOptional;
	return (test & input) == test;
}

inline bool compareKeyMod(KeyMod input, KeyMod test) {
	return (input | (KeyModOptional & ~test)) == (test | KeyModOptional);
}

Json keyModsToJson(KeyMod mod) {
  JsonArray array;
  
  if (bool(mod & KeyMod::LShift)) array.emplace_back("LShift");
  if (bool(mod & KeyMod::RShift)) array.emplace_back("RShift");
  if (bool(mod & KeyMod::LCtrl )) array.emplace_back("LCtrl" );
  if (bool(mod & KeyMod::RCtrl )) array.emplace_back("RCtrl" );
  if (bool(mod & KeyMod::LAlt  )) array.emplace_back("LAlt"  );
  if (bool(mod & KeyMod::RAlt  )) array.emplace_back("RAlt"  );
  if (bool(mod & KeyMod::LGui  )) array.emplace_back("LGui"  );
  if (bool(mod & KeyMod::RGui  )) array.emplace_back("RGui"  );
  if (bool(mod & KeyMod::Num   )) array.emplace_back("Num"   );
  if (bool(mod & KeyMod::Caps  )) array.emplace_back("Caps"  );
  if (bool(mod & KeyMod::AltGr )) array.emplace_back("AltGr" );
  if (bool(mod & KeyMod::Scroll)) array.emplace_back("Scroll");

  return array.empty() ? Json() : std::move(array);
}

// Optional pointer argument to output calculated priority
[[nodiscard]] std::expected<KeyMod, StarException> keyModsFromJson(Json const& json, std::uint8_t* priority = nullptr) {
  KeyMod mod = KeyMod::NoMod;
  if (!json.isType(Json::Type::Array))
    return mod;

  std::uint8_t modPriority = 0;
  auto array = json.toArray();
  if (!array) return std::unexpected(std::move(array.error()));
  for (Json const& jMod : *array) {
    auto text = jMod.toString();
    if (!text) return std::unexpected(std::move(text.error()));
    auto named = enumValue<KeyModNames>(*text);
    if (!named) return std::unexpected(std::move(named.error()));
    KeyMod changedMod = mod | *named;
    if (mod != changedMod) {
      mod = changedMod;
      ++modPriority;
    }
  }
  if (priority)
    *priority = modPriority;

  return mod;
}

std::size_t hash<InputVariant>::operator()(InputVariant const& v) const {
  std::size_t indexHash = hashOf(static_cast<std::uint8_t>(v.index()));
  if (auto key = std::get_if<Key>(&v))
    hashCombine(indexHash, hashOf(*key));
  else if (auto mButton = std::get_if<MouseButton>(&v))
    hashCombine(indexHash, hashOf(*mButton));
  else if (auto cButton = std::get_if<ControllerButton>(&v))
    hashCombine(indexHash, hashOf(*cButton));

  return indexHash;
}

std::expected<Json, StarException> Input::inputEventToJson(InputEvent const& input) {
  String type;
  Json data;

  if (auto keyDown = std::get_if<KeyDownEvent>(&input)) {
    type = "KeyDown";
    auto name = enumName<KeyNames>(keyDown->key);
    if (!name) return std::unexpected(std::move(name.error()));
    data = JsonObject{
      {"key", String(*name)},
      {"mods", keyModsToJson(keyDown->mods)}
    };
  } else if (auto keyUp = std::get_if<KeyUpEvent>(&input)) {
    type = "KeyUp";
    auto name = enumName<KeyNames>(keyUp->key);
    if (!name) return std::unexpected(std::move(name.error()));
    data = JsonObject{{"key", String(*name)}};
  } else if (auto mouseDown = std::get_if<MouseButtonDownEvent>(&input)) {
    type = "MouseButtonDown";
    auto name = enumName<MouseButtonNames>(mouseDown->mouseButton);
    if (!name) return std::unexpected(std::move(name.error()));
    data = JsonObject{
      {"mouseButton", String(*name)},
      {"mousePosition", jsonFromVec2F(mouseDown->mousePosition)}
    };
  } else if (auto mouseUp = std::get_if<MouseButtonUpEvent>(&input)) {
    type = "MouseButtonUp";
    auto name = enumName<MouseButtonNames>(mouseUp->mouseButton);
    if (!name) return std::unexpected(std::move(name.error()));
    data = JsonObject{
      {"mouseButton", String(*name)},
      {"mousePosition", jsonFromVec2F(mouseUp->mousePosition)}
    };
  } else if (auto mouseWheel = std::get_if<MouseWheelEvent>(&input)) {
    type = "MouseWheel";
    data = JsonObject{
      {"mouseWheel", mouseWheel->mouseWheel == MouseWheel::Up ? 1 : -1},
      {"mousePosition", jsonFromVec2F(mouseWheel->mousePosition)}
    };
  } else if (auto mouseMove = std::get_if<MouseMoveEvent>(&input)) {
    type = "MouseMove";
    data = JsonObject{
      {"mouseMove", jsonFromVec2F(mouseMove->mouseMove)},
      {"mousePosition", jsonFromVec2F(mouseMove->mousePosition)}
    };
  } else if (auto controllerDown = std::get_if<ControllerButtonDownEvent>(&input)) {
    type = "ControllerButtonDown";
    auto name = enumName<ControllerButtonNames>(controllerDown->controllerButton);
    if (!name) return std::unexpected(std::move(name.error()));
    data = JsonObject{
      {"controllerButton", String(*name)},
      {"controller", controllerDown->controller}};
  } else if (auto controllerUp = std::get_if<ControllerButtonUpEvent>(&input)) {
    type = "ControllerButtonUp";
    auto name = enumName<ControllerButtonNames>(controllerUp->controllerButton);
    if (!name) return std::unexpected(std::move(name.error()));
    data = JsonObject{
      {"controllerButton", String(*name)},
      {"controller", controllerUp->controller}};
  } else if (auto controllerAxis = std::get_if<ControllerAxisEvent>(&input)) {
    type = "ControllerAxis";
    auto name = enumName<ControllerAxisNames>(controllerAxis->controllerAxis);
    if (!name) return std::unexpected(std::move(name.error()));
    data = JsonObject{
      {"controllerAxis", String(*name)},
      {"controllerAxisValue", controllerAxis->controllerAxisValue},
      {"controller", controllerAxis->controller}};
  }

  if (data) {
    return JsonObject{
      {"type", type},
      {"data", data}
    };
  }

  return data;
}

std::expected<Input::Bind, StarException> Input::bindFromJson(Json const& json) {
  Bind bind;
  if (json.isNull())
    return bind;

  auto typeResult = json.getString("type");
  if (!typeResult) return std::unexpected(std::move(typeResult.error()));
  String const& type = *typeResult;
  auto valueResult = json.get("value", {});
  if (!valueResult) return std::unexpected(std::move(valueResult.error()));
  Json const& value = *valueResult;
  auto text = value.toString();
  if ((type == "key" || type == "mouse" || type == "controller") && !text)
    return std::unexpected(std::move(text.error()));

  if (type == "key") {
    KeyBind keyBind;
    auto key = maybeEnumValue<KeyNames>(*text);
    if (!key) return std::unexpected(std::move(key.error()));
    if (!*key) return bind;
    keyBind.key = **key;
    auto mods = json.getArray("mods", {});
    if (!mods) return std::unexpected(std::move(mods.error()));
    auto parsed = keyModsFromJson(Json(*mods), &keyBind.priority);
    if (!parsed) return std::unexpected(std::move(parsed.error()));
    keyBind.mods = *parsed;
    bind = std::move(keyBind);
  }
  else if (type == "mouse") {
    MouseBind mouseBind;
    auto button = maybeEnumValue<MouseButtonNames>(*text);
    if (!button) return std::unexpected(std::move(button.error()));
    if (!*button) return bind;
    mouseBind.button = **button;
    auto mods = json.getArray("mods", {});
    if (!mods) return std::unexpected(std::move(mods.error()));
    auto parsed = keyModsFromJson(Json(*mods), &mouseBind.priority);
    if (!parsed) return std::unexpected(std::move(parsed.error()));
    mouseBind.mods = *parsed;
    bind = std::move(mouseBind);
  }
  else if (type == "controller") {
    ControllerBind controllerBind;
    auto button = maybeEnumValue<ControllerButtonNames>(*text);
    if (!button) return std::unexpected(std::move(button.error()));
    if (!*button) return bind;
    controllerBind.button = **button;
    auto controller = json.getUInt("controller", 0);
    if (!controller) return std::unexpected(std::move(controller.error()));
    controllerBind.controller = *controller;
    bind = std::move(controllerBind);
  }

  return bind;
}

std::expected<Json, StarException> Input::bindToJson(Bind const& bind) {
  if (auto keyBind = std::get_if<KeyBind>(&bind)) {
    auto name = enumName<KeyNames>(keyBind->key);
    if (!name) return std::unexpected(std::move(name.error()));
    auto obj = JsonObject{
      {"type", "key"},
      {"value", String(*name)}
    }; // don't want empty mods to exist as null entry
    if (auto mods = keyModsToJson(keyBind->mods))
      obj.emplace("mods", std::move(mods));
    return obj;
  }
  else if (auto mouseBind = std::get_if<MouseBind>(&bind)) {
    auto name = enumName<MouseButtonNames>(mouseBind->button);
    if (!name) return std::unexpected(std::move(name.error()));
    auto obj = JsonObject{
      {"type", "mouse"},
      {"value", String(*name)}
    };
    if (auto mods = keyModsToJson(mouseBind->mods))
      obj.emplace("mods", std::move(mods));
    return obj;
  }
  else if (auto controllerBind = std::get_if<ControllerBind>(&bind)) {
    auto name = enumName<ControllerButtonNames>(controllerBind->button);
    if (!name) return std::unexpected(std::move(name.error()));
    return JsonObject{
      {"type", "controller"},
      {"value", String(*name)},
      {"controller", controllerBind->controller}
    };
  }

  return Json();
}

std::expected<Input::BindEntry, StarException> Input::BindEntry::create(Input& input, String entryId, Json const& config, BindCategory const& parentCategory) {
  BindEntry result(input);
  result.category = &parentCategory;
  result.id = std::move(entryId);
  auto name = config.getString("name", result.id);
  if (!name) return std::unexpected(std::move(name.error()));
  result.name = std::move(*name);
  auto tags = config.get("tags", JsonArray());
  if (!tags) return std::unexpected(std::move(tags.error()));
  auto tagList = jsonToStringList(*tags);
  if (!tagList) return std::unexpected(std::move(tagList.error()));
  result.tags = std::move(*tagList);
  auto defaults = config.getArray("default", {});
  if (!defaults) return std::unexpected(std::move(defaults.error()));
  for (Json const& jBind : *defaults) {
    auto bind = bindFromJson(jBind);
    if (!bind) {
      if (!bind.error().is<JsonException>()) return std::unexpected(std::move(bind.error()));
      auto logged = input.m_root.logger().error("Binds: Error loading default bind in {}.{}: {}", parentCategory.id, result.id, bind.error().what());
      if (!logged) return std::unexpected(std::move(logged.error()));
    } else result.defaultBinds.emplace_back(std::move(*bind));
  }
  return result;
}

std::expected<void, StarException> Input::BindEntry::updated() {
  auto config = owner.m_root.configuration();
  if (!config) return std::unexpected(std::move(config.error()));
  JsonArray array;
  for (auto const& bind : customBinds) {
    auto json = bindToJson(bind);
    if (!json) return std::unexpected(std::move(json.error()));
    array.emplace_back(std::move(*json));
  }
  auto root = (*config)->get(InputBindingConfigRoot);
  if (!root) return std::unexpected(std::move(root.error()));
  if (!root->isType(Json::Type::Object)) {
    if (auto r = (*config)->set(InputBindingConfigRoot, JsonObject()); !r) return std::unexpected(std::move(r.error()));
  }
  String path = strf("{}.{}", InputBindingConfigRoot, category->id);
  auto category = (*config)->getPath(path);
  if (!category) return std::unexpected(std::move(category.error()));
  if (!category->isType(Json::Type::Object)) {
    if (auto r = (*config)->setPath(path, JsonObject{{id, std::move(array)}}); !r) return std::unexpected(std::move(r.error()));
  } else {
    path = strf("{}.{}", path, id);
    if (auto r = (*config)->setPath(path, array); !r) return std::unexpected(std::move(r.error()));
  }
  owner.rebuildMappings();
  return {};
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

Input::BindCategory::BindCategory(Input& input) : owner(input) {}

std::expected<void, StarException> Input::BindCategory::mergeJson(String const& categoryId, Json const& categoryConfig) {
  id = categoryId;
  config = jsonMerge(config, categoryConfig);
  auto categoryName = config.getString("name", id);
  if (!categoryName) return std::unexpected(std::move(categoryName.error()));
  name = std::move(*categoryName);
  entries.clear();
  auto userConfig = owner.m_root.configuration();
  if (!userConfig) return std::unexpected(std::move(userConfig.error()));
  auto userBindings = (*userConfig)->get(InputBindingConfigRoot);
  if (!userBindings) return std::unexpected(std::move(userBindings.error()));
  auto configured = config.getObject("binds", {});
  if (!configured) return std::unexpected(std::move(configured.error()));
  for (auto const& pair : *configured) {
    String const& bindId = pair.first;
    Json const& bindConfig = pair.second;
    if (!bindConfig.isType(Json::Type::Object)) continue;
    auto built = BindEntry::create(owner, bindId, bindConfig, *this);
    if (!built) return std::unexpected(std::move(built.error()));
    auto& entry = entries.try_emplace(bindId, std::move(*built)).first->second;
    if (userBindings->isType(Json::Type::Object)) {
      auto bindings = userBindings->queryArray(strf("{}.{}", id, bindId), {});
      if (!bindings) return std::unexpected(std::move(bindings.error()));
      for (auto const& jBind : *bindings) {
        auto bind = bindFromJson(jBind);
        if (!bind) {
          if (!bind.error().is<JsonException>()) return std::unexpected(std::move(bind.error()));
          auto logged = owner.m_root.logger().error("Binds: Error loading user bind in {}.{}: {}", id, bindId, bind.error().what());
          if (!logged) return std::unexpected(std::move(logged.error()));
        } else entry.customBinds.emplace_back(std::move(*bind));
      }
    }
    if (entry.customBinds.empty()) entry.customBinds = entry.defaultBinds;
  }
  return {};
}

List<Input::BindEntry*> Input::filterBindEntries(List<Input::BindRef> const& binds, KeyMod mods) const {
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

Input::BindEntry* Input::bindEntryPtr(String const& categoryId, String const& bindId) {
  if (auto category = m_bindCategories.ptr(categoryId)) {
    if (auto entry = category->entries.ptr(bindId)) {
      return entry;
    }
  }
  
  return nullptr;
}

std::expected<Input::BindEntry*, StarException> Input::bindEntry(String const& categoryId, String const& bindId) {
  if (auto ptr = bindEntryPtr(categoryId, bindId))
    return ptr;
  else
    return std::unexpected(InputException::format("Could not find bind entry {}.{}", categoryId, bindId));
}

Input::InputState* Input::bindStatePtr(String const& categoryId, String const& bindId) {
  if (auto ptr = bindEntryPtr(categoryId, bindId))
    return m_bindStates.ptr(ptr);
  else
    return nullptr;
}

Input::InputState& Input::addBindState(BindEntry const* bindEntry) {
  auto insertion = m_bindStates.insert(bindEntry, InputState());
  if (insertion.second) {
    for (auto& tag : bindEntry->tags)
      ++m_activeTags[tag];
  }
  return insertion.first->second;
}

Input::Input(Root& root) : m_root(root), m_pressedMods(KeyMod::NoMod) {}

std::expected<std::shared_ptr<Input>, StarException> Input::create(Root& root) {
  auto result = std::shared_ptr<Input>(new Input(root));
  if (auto r = result->reload(); !r) return std::unexpected(std::move(r.error()));
  result->m_rootReloadListener = std::make_shared<CallbackListener>([ptr = result.get()]() {
    return ptr->reload();
  });
  result->m_rootReloadSubscription = root.registerReloadListener(result->m_rootReloadListener);
  return result;
}

Input::~Input() {
  m_rootReloadSubscription.disconnect();
}

List<std::pair<InputEvent, bool>> const& Input::inputEventsThisFrame() const {
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
    auto eraseCond = [](auto& p) {
      if (p.second.held)
        p.second.reset();
      return !p.second.held;
    };

    eraseWhere(m_keyStates,        eraseCond);
    eraseWhere(m_mouseStates,      eraseCond);
    eraseWhere(m_controllerStates, eraseCond);
    eraseWhere(m_bindStates, [&](auto& p) {
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
  reset(false);
}

std::expected<bool, StarException> Input::handleInput(InputEvent const& input, bool gameProcessed) {
  if (!KeysToMods) return std::unexpected(KeysToMods.error());
  m_inputEvents.emplace_back(input, gameProcessed);
  if (auto keyDown = std::get_if<KeyDownEvent>(&input)) {
    auto keyToMod = KeysToMods->rightPtr(keyDown->key);
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
  }
  else if (auto keyUp = std::get_if<KeyUpEvent>(&input)) {
    auto keyToMod = KeysToMods->rightPtr(keyUp->key);
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
  }
  else if (auto mouseDown = std::get_if<MouseButtonDownEvent>(&input)) {
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
  }
  else if (auto mouseUp = std::get_if<MouseButtonUpEvent>(&input)) {
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
  }
  else if (auto mouseMove = std::get_if<MouseMoveEvent>(&input)) {
    m_mousePosition = mouseMove->mousePosition;
  }
  else if (auto controllerDown = std::get_if<ControllerButtonDownEvent>(&input)) {
    if (!gameProcessed) {
      auto& state = m_controllerStates[controllerDown->controllerButton];
      state.press();

      if (auto binds = m_bindMappings.ptr(controllerDown->controllerButton)) {
        for (auto bind : filterBindEntries(*binds, m_pressedMods))
          addBindState(bind).press();
      }
    }
  }
  else if (auto controllerUp = std::get_if<ControllerButtonUpEvent>(&input)) {
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
        if (auto keyBind = std::get_if<KeyBind>(&bind))
          m_bindMappings[keyBind->key].emplace_back(entry, *keyBind);
        if (auto mouseBind = std::get_if<MouseBind>(&bind))
          m_bindMappings[mouseBind->button].emplace_back(entry, *mouseBind);
        if (auto controllerBind = std::get_if<ControllerBind>(&bind))
          m_bindMappings[controllerBind->button].emplace_back(entry);
      }
    }
  }

  for (auto& pair : m_bindMappings) {
    pair.second.sort([](BindRef const& a, BindRef const& b)
      { return a.priority > b.priority; });
  }
}

std::expected<void, StarException> Input::reload() {
  m_bindCategories.clear();
  auto assets = m_root.assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto paths = (*assets)->scanExtension("binds");
  if (!paths) return std::unexpected(std::move(paths.error()));
  for (auto const& bindPath : **paths) {
    auto json = (*assets)->json(bindPath.string());
    if (!json) return std::unexpected(std::move(json.error()));
    auto categories = json->toObject();
    if (!categories) return std::unexpected(std::move(categories.error()));
    for (auto const& pair : *categories) {
      if (!pair.second.isType(Json::Type::Object)) continue;
      auto r = m_bindCategories.try_emplace(pair.first, *this).first->second.mergeJson(pair.first, pair.second);
      if (!r) return std::unexpected(std::move(r.error()));
    }
  }
  std::size_t count = 0;
  for (auto const& pair : m_bindCategories) count += pair.second.entries.size();
  if (auto r = m_root.logger().info("Binds: Loaded {} bind{}", count, count == 1 ? "" : "s"); !r) return std::unexpected(std::move(r.error()));
  rebuildMappings();
  return {};
}

void Input::setTextInputActive(bool active) {
  m_textInputActive = active;
}

std::optional<unsigned> Input::bindDown(String const& categoryId, String const& bindId) {
  if (auto state = bindStatePtr(categoryId, bindId)) {
    if (state->presses)
      return state->presses;
  }
  return {};
}

bool Input::bindHeld(String const& categoryId, String const& bindId) {
  if (auto state = bindStatePtr(categoryId, bindId))
    return state->held;
  else
    return false;
}

std::optional<unsigned> Input::bindUp(String const& categoryId, String const& bindId) {
  if (auto state = bindStatePtr(categoryId, bindId)) {
    if (state->releases)
      return state->releases;
  }
  return {};
}

std::optional<unsigned> Input::keyDown(Key key, std::optional<KeyMod> keyMod) {
  if (auto state = m_keyStates.ptr(key)) {
    if (state->presses && (!keyMod || compareKeyMod(*keyMod, state->mods)))
      return state->presses;
  }
  return {};
}

bool Input::keyHeld(Key key) {
  auto state = m_keyStates.ptr(key);
  return state && state->held;
}

std::optional<unsigned> Input::keyUp(Key key) {
  if (auto state = m_keyStates.ptr(key)) {
    if (state->releases)
      return state->releases;
  }
  return {};
}

std::optional<List<Vec2F>> Input::mouseDown(MouseButton button) {
  if (auto state = m_mouseStates.ptr(button)) {
    if (state->presses)
      return state->pressPositions;
  }
  return {};
}

bool Input::mouseHeld(MouseButton button) {
  auto state = m_mouseStates.ptr(button);
  return state && state->held;
}

std::optional<List<Vec2F>> Input::mouseUp(MouseButton button) {
  if (auto state = m_mouseStates.ptr(button)) {
    if (state->releases)
      return state->releasePositions;
  }
  return {};
}

Vec2F Input::mousePosition() const {
  return m_mousePosition;
}

std::expected<void, StarException> Input::resetBinds(String const& categoryId, String const& bindId) {
  auto entry = bindEntry(categoryId, bindId);
  if (!entry) return std::unexpected(std::move(entry.error()));
  (*entry)->customBinds = (*entry)->defaultBinds;
  return (*entry)->updated();
}

std::expected<Json, StarException> Input::getDefaultBinds(String const& categoryId, String const& bindId) {
  auto entry = bindEntry(categoryId, bindId);
  if (!entry) return std::unexpected(std::move(entry.error()));
  JsonArray array;
  for (Bind const& bind : (*entry)->defaultBinds) {
    auto json = bindToJson(bind);
    if (!json) return std::unexpected(std::move(json.error()));
    array.emplace_back(std::move(*json));
  }
  return array;
}

std::expected<Json, StarException> Input::getBinds(String const& categoryId, String const& bindId) {
  auto entry = bindEntry(categoryId, bindId);
  if (!entry) return std::unexpected(std::move(entry.error()));
  JsonArray array;
  for (Bind const& bind : (*entry)->customBinds) {
    auto json = bindToJson(bind);
    if (!json) return std::unexpected(std::move(json.error()));
    array.emplace_back(std::move(*json));
  }
  return array;
}

std::expected<void, StarException> Input::setBinds(String const& categoryId, String const& bindId, Json const& jBinds) {
  auto entry = bindEntry(categoryId, bindId);
  if (!entry) return std::unexpected(std::move(entry.error()));
  auto array = jBinds.toArray();
  if (!array) return std::unexpected(std::move(array.error()));
  List<Bind> binds;
  for (Json const& jBind : *array) {
    auto bind = bindFromJson(jBind);
    if (!bind) return std::unexpected(std::move(bind.error()));
    binds.emplace_back(std::move(*bind));
  }
  (*entry)->customBinds = std::move(binds);
  return (*entry)->updated();
}

unsigned Input::getTag(String const& tagName) const {
  if (auto tag = m_activeTags.ptr(tagName))
    return *tag;
  else
    return 0;
}

Input::ClipboardUnlock::ClipboardUnlock(Input& input)
    : m_input(&input) { ++m_input->m_clipboardAllowed; };

Input::ClipboardUnlock::ClipboardUnlock(ClipboardUnlock&& other) { m_input = std::exchange(other.m_input, nullptr); };

Input::ClipboardUnlock::~ClipboardUnlock() { if (m_input) --m_input->m_clipboardAllowed; };


Input::ClipboardUnlock Input::unlockClipboard() {
  return Input::ClipboardUnlock(*this);
}

bool Input::clipboardAllowed() const {
  return m_clipboardAllowed > 0 ? true : getTag("clipboard") > 0;
}

}