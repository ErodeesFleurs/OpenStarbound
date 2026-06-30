#pragma once

#include "StarButtonGroup.hpp"

namespace Star {

class ButtonWidget;
using ButtonWidgetPtr = SharedPtr<ButtonWidget>;

class ButtonWidget : public Widget {
public:
  explicit ButtonWidget(GuiContext& context);
  ButtonWidget(GuiContext& context,
      WidgetCallbackFunc callback,
      String const& baseImage,
      String const& hoverImage = "",
      String const& pressedImage = "",
      String const& disabledImage = "");
  virtual ~ButtonWidget();

  [[nodiscard]] bool sendEvent(InputEvent const& event) override;
  void mouseOver() override;
  void mouseOut() override;
  void mouseReturnStillDown() override;
  void hide() override;

  // Callback is called when the checked / pressed state is changed.
  void setCallback(WidgetCallbackFunc callback);

  [[nodiscard]] observer_ptr<ButtonGroup> buttonGroup() const;
  // Sets the button group for this widget, and adds it to the button group if
  // it is not already added.  Additionally, sets the button as checkable.
  void setButtonGroup(observer_ptr<ButtonGroup> buttonGroup, int id = ButtonGroup::NoButton);
  // If a button group is set, returns this button's id in the button group.
  [[nodiscard]] int buttonGroupId();

  [[nodiscard]] bool isHovered() const;

  [[nodiscard]] bool isPressed() const;
  void setPressed(bool pressed);

  [[nodiscard]] bool isCheckable() const;
  void setCheckable(bool checkable);

  [[nodiscard]] bool isHighlighted() const;
  void setHighlighted(bool highlighted);

  [[nodiscard]] bool isChecked() const;
  void setChecked(bool checked);
  // Either checks a button, or toggles the state, depending on whether the
  // button is part of an exclusive group or not.
  void check();

  [[nodiscard]] bool sustainCallbackOnDownHold();
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
  [[nodiscard]] Vec2I const& pressedOffset() const;
  void setPressedOffset(Vec2I const& offset);

  [[nodiscard]] virtual String const& getText() const;
  virtual void setText(String const& text);
  virtual void setFontSize(int size);
  virtual void setFontDirectives(String directives);
  virtual void setTextOffset(Vec2I textOffset);

  void setTextAlign(HorizontalAnchor hAnchor);
  void setFontColor(Color color);
  void setFontColorDisabled(Color color);
  void setFontColorChecked(Color color);

  [[nodiscard]] WidgetRef<Widget> getChildAt(Vec2I const& pos) override;

  void disable();
  void enable();
  void setEnabled(bool enabled);

  void setInvisible(bool invisible);

protected:
  [[nodiscard]] RectI getScissorRect() const override;
  void renderImpl() override;

  void drawButtonPart(String const& image, Vec2F const& position);
  void updateSize();

  WidgetCallbackFunc m_callback;
  observer_ptr<ButtonGroup> m_buttonGroup;

  bool m_hovered = false;
  bool m_pressed = false;
  bool m_checkable = false;
  bool m_checked = false;

  bool m_disabled = false;
  bool m_highlighted = false;

  String m_baseImage;
  String m_hoverImage;
  String m_pressedImage;
  String m_disabledImage;

  bool m_hasCheckedImages = false;
  String m_baseImageChecked;
  String m_hoverImageChecked;
  String m_pressedImageChecked;
  String m_disabledImageChecked;

  String m_overlayImage;

  bool m_invisible = false;

  Vec2I m_pressedOffset;
  Vec2U m_buttonBoundSize;

  TextStyle m_textStyle;
  String m_text;
  Vec2I m_textOffset;

  StringList m_clickSounds;
  StringList m_releaseSounds;
  StringList m_hoverSounds;
  StringList m_hoverOffSounds;

  bool m_sustain = false;

private:
  HorizontalAnchor m_hTextAnchor = HorizontalAnchor::HMidAnchor;
  Color m_fontColor = Color::White;
  Color m_fontColorDisabled = Color::Gray;
  Maybe<Color> m_fontColorChecked;
};

}
