import star.json;
#include "StarCasting.hpp"
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
#include "StarConfig.hpp"
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;

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
import star.configuration;
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
import star.button_group;
import star.button_widget;

namespace Star {

void ButtonGroup::setCallback(WidgetCallbackFunc callback) {
  m_callback = callback;
}

ButtonWidget* ButtonGroup::button(int id) const {
  return m_buttons.value(id);
}

List<ButtonWidget*> ButtonGroup::buttons() const {
  return m_buttons.values();
}

size_t ButtonGroup::buttonCount() const {
  return m_buttons.size();
}

int ButtonGroup::addButton(ButtonWidget* button, int id) {
  if (!button)
    return NoButton;
  else if (m_buttonIds.contains(button))
    return m_buttonIds.get(button);

  // If we are auto-generating an id, start at the last id and work forward.
  if (id == NoButton && !m_buttons.empty())
    id = (prev(m_buttons.end()))->first;

  while (m_buttons.contains(id))
    ++id;

  m_buttons[id] = button;
  m_buttonIds[button] = id;
  return id;
}

void ButtonGroup::removeButton(ButtonWidget* button) {
  if (m_buttonIds.contains(button)) {
    m_buttons.remove(m_buttonIds.get(button));
    m_buttonIds.remove(button);
  }
}

int ButtonGroup::id(ButtonWidget* button) const {
  if (m_buttonIds.contains(button))
    return m_buttonIds.get(button);
  else
    return NoButton;
}

ButtonWidget* ButtonGroup::checkedButton() const {
  for (auto const& pair : m_buttons) {
    if (pair.second->isChecked())
      return pair.second;
  }
  return {};
}

int ButtonGroup::checkedId() const {
  return id(checkedButton());
}

void ButtonGroup::select(int id) {
  auto b = button(id);
  if (!b->isChecked())
    b->check();
}

void ButtonGroup::wasChecked(ButtonWidget* self) {
  for (auto const& pair : m_buttons) {
    if (pair.second != self)
      pair.second->setChecked(false);
  }

  if (m_callback)
    m_callback(self);
}

bool ButtonGroup::toggle() const {
  return m_toggle;
}

void ButtonGroup::setToggle(bool toggleMode) {
  m_toggle = toggleMode;
}

}
