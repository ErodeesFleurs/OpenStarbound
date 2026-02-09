#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarHash.hpp"
#include "StarInputEvent.hpp"
#include "StarJson.hpp"
#include "StarListener.hpp"

import std;

namespace Star {

using InputException = ExceptionDerived<"InputException">;

using InputVariant = Variant<Key, MouseButton, ControllerButton>;

template <>
struct hash<InputVariant> {
  auto operator()(InputVariant const& v) const -> std::size_t;
};

class Input {
public:
  static auto inputEventToJson(InputEvent const& event) -> Json;

  struct KeyBind {
    Key key = Key::Zero;
    KeyMod mods = KeyMod::NoMod;
    std::uint8_t priority = 0;

    inline auto operator<(KeyBind const& rhs) const -> bool {
      return priority < rhs.priority;
    }

    inline auto operator>(KeyBind const& rhs) const -> bool {
      return priority > rhs.priority;
    }
  };

  struct MouseBind {
    MouseButton button = MouseButton::Left;
    KeyMod mods = KeyMod::NoMod;
    std::uint8_t priority = 0;
  };

  struct ControllerBind {
    unsigned int controller = 0;
    ControllerButton button = ControllerButton::Invalid;
  };

  using Bind = MVariant<KeyBind, MouseBind, ControllerBind>;

  static auto bindFromJson(Json const& json) -> Bind;
  static auto bindToJson(Bind const& bind) -> Json;

  struct BindCategory;

  struct BindEntry {
    // The internal ID of this entry.
    String id;
    // The user-facing name of this entry.
    String name;
    // The category this entry belongs to.
    BindCategory const* category;
    // Associated string tags that become active when this bind is pressed.
    StringList tags;

    // The default binds.
    List<Bind> defaultBinds;
    // The user-configured binds.
    List<Bind> customBinds;

    BindEntry(String entryId, Json const& config, BindCategory const& parentCategory);
    void updated();
  };

  struct BindRef {
    KeyMod mods;
    std::uint8_t priority = 0;
    BindEntry* entry = nullptr;// Invalidated on reload, careful!

    BindRef(BindEntry& bindEntry, KeyBind& keyBind);
    BindRef(BindEntry& bindEntry, MouseBind& mouseBind);
    BindRef(BindEntry& bindEntry);
  };

  struct BindCategory {
    String id;
    String name;
    Json config;

    StableHashMap<String, BindEntry> entries;

    BindCategory(String categoryId, Json const& categoryConfig);
  };

  struct InputState {
    unsigned presses = 0;
    unsigned releases = 0;
    bool pressed = false;
    bool held = false;
    bool released = false;

    // Calls the passed functions for each press and release.
    template <typename PressFunction, typename ReleaseFunction>
    void forEach(PressFunction&& pressed, ReleaseFunction&& released) {
      for (std::size_t i = 0; i != releases; ++i) {
        pressed();
        released();
      }
    }

    inline void reset() {
      presses = releases = 0;
      pressed = released = false;
    }

    inline void press() {
      pressed = ++presses;
      held = true;
    }
    inline void release() {
      released = ++releases;
      held = false;
    }
  };

  struct KeyInputState : InputState {
    KeyMod mods = KeyMod::NoMod;
  };

  struct MouseInputState : InputState {
    List<Vec2F> pressPositions;
    List<Vec2F> releasePositions;
  };

  using ControllerInputState = InputState;

  // Get pointer to the singleton Input instance, if it exists.  Otherwise,
  // returns nullptr.
  static auto singletonPtr() -> Input*;

  // Gets reference to Input singleton, throws InputException if root
  // is not initialized.
  static auto singleton() -> Input&;

  Input();
  ~Input();

  Input(Input const&) = delete;
  auto operator=(Input const&) -> Input& = delete;

  [[nodiscard]] auto inputEventsThisFrame() const -> List<std::pair<InputEvent, bool>> const&;

  // Clears input state. Should be done at the very start or end of the client loop.
  void reset(bool clear = false);

  void update();

  // Handles an input event.
  auto handleInput(InputEvent const& input, bool gameProcessed) -> bool;

  void rebuildMappings();

  // Loads input categories and their binds from Assets.
  void reload();

  void setTextInputActive(bool active);

  auto bindDown(String const& categoryId, String const& bindId) -> std::optional<unsigned>;
  auto bindHeld(String const& categoryId, String const& bindId) -> bool;
  auto bindUp(String const& categoryId, String const& bindId) -> std::optional<unsigned>;

  auto keyDown(Key key, std::optional<KeyMod> keyMod) -> std::optional<unsigned>;
  auto keyHeld(Key key) -> bool;
  auto keyUp(Key key) -> std::optional<unsigned>;

  auto mouseDown(MouseButton button) -> std::optional<List<Vec2F>>;
  auto mouseHeld(MouseButton button) -> bool;
  auto mouseUp(MouseButton button) -> std::optional<List<Vec2F>>;

  [[nodiscard]] auto mousePosition() const -> Vec2F;

  void resetBinds(String const& categoryId, String const& bindId);
  void setBinds(String const& categoryId, String const& bindId, Json const& binds);
  auto getDefaultBinds(String const& categoryId, String const& bindId) -> Json;
  auto getBinds(String const& categoryId, String const& bindId) -> Json;

  [[nodiscard]] auto getTag(String const& tagName) const -> unsigned;

  class ClipboardUnlock {
  public:
    ClipboardUnlock(Input& input);
    ClipboardUnlock(ClipboardUnlock const&) = delete;
    ClipboardUnlock(ClipboardUnlock&&);
    ~ClipboardUnlock();

  private:
    Input* m_input;
  };

  auto unlockClipboard() -> ClipboardUnlock;
  [[nodiscard]] auto clipboardAllowed() const -> bool;

private:
  [[nodiscard]] auto filterBindEntries(List<BindRef> const& binds, KeyMod mods) const -> List<BindEntry*>;

  auto bindEntryPtr(String const& categoryId, String const& bindId) -> BindEntry*;
  auto bindEntry(String const& categoryId, String const& bindId) -> BindEntry&;

  auto bindStatePtr(String const& categoryId, String const& bindId) -> InputState*;

  auto addBindState(BindEntry const* bindEntry) -> InputState&;

  static Input* s_singleton;

  // Regenerated on reload.
  StableHashMap<String, BindCategory> m_bindCategories;
  // Contains raw pointers to bind entries in categories, so also regenerated on reload.
  HashMap<InputVariant, List<BindRef>> m_bindMappings;

  Ptr<Listener> m_rootReloadListener;

  // Per-frame input event storage for Lua.
  List<std::pair<InputEvent, bool>> m_inputEvents;

  // Per-frame input state maps.
  //Input states
  HashMap<Key, KeyInputState> m_keyStates;
  HashMap<MouseButton, MouseInputState> m_mouseStates;
  HashMap<ControllerButton, ControllerInputState> m_controllerStates;
  //Bind states
  HashMap<BindEntry const*, InputState> m_bindStates;
  StringMap<unsigned> m_activeTags;

  KeyMod m_pressedMods;
  bool m_textInputActive;
  Vec2F m_mousePosition;

  unsigned m_clipboardAllowed = 0;
};

}// namespace Star
