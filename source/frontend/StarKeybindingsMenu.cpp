#include "StarKeybindingsMenu.hpp"
#include "StarAlgorithm.hpp"
#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarException.hpp"
#include "StarGuiReader.hpp"
#include "StarListWidget.hpp"
#include "StarLabelWidget.hpp"
#include "StarButtonWidget.hpp"
#include "StarOrderedSet.hpp"
#include "StarJsonExtra.hpp"

namespace Star {

KeybindingsMenu::KeybindingsMenu(KeybindingsMenuServices services)
  : Pane(services.guiContext),
    m_activeKeybinding(nullptr),
    m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "KeybindingsMenu", "assets")),
    m_configuration(requireServiceValueAs<StarException>(std::move(services.configuration), "KeybindingsMenu", "configuration")) {
  GuiReader reader(context());
  reader.registerCallback("cancel",
      [&](Widget*) {
        revert();
        dismiss();
      });
  reader.registerCallback("accept",
      [&](Widget*) {
        apply();
        dismiss();
      });
  reader.registerCallback("setDefault", [&](Widget*) { resetDefaults(); });

  m_maxBindings = m_assets->json("/interface/windowconfig/keybindingsmenu.config:maxBindings").toUInt();

  Json paneLayout = m_assets->json("/interface/windowconfig/keybindingsmenu.config:paneLayout");
  reader.construct(paneLayout, this);

  buildListsFromConfig();

  m_currentMods = KeyMod::NoMod;
}

KeyboardCaptureMode KeybindingsMenu::keyboardCaptureMode() const {
  return m_activeKeybinding ? KeyboardCaptureMode::KeyEvents : KeyboardCaptureMode::None;
}

bool KeybindingsMenu::sendEvent(InputEvent const& event) {
  if (!m_visible)
    return false;

  if (m_activeKeybinding) {
    if (context().actions(event).contains(InterfaceAction::KeybindingClear)) {
      clearActive();
      return true;
    }

    if (context().actions(event).contains(InterfaceAction::KeybindingCancel)) {
      exitActiveMode();
      return true;
    }
  }

  if (m_activeKeybinding) {
    // HACK: I need to pass events only to the trash button first.
    if (m_activeKeybinding->parent()->fetchChild<ButtonWidget>("deleteBinding")->sendEvent(event))
      return true;

    if (auto keyUp = event.ptr<KeyUpEvent>()) {
      if (Maybe<KeyMod> modKey = KeyChordMods.maybe(keyUp->key)) {
        m_currentMods &= ~*modKey;
        setKeybinding(KeyChord{keyUp->key, m_currentMods});
        return true;
      }
    } else if (auto keyDown = event.ptr<KeyDownEvent>()) {
      Maybe<KeyMod> modKey = KeyModNames.maybeLeft(KeyNames.getRight(keyDown->key));

      if (modKey) {
        m_currentMods |= *modKey;
        return true;
      } else {
        setKeybinding(KeyChord{keyDown->key, m_currentMods});
        return true;
      }
    }
  }

  if (context().actions(event).contains(InterfaceAction::GuiClose)) {
    dismiss();
    return true;
  }

  if (Pane::sendEvent(event))
    return true;

  return false;
}

void KeybindingsMenu::show() {
  m_origConfiguration = m_configuration->get("bindings");
  Pane::show();
}

void KeybindingsMenu::dismissed() {
  exitActiveMode();
  Pane::dismissed();
}

