#pragma once

#include "StarWidget.hpp"

namespace Star {

enum class SpecialRepeatKeyCodes : String::Char { None, Delete, Backspace, Left, Right };

class TextBoxWidget;
using TextBoxWidgetPtr = SharedPtr<TextBoxWidget>;
class TextBoxWidget : public Widget {
public:
  TextBoxWidget(GuiContext& context, String const& startingText, String const& hint, WidgetCallbackFunc callback);

  void update(float dt) override;

  [[nodiscard]] String const& getText() const;
  [[nodiscard]] bool setText(String const& text, bool callback = true, bool moveCursor = true);

  [[nodiscard]] String const& getHint() const;
  void setHint(String const& hint);

  [[nodiscard]] int const& getCursorPosition() const;
  void setCursorPosition(int cursorPosition);


  [[nodiscard]] bool getHidden() const;
  void setHidden(bool hidden);

  // Set the regex that the text-box must match.  Defaults to .*
  [[nodiscard]] String getRegex();
  void setRegex(String const& regex);

  void setColor(Color const& color);
  void setDirectives(String const& directives);
  void setFontSize(int fontSize);
  void setMaxWidth(int maxWidth);
  void setOverfillMode(bool overfillMode);

  void setOnBlurCallback(WidgetCallbackFunc onBlur);
  void setOnEnterKeyCallback(WidgetCallbackFunc onEnterKey);
  void setOnEscapeKeyCallback(WidgetCallbackFunc onEscapeKey);

  void setNextFocus(Maybe<String> nextFocus);
  void setPrevFocus(Maybe<String> prevFocus);

  void setFont(String const& font);

  [[nodiscard]] bool sendEvent(InputEvent const& event) override;

  void setDrawBorder(bool drawBorder);
  void setTextAlign(HorizontalAnchor hAnchor);
  [[nodiscard]] int getCursorDrawOffset() const;

  void mouseOver() override;
  void mouseOut() override;
  void mouseReturnStillDown() override;

  void blur() override;

  [[nodiscard]] KeyboardCaptureMode keyboardCaptureMode() const override;
  [[nodiscard]] Maybe<pair<RectI, int>> keyboardCaptureArea() const override;

protected:
  void renderImpl() override;

private:
  [[nodiscard]] bool innerSendEvent(InputEvent const& event);
  [[nodiscard]] bool modText(String const& text);
  [[nodiscard]] bool newTextValid(String const& text) const;

  bool m_textHidden = false;
  String m_text;
  String m_hint;
  String m_regex = ".*";
  HorizontalAnchor m_hAnchor = HorizontalAnchor::LeftAnchor;
  VerticalAnchor m_vAnchor = VerticalAnchor::BottomAnchor;
  TextStyle m_textStyle;
  int m_maxWidth;
  int m_cursorOffset;
  bool m_isHover = false;
  bool m_isPressed = false;
  SpecialRepeatKeyCodes m_repeatCode = SpecialRepeatKeyCodes::None;
  int64_t m_repeatKeyThreshold = 0;
  WidgetCallbackFunc m_callback;
  WidgetCallbackFunc m_onBlur;
  WidgetCallbackFunc m_onEnterKey;
  WidgetCallbackFunc m_onEscapeKey;
  Maybe<String> m_nextFocus;
  Maybe<String> m_prevFocus;
  bool m_drawBorder = false;
  Vec2I m_cursorHoriz;
  Vec2I m_cursorVert;
  bool m_overfillMode = true;
};

}
