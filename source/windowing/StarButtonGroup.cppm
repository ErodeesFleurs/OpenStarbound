module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
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

namespace Star {

STAR_CLASS(ButtonWidget);
STAR_CLASS(ButtonGroup);
STAR_CLASS(ButtonGroupWidget);

// Manages group of buttons in which *at most* a single button can be checked
// at any time.
class ButtonGroup {
public:
  friend class ButtonWidget;

  static int const NoButton = -1;

  // Callback is called when any child buttons checked state is changed, and
  // its parameter is the button being checked.
  void setCallback(WidgetCallbackFunc callback);

  ButtonWidget* button(int id) const;
  List<ButtonWidget*> buttons() const;
  size_t buttonCount() const;

  int addButton(ButtonWidget* button, int id = NoButton);
  void removeButton(ButtonWidget* button);

  int id(ButtonWidget* button) const;

  void select(int id);

  // Will return null if no button is checked.
  ButtonWidget* checkedButton() const;
  // Will return NoButton if no button is checked.
  int checkedId() const;

  // when true it is not required for one of the buttons to be selected
  bool toggle() const;
  void setToggle(bool toggleMode);

protected:
  // Should be called by child button widgets when they are changed from
  // unchecked to checked.
  void wasChecked(ButtonWidget* self);

private:
  WidgetCallbackFunc m_callback;
  Map<int, ButtonWidget*> m_buttons;
  Map<ButtonWidget*, int> m_buttonIds;
  bool m_toggle;
};

class ButtonGroupWidget : public ButtonGroup, public Widget {};
}

export module star.button_group;

export namespace Star {
  using ::Star::ButtonGroup;
  using ::Star::ButtonGroupWidget;
  using ::Star::ButtonWidget;
  using ::Star::ButtonWidgetPtr;
  using ::Star::ButtonWidgetConstPtr;
  using ::Star::ButtonWidgetWeakPtr;
  using ::Star::ButtonWidgetConstWeakPtr;
  using ::Star::ButtonWidgetUPtr;
  using ::Star::ButtonWidgetConstUPtr;
  using ::Star::ButtonGroupPtr;
  using ::Star::ButtonGroupConstPtr;
  using ::Star::ButtonGroupWeakPtr;
  using ::Star::ButtonGroupConstWeakPtr;
  using ::Star::ButtonGroupUPtr;
  using ::Star::ButtonGroupConstUPtr;
  using ::Star::ButtonGroupWidgetPtr;
  using ::Star::ButtonGroupWidgetConstPtr;
  using ::Star::ButtonGroupWidgetWeakPtr;
  using ::Star::ButtonGroupWidgetConstWeakPtr;
  using ::Star::ButtonGroupWidgetUPtr;
  using ::Star::ButtonGroupWidgetConstUPtr;
}
