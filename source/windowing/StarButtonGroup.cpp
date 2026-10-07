#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVersion.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"

#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
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
