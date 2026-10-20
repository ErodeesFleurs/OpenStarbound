module;
import std;
import star.json;

import star.vector;
import star.casting;
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.rect;
import star.application_controller;
import star.color;
import star.font;
import star.variant;
import star.bi_map;
import star.ref_ptr;
import star.renderer;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
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

enum class SpecialRepeatKeyCodes : String::Char { None, Delete, Backspace, Left, Right };

class TextBoxWidget : public Widget {
public:
  TextBoxWidget(GuiContext& context, String const& startingText, String const& hint, WidgetCallbackFunc callback);

  virtual void update(float dt) override;

  String const& getText() const;
  bool setText(String const& text, bool callback = true, bool moveCursor = true);

  String const& getHint() const;
  void setHint(String const& hint);

  int const& getCursorPosition() const;
  void setCursorPosition(int cursorPosition);


  bool getHidden() const;
  void setHidden(bool hidden);

  // Set the regex that the text-box must match.  Defaults to .*
  String getRegex();
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

  bool sendEvent(InputEvent const& event) override;

  void setDrawBorder(bool drawBorder);
  void setTextAlign(HorizontalAnchor hAnchor);
  int getCursorDrawOffset() const;

  virtual void mouseOver() override;
  virtual void mouseOut() override;
  virtual void mouseReturnStillDown() override;

  virtual void blur() override;

  virtual KeyboardCaptureMode keyboardCaptureMode() const override;
  virtual Maybe<std::pair<RectI, int>> keyboardCaptureArea() const override;

protected:
  virtual void renderImpl() override;

private:
  bool innerSendEvent(InputEvent const& event);
  bool modText(String const& text);
  bool newTextValid(String const& text) const;

  bool m_textHidden;
  String m_text;
  String m_hint;
  String m_regex;
  HorizontalAnchor m_hAnchor;
  VerticalAnchor m_vAnchor;
  TextStyle m_textStyle;
  int m_maxWidth;
  int m_cursorOffset;
  bool m_isHover;
  bool m_isPressed;
  SpecialRepeatKeyCodes m_repeatCode;
  std::int64_t m_repeatKeyThreshold;
  WidgetCallbackFunc m_callback;
  WidgetCallbackFunc m_onBlur;
  WidgetCallbackFunc m_onEnterKey;
  WidgetCallbackFunc m_onEscapeKey;
  Maybe<String> m_nextFocus;
  Maybe<String> m_prevFocus;
  bool m_drawBorder;
  Vec2I m_cursorHoriz;
  Vec2I m_cursorVert;
  bool m_overfillMode;
};

}

export module star.text_box_widget;

export namespace Star {
  using ::Star::SpecialRepeatKeyCodes;
  using ::Star::TextBoxWidget;
}