void KeybindingsMenu::buildListsFromConfig() {
  m_playerList = fetchChild<ListWidget>("categories.tabs.player.scrollArea.keyList");
  m_toolBarList = fetchChild<ListWidget>("categories.tabs.toolbar.scrollArea.keyList");
  m_gameList = fetchChild<ListWidget>("categories.tabs.game.scrollArea.keyList");

  m_childToAction.clear();

  auto doKeybindingsFor = [&](ListWidgetPtr const& list, Json const& keybinds) {
    list->clear();

    list->registerMemberCallback("activateBinding", [this](Widget* widget) { activateBinding(widget); });

    list->registerMemberCallback("deleteBinding", [this](Widget*) { clearActive(); });

    auto bindings = m_configuration->get("bindings");

    for (auto const& keybind : keybinds.iterateArray()) {
      auto newListMember = list->addItem();
      auto actionString = keybind.get("action").toString();
      auto action = InterfaceActionNames.getLeft(actionString);
      List<KeyChord> inputDesc;
      try {
        for (auto const& bindingEntry : bindings.get(actionString).iterateArray())
          inputDesc.append(inputDescriptorFromJson(bindingEntry));
      } catch (StarException const& e) {
        Logger::warn("Could not load keybinding for {}. {}\n", actionString, e.what());
      }

      m_childToAction.insert({newListMember->fetchChild<ButtonWidget>("boundKeys").get(), action});
      newListMember->fetchChild<LabelWidget>("actionName")->setText(keybind.getString("label"));
      newListMember->fetchChild<ButtonWidget>("boundKeys")->setText(StringList(inputDesc.transformed(printInputDescriptor)).join(", "));
      newListMember->fetchChild<ButtonWidget>("deleteBinding")->hide();
    }
  };

  doKeybindingsFor(m_playerList, m_assets->json("/interface/windowconfig/keybindingsmenu.config:keyActions.player"));
  doKeybindingsFor(m_toolBarList, m_assets->json("/interface/windowconfig/keybindingsmenu.config:keyActions.toolbar"));
  doKeybindingsFor(m_gameList, m_assets->json("/interface/windowconfig/keybindingsmenu.config:keyActions.game"));
}

bool KeybindingsMenu::activateBinding(Widget* widget) {
  exitActiveMode();

  m_activeKeybinding = widget;
  m_activeKeybinding->parent()->fetchChild<ButtonWidget>("deleteBinding")->show();
  convert<ButtonWidget>(m_activeKeybinding)->setHighlighted(true);

  return false;
}

void KeybindingsMenu::setKeybinding(KeyChord desc) {
  if (!m_activeKeybinding)
    return;

  auto out = inputDescriptorToJson(desc);

  auto base = m_configuration->get("bindings");

  auto action = m_childToAction.get(m_activeKeybinding);
  auto key = InterfaceActionNames.getRight(action);

  auto bindings = OrderedHashSet<Json>::from(base.get(key).toArray());

  if (bindings.contains(out))
    bindings.clear();

  bindings.add(out);

  if (bindings.size() > m_maxBindings)
    bindings.removeFirst();

  base = base.set(key, JsonArray::from(bindings));

  m_configuration->set("bindings", base);

  StringList buttonText;

  for (auto const& entry : base.get(key).iterateArray()) {
    try {
      auto stored = inputDescriptorFromJson(entry);
      buttonText.push_back(printInputDescriptor(stored));
    } catch (StarException const& e) {
      buttonText.push_back("unknown");
    }
  }

  convert<ButtonWidget>(m_activeKeybinding)->setText(buttonText.join(", "));

  apply();
  exitActiveMode();
}

void KeybindingsMenu::clearActive() {
  if (!m_activeKeybinding)
    return;

  auto base = m_configuration->get("bindings").toObject();

  auto action = m_childToAction.get(m_activeKeybinding);
  auto key = InterfaceActionNames.getRight(action);

  base[key] = JsonArray{};
  m_configuration->set("bindings", base);

  convert<ButtonWidget>(m_activeKeybinding)->setText("<Unbound>");

  apply();
  exitActiveMode();
}

void KeybindingsMenu::exitActiveMode() {
  if (!m_activeKeybinding)
    return;

  m_activeKeybinding->parent()->fetchChild<ButtonWidget>("deleteBinding")->hide();
  convert<ButtonWidget>(m_activeKeybinding)->setHighlighted(false);
  m_activeKeybinding = nullptr;
  m_currentMods = KeyMod::NoMod;
}

void KeybindingsMenu::apply() {
  context().refreshKeybindings();
}

void KeybindingsMenu::revert() {
  m_configuration->set("bindings", m_origConfiguration);
  apply();

  buildListsFromConfig();
}

void KeybindingsMenu::resetDefaults() {
  m_configuration->set("bindings", m_configuration->getDefault("bindings"));
  apply();

  buildListsFromConfig();
}

}
