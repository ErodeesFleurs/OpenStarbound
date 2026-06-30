#pragma once

#include "StarWidget.hpp"

namespace Star {

class ButtonWidget;
using ButtonWidgetPtr = SharedPtr<ButtonWidget>;
class ButtonGroup;
using ButtonGroupPtr = SharedPtr<ButtonGroup>;
class ButtonGroupWidget;
using ButtonGroupWidgetPtr = SharedPtr<ButtonGroupWidget>;

// Manages group of buttons in which *at most* a single button can be checked
// at any time.
class ButtonGroup {
public:
  friend class ButtonWidget;

  static constexpr int NoButton = -1;

  ButtonGroup() = default;

  // Callback is called when any child buttons checked state is changed, and
  // its parameter is the button being checked.
  void setCallback(WidgetCallbackFunc callback);

  [[nodiscard]] ButtonWidget* button(int id) const;
  [[nodiscard]] List<ButtonWidget*> buttons() const;
  [[nodiscard]] size_t buttonCount() const;

  [[nodiscard]] int addButton(ButtonWidget* button, int id = NoButton);
  void removeButton(ButtonWidget* button);

  [[nodiscard]] int id(ButtonWidget* button) const;

  void select(int id);

  // Will return null if no button is checked.
  [[nodiscard]] ButtonWidget* checkedButton() const;
  // Will return NoButton if no button is checked.
  [[nodiscard]] int checkedId() const;

  // when true it is not required for one of the buttons to be selected
  [[nodiscard]] bool toggle() const;
  void setToggle(bool toggleMode);

protected:
  // Should be called by child button widgets when they are changed from
  // unchecked to checked.
  void wasChecked(ButtonWidget* self);

private:
  WidgetCallbackFunc m_callback;
  Map<int, ButtonWidget*> m_buttons;
  Map<ButtonWidget*, int> m_buttonIds;
  bool m_toggle = false;
};

class ButtonGroupWidget : public ButtonGroup, public Widget {
public:
  explicit ButtonGroupWidget(GuiContext& context);
};
}
