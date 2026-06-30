#pragma once

#include "StarObserverPtr.hpp"
#include "StarInputEvent.hpp"
#include "StarJson.hpp"
#include "StarListener.hpp"
#include "StarHash.hpp"
#include "StarAssets.hpp"

namespace Star {

class Configuration;
using ConfigurationPtr = SharedPtr<Configuration>;

class Input;
using InputPtr = SharedPtr<Input>;
struct InputExceptionTag { static constexpr char const* typeName = "InputException"; };
using InputException = TypedException<StarException, InputExceptionTag>;

struct InputServices {
  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  function<void(ListenerWeakPtr)> registerReloadListener;
};

using InputVariant = Variant<Key, MouseButton, ControllerButton>;

template <>
struct hash<InputVariant> {
  [[nodiscard]] size_t operator()(InputVariant const& v) const;
};

class Input {
public:

  [[nodiscard]] static Json inputEventToJson(InputEvent const& event);

  struct KeyBind {
    Key key = Key::Zero;
    KeyMod mods = KeyMod::NoMod;
    uint8_t priority = 0;

    inline bool operator<(KeyBind const& rhs) const {
      return priority < rhs.priority;
    }

    inline bool operator>(KeyBind const& rhs) const {
      return priority > rhs.priority;
    }
  };

  struct MouseBind {
    MouseButton button = MouseButton::Left;
    KeyMod mods = KeyMod::NoMod;
    uint8_t priority = 0;
  };

  struct ControllerBind {
    unsigned int controller = 0;
    ControllerButton button = ControllerButton::Invalid;
  };

  using Bind = MVariant<KeyBind, MouseBind, ControllerBind>;

  [[nodiscard]] static Bind bindFromJson(Json const& json);
  [[nodiscard]] static Json bindToJson(Bind const& bind);

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
    uint8_t priority = 0;  
    BindEntry* entry = nullptr; // Invalidated on reload, careful!

    BindRef(BindEntry& bindEntry, KeyBind& keyBind);
    BindRef(BindEntry& bindEntry, MouseBind& mouseBind);
    BindRef(BindEntry& bindEntry);
  };

  struct BindCategory {
    String id;
    String name;
    Json config;
    ConfigurationPtr configuration;
    function<void()> rebuildMappings;

    StableHashMap<String, BindEntry> entries;

    BindCategory(String categoryId, Json const& categoryConfig, ConfigurationPtr configuration, function<void()> rebuildMappings);
  };

  struct InputState {
    unsigned presses = 0;
    unsigned releases = 0;
    bool pressed = false;
    bool held = false;
    bool released = false;

    // Calls the passed functions for each press and release.
    template <typename PressFunction, typename ReleaseFunction>
    void forEach(PressFunction&& onPress, ReleaseFunction&& onRelease) {
      for (size_t i = 0; i != releases; ++i) {
        onPress();
        onRelease();
      }
    }

    inline void reset() {
      presses = releases = 0;
      pressed = released = false;
    }

    inline void press() { pressed = ++presses; held = true; }
    inline void release() { released = ++releases; held = false; }
  };

  struct KeyInputState : InputState {
    KeyMod mods = KeyMod::NoMod;
  };

  struct MouseInputState : InputState {
    List<Vec2F> pressPositions;
    List<Vec2F> releasePositions;
  };

  using ControllerInputState = InputState;

  Input(InputServices services);
  ~Input();

  Input(Input const&) = delete;
  Input& operator=(Input const&) = delete;

  List<std::pair<InputEvent, bool>> const& inputEventsThisFrame() const;

  // Clears input state. Should be done at the very start or end of the client loop.
  void reset(bool clear = false);

  void update();

  // Handles an input event.
  [[nodiscard]] bool handleInput(InputEvent const& input, bool gameProcessed);

  void rebuildMappings();

  // Loads input categories and their binds from Assets.
  void reload();

  void setTextInputActive(bool active);

  [[nodiscard]] Maybe<unsigned> bindDown(String const& categoryId, String const& bindId);
  [[nodiscard]] bool            bindHeld(String const& categoryId, String const& bindId);
  [[nodiscard]] Maybe<unsigned> bindUp  (String const& categoryId, String const& bindId);

  [[nodiscard]] Maybe<unsigned> keyDown(Key key, Maybe<KeyMod> keyMod);
  [[nodiscard]] bool            keyHeld(Key key);
  [[nodiscard]] Maybe<unsigned> keyUp  (Key key);

  [[nodiscard]] Maybe<List<Vec2F>> mouseDown(MouseButton button);
  [[nodiscard]] bool               mouseHeld(MouseButton button);
  [[nodiscard]] Maybe<List<Vec2F>> mouseUp  (MouseButton button);

  [[nodiscard]] Vec2F mousePosition() const;

  void resetBinds(String const& categoryId, String const& bindId);
  void setBinds(String const& categoryId, String const& bindId, Json const& binds);
  [[nodiscard]] Json getDefaultBinds(String const& categoryId, String const& bindId); 
  [[nodiscard]] Json getBinds(String const& categoryId, String const& bindId);

  [[nodiscard]] unsigned getTag(String const& tagName) const;

  class ClipboardUnlock {
  public:
    ClipboardUnlock(Input& input);
    ClipboardUnlock(ClipboardUnlock const&) = delete;
    ClipboardUnlock(ClipboardUnlock&&);
    ~ClipboardUnlock();

  private:
    observer_ptr<Input> m_input;
  };

  [[nodiscard]] ClipboardUnlock unlockClipboard();
  [[nodiscard]] bool clipboardAllowed() const;
private:
  [[nodiscard]] List<BindEntry*> filterBindEntries(List<BindRef> const& binds, KeyMod mods) const;

  [[nodiscard]] BindEntry* bindEntryPtr(String const& categoryId, String const& bindId);
  [[nodiscard]] BindEntry& bindEntry(String const& categoryId, String const& bindId);

  [[nodiscard]] InputState* bindStatePtr(String const& categoryId, String const& bindId);

  [[nodiscard]] InputState& addBindState(BindEntry const& bindEntry);

  // Regenerated on reload.
  StableHashMap<String, BindCategory> m_bindCategories;
  // Contains raw pointers to bind entries in categories, so also regenerated on reload.
  HashMap<InputVariant, List<BindRef>> m_bindMappings;

  ListenerPtr m_rootReloadListener;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;

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

}
