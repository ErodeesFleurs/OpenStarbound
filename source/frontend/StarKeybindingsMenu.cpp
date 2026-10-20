import std;
import star.json;
import star.json_extra;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.format;
import star.version;
import star.ordered_set;


import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.variant;
import star.renderer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.root;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.pane;


import star.keybindings_menu;
import star.configuration;
import star.widget_parsing;
import star.gui_reader;
import star.list_widget;
import star.label_widget;
import star.button_group;
import star.button_widget;

namespace Star {

KeybindingsMenu::KeybindingsMenu(GuiContext& guiContext) : Pane(guiContext), m_activeKeybinding(nullptr) {
  GuiReader reader(*context());
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

  auto assets = context()->root().assets();

  m_maxBindings = assets->json("/interface/windowconfig/keybindingsmenu.config:maxBindings").toUInt();

  Json paneLayout = assets->json("/interface/windowconfig/keybindingsmenu.config:paneLayout");
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
    if (m_context->actions(event).contains(InterfaceAction::KeybindingClear)) {
      clearActive();
      return true;
    }

    if (m_context->actions(event).contains(InterfaceAction::KeybindingCancel)) {
      exitActiveMode();
      return true;
    }
  }

  if (m_activeKeybinding) {
    // HACK: I need to pass events only to the trash button first.
    if (m_activeKeybinding->parent()->fetchChild<ButtonWidget>("deleteBinding")->sendEvent(event))
      return true;

    if (auto keyUp = event.ptr<KeyUpEvent>()) {
      if (auto modKey = std::ranges::find(KeyChordMods, keyUp->key, &std::pair<Key, KeyMod>::first);
          modKey != KeyChordMods.end()) {
        m_currentMods &= ~modKey->second;
        setKeybinding(KeyChord{keyUp->key, m_currentMods});
        return true;
      }
    } else if (auto keyDown = event.ptr<KeyDownEvent>()) {
      std::optional<KeyMod> modKey = KeyModNames.maybeLeft(KeyNames.getRight(keyDown->key));

      if (modKey) {
        m_currentMods |= *modKey;
        return true;
      } else {
        setKeybinding(KeyChord{keyDown->key, m_currentMods});
        return true;
      }
    }
  }

  if (m_context->actions(event).contains(InterfaceAction::GuiClose)) {
    dismiss();
    return true;
  }

  if (Pane::sendEvent(event))
    return true;

  return false;
}

void KeybindingsMenu::show() {
  m_origConfiguration = context()->root().configuration()->get("bindings");
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

  auto doKeybindingsFor = [&](std::shared_ptr<ListWidget> const& list, Json const& keybinds) {
    list->clear();

    list->registerMemberCallback("activateBinding", [this](Widget* widget) { activateBinding(widget); });

    list->registerMemberCallback("deleteBinding", [this](Widget*) { clearActive(); });

    auto config = context()->root().configuration();
    auto bindings = config->get("bindings");

    for (auto const& keybind : keybinds.iterateArray()) {
      auto newListMember = list->addItem();
      auto actionString = keybind.get("action").toString();
      auto action = InterfaceActionNames.getLeft(actionString);
      List<KeyChord> inputDesc;
      try {
        for (auto const& bindingEntry : bindings.get(actionString).iterateArray())
          inputDesc.append(inputDescriptorFromJson(bindingEntry));
      } catch (StarException const& e) {
        context()->root().logger().warn("Could not load keybinding for {}. {}\n", actionString, e.what());
      }

      m_childToAction.insert({newListMember->fetchChild<ButtonWidget>("boundKeys").get(), action});
      newListMember->fetchChild<LabelWidget>("actionName")->setText(keybind.getString("label"));
      newListMember->fetchChild<ButtonWidget>("boundKeys")->setText(StringList(inputDesc.transformed(printInputDescriptor)).join(", "));
      newListMember->fetchChild<ButtonWidget>("deleteBinding")->hide();
    }
  };

  auto assets = context()->root().assets();
  doKeybindingsFor(m_playerList, assets->json("/interface/windowconfig/keybindingsmenu.config:keyActions.player"));
  doKeybindingsFor(m_toolBarList, assets->json("/interface/windowconfig/keybindingsmenu.config:keyActions.toolbar"));
  doKeybindingsFor(m_gameList, assets->json("/interface/windowconfig/keybindingsmenu.config:keyActions.game"));
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

  auto config = context()->root().configuration();
  auto base = config->get("bindings");

  auto action = m_childToAction.get(m_activeKeybinding);
  auto key = InterfaceActionNames.getRight(action);

  auto bindings = OrderedHashSet<Json>::from(base.get(key).toArray());

  if (bindings.contains(out))
    bindings.clear();

  bindings.add(out);

  if (bindings.size() > m_maxBindings)
    bindings.removeFirst();

  base = base.set(key, JsonArray::from(bindings));

  config->set("bindings", base);

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

  auto config = context()->root().configuration();
  auto base = config->get("bindings").toObject();

  auto action = m_childToAction.get(m_activeKeybinding);
  auto key = InterfaceActionNames.getRight(action);

  base[key] = JsonArray{};
  config->set("bindings", base);

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
  m_context->refreshKeybindings();
}

void KeybindingsMenu::revert() {
  context()->root().configuration()->set("bindings", m_origConfiguration);
  apply();

  buildListsFromConfig();
}

void KeybindingsMenu::resetDefaults() {
  auto config = context()->root().configuration();
  config->set("bindings", config->getDefault("bindings"));
  apply();

  buildListsFromConfig();
}

}
