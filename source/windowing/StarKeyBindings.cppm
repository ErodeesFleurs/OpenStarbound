module;

import std;
import star.format;
import star.json;
import star.string;
import star.list;
import star.map;
import star.bi_map;
import star.input_event;
import star.set;

namespace Star {

class Root;

enum class InterfaceAction {
  None,
  PlayerUp,
  PlayerDown,
  PlayerLeft,
  PlayerRight,
  PlayerJump,
  PlayerMainItem,
  PlayerAltItem,
  PlayerDropItem,
  PlayerInteract,
  PlayerShifting,
  PlayerTechAction1,
  PlayerTechAction2,
  PlayerTechAction3,
  EmoteBlabbering,
  EmoteShouting,
  EmoteHappy,
  EmoteSad,
  EmoteNeutral,
  EmoteLaugh,
  EmoteAnnoyed,
  EmoteOh,
  EmoteOooh,
  EmoteBlink,
  EmoteWink,
  EmoteEat,
  EmoteSleep,
  ShowLabels,
  CameraShift,
  TitleBack,
  CinematicSkip,
  CinematicNext,
  GuiClose,
  GuiShifting,
  KeybindingClear,
  KeybindingCancel,
  ChatPageUp,
  ChatPageDown,
  ChatPreviousLine,
  ChatNextLine,
  ChatSendLine,
  ChatBegin,
  ChatBeginCommand,
  ChatStop,
  InterfaceShowHelp,
  InterfaceHideHud,
  InterfaceChangeBarGroup,
  InterfaceDeselectHands,
  InterfaceBar1,
  InterfaceBar2,
  InterfaceBar3,
  InterfaceBar4,
  InterfaceBar5,
  InterfaceBar6,
  InterfaceBar7,
  InterfaceBar8,
  InterfaceBar9,
  InterfaceBar10,
  EssentialBar1,
  EssentialBar2,
  EssentialBar3,
  EssentialBar4,
  InterfaceRepeatCommand,
  InterfaceToggleFullscreen,
  InterfaceReload,
  InterfaceEscapeMenu,
  InterfaceInventory,
  InterfaceCodex,
  InterfaceQuest,
  InterfaceCrafting,
};
extern std::expected<EnumMap<InterfaceAction>, StarException> const InterfaceActionNames;

// Modifier keys accepted in key chords; lock-state bits are excluded.
inline constexpr std::array<std::pair<Key, KeyMod>, 9> KeyChordMods{{
  {Key::LShift, KeyMod::LShift},
  {Key::RShift, KeyMod::RShift},
  {Key::LCtrl, KeyMod::LCtrl},
  {Key::RCtrl, KeyMod::RCtrl},
  {Key::LAlt, KeyMod::LAlt},
  {Key::RAlt, KeyMod::RAlt},
  {Key::LGui, KeyMod::LGui},
  {Key::RGui, KeyMod::RGui},
  {Key::AltGr, KeyMod::AltGr}
}};

struct KeyChord {
  Key key;
  KeyMod mods;

  // Was tie(key, mods) < tie(rhs.key, rhs.mods): memberwise order is the same.
  auto operator<=>(KeyChord const& rhs) const = default;
};

[[nodiscard]] std::expected<KeyChord, StarException> inputDescriptorFromJson(Json const& json);
[[nodiscard]] std::expected<Json, StarException> inputDescriptorToJson(KeyChord const& chord);

[[nodiscard]] std::expected<String, StarException> printInputDescriptor(KeyChord chord);

class KeyBindings {
public:
  KeyBindings();
  [[nodiscard]] static std::expected<KeyBindings, StarException> create(Root& root, Json const& json);

  Set<InterfaceAction> actions(Key key) const;
  Set<InterfaceAction> actions(InputEvent const& event) const;
  Set<InterfaceAction> actions(KeyChord chord) const;
  Set<InterfaceAction> actionsForKey(Key key) const;

private:
  // Maps the primary key to a list of InterfaceActions, and any mods that they
  // require to be held.
  Map<Key, List<std::pair<KeyMod, InterfaceAction>>> m_actions;
};

}

export module star.key_bindings;

export namespace Star {
  using ::Star::InterfaceAction;
  using ::Star::KeyChord;
  using ::Star::KeyBindings;
  using ::Star::InterfaceActionNames;
  using ::Star::KeyChordMods;
  using ::Star::inputDescriptorFromJson;
  using ::Star::inputDescriptorToJson;
  using ::Star::printInputDescriptor;
}
