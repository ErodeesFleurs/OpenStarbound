module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
#include "StarRect.hpp"
import star.application_controller;
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarVariant.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarRefPtr.hpp"
import star.renderer;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
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
import star.button_group;

namespace Star {
STAR_CLASS(ButtonWidget);

class ButtonWidget : public Widget {
public:
  ButtonWidget();
  ButtonWidget(WidgetCallbackFunc callback,
      String const& baseImage,
      String const& hoverImage = "",
      String const& pressedImage = "",
      String const& disabledImage = "");
  virtual ~ButtonWidget();

  virtual bool sendEvent(InputEvent const& event) override;
  virtual void mouseOver() override;
  virtual void mouseOut() override;
  virtual void mouseReturnStillDown() override;
  virtual void hide() override;

  // Callback is called when the checked / pressed state is changed.
  void setCallback(WidgetCallbackFunc callback);

  ButtonGroupPtr buttonGroup() const;
  // Sets the button group for this widget, and adds it to the button group if
  // it is not already added.  Additionally, sets the button as checkable.
  void setButtonGroup(ButtonGroupPtr buttonGroup, int id = ButtonGroup::NoButton);
  // If a button group is set, returns this button's id in the button group.
  int buttonGroupId();

  bool isHovered() const;

  bool isPressed() const;
  void setPressed(bool pressed);

  bool isCheckable() const;
  void setCheckable(bool checkable);

  bool isHighlighted() const;
  void setHighlighted(bool highlighted);

  bool isChecked() const;
  void setChecked(bool checked);
  // Either checks a button, or toggles the state, depending on whether the
  // button is part of an exclusive group or not.
  void check();

  bool sustainCallbackOnDownHold();
  void setSustainCallbackOnDownHold(bool sustain);

  void setImages(String const& baseImage,
      String const& hoverImage = "",
      String const& pressedImage = "",
      String const& disabledImage = "");
  void setCheckedImages(String const& baseImage,
      String const& hoverImage = "",
      String const& pressedImage = "",
      String const& disabledImage = "");
  void setOverlayImage(String const& overlayImage = "");

  // Used to offset drawing when the button is being pressed / checked
  Vec2I const& pressedOffset() const;
  void setPressedOffset(Vec2I const& offset);

  virtual String const& getText() const;
  virtual void setText(String const& text);
  virtual void setFontSize(int size);
  virtual void setFontDirectives(String directives);
  virtual void setTextOffset(Vec2I textOffset);

  void setTextAlign(HorizontalAnchor hAnchor);
  void setFontColor(Color color);
  void setFontColorDisabled(Color color);
  void setFontColorChecked(Color color);

  virtual WidgetPtr getChildAt(Vec2I const& pos) override;

  void disable();
  void enable();
  void setEnabled(bool enabled);

  void setInvisible(bool invisible);

protected:
  virtual RectI getScissorRect() const override;
  virtual void renderImpl() override;

  void drawButtonPart(String const& image, Vec2F const& position);
  void updateSize();

  WidgetCallbackFunc m_callback;
  ButtonGroupWeakPtr m_buttonGroup;

  bool m_hovered;
  bool m_pressed;
  bool m_checkable;
  bool m_checked;

  bool m_disabled;
  bool m_highlighted;

  String m_baseImage;
  String m_hoverImage;
  String m_pressedImage;
  String m_disabledImage;

  bool m_hasCheckedImages;
  String m_baseImageChecked;
  String m_hoverImageChecked;
  String m_pressedImageChecked;
  String m_disabledImageChecked;

  String m_overlayImage;

  bool m_invisible;

  Vec2I m_pressedOffset;
  Vec2U m_buttonBoundSize;

  TextStyle m_textStyle;
  String m_text;
  Vec2I m_textOffset;

  StringList m_clickSounds;
  StringList m_releaseSounds;
  StringList m_hoverSounds;
  StringList m_hoverOffSounds;

  bool m_sustain;

private:
  HorizontalAnchor m_hTextAnchor;
  Color m_fontColor;
  Color m_fontColorDisabled;
  Maybe<Color> m_fontColorChecked;
};

// GCC 16 needs the existing key function in this complete-definition TU for RTTI.
ButtonWidget::~ButtonWidget() {
  if (auto group = m_buttonGroup.lock())
    group->removeButton(this);
}

}

export module star.button_widget;

export namespace Star {
  using ::Star::ButtonWidget;
  using ::Star::ButtonWidgetPtr;
  using ::Star::ButtonWidgetConstPtr;
  using ::Star::ButtonWidgetWeakPtr;
  using ::Star::ButtonWidgetConstWeakPtr;
  using ::Star::ButtonWidgetUPtr;
  using ::Star::ButtonWidgetConstUPtr;
}
