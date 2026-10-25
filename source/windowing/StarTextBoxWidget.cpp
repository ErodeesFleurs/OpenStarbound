import std;
import star.format;
import star.json;
import star.json_extra;
import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.rect;
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
import star.time;
import star.version;
import star.string;
import star.text;
import star.poly;
import star.asset_path;
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
import star.text_box_widget;


import star.math_common;
namespace Star {

TextBoxWidget::TextBoxWidget(GuiContext& context, String const& startingText, String const& hint, WidgetCallbackFunc callback)
  : Widget(context), m_text(startingText), m_hint(hint), m_callback(std::move(callback)) {
  m_textHidden = false;
  m_regex = "[\\s\\S]*";
  m_repeatKeyThreshold = 0;
  m_repeatCode = SpecialRepeatKeyCodes::None;
  m_isPressed = false;
  m_isHover = false;
  m_cursorOffset = 0;
  m_hAnchor = HorizontalAnchor::LeftAnchor;
  m_vAnchor = VerticalAnchor::BottomAnchor;
  m_drawBorder = false;
  m_cursorHoriz = Vec2I();
  m_cursorVert = Vec2I();
  m_overfillMode = true;

}

std::expected<std::shared_ptr<TextBoxWidget>, StarException> TextBoxWidget::create(GuiContext& context, String const& text, String const& hint, WidgetCallbackFunc callback) {
  auto widget = std::shared_ptr<TextBoxWidget>(new TextBoxWidget(context, text, hint, std::move(callback)));
  auto initialized = widget->initialize();
  if (!initialized)
    return std::unexpected(std::move(initialized.error()));
  return widget;
}

std::expected<void, StarException> TextBoxWidget::initialize() {
  auto length = m_text.size();
  if (!length)
    return std::unexpected(std::move(length.error()));
  if (*length > std::numeric_limits<int>::max())
    return std::unexpected(GuiException("Text box content is too long"));
  m_cursorOffset = static_cast<int>(*length);
  auto assets = context().root().assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  auto config = (*assets)->json("/interface.config");
  if (!config)
    return std::unexpected(std::move(config.error()));
  auto width = config->getInt("textBoxDefaultWidth");
  if (!width)
    return std::unexpected(std::move(width.error()));
  m_maxWidth = *width;
  auto styleJson = config->get("textBoxTextStyle");
  if (!styleJson)
    return std::unexpected(std::move(styleJson.error()));
  auto style = TextStyle::create(*styleJson);
  if (!style)
    return std::unexpected(std::move(style.error()));
  m_textStyle = std::move(*style);
  setSize({m_maxWidth + 6, m_textStyle.fontSize + 2});
  auto horizontalJson = config->get("textboxCursorHorizontal");
  if (!horizontalJson)
    return std::unexpected(std::move(horizontalJson.error()));
  auto horizontal = jsonToVec2I(*horizontalJson);
  if (!horizontal)
    return std::unexpected(std::move(horizontal.error()));
  auto verticalJson = config->get("textboxCursorVertical");
  if (!verticalJson)
    return std::unexpected(std::move(verticalJson.error()));
  auto vertical = jsonToVec2I(*verticalJson);
  if (!vertical)
    return std::unexpected(std::move(vertical.error()));
  m_cursorHoriz = *horizontal;
  m_cursorVert = *vertical;
  return {};
}

std::expected<String, StarException> TextBoxWidget::displayText() const {
  if (m_textHidden) {
    auto length = m_text.length();
    if (!length)
      return std::unexpected(std::move(length.error()));
    return String::create('*', *length);
  }
  auto display = m_text.replace("\n", "↵");
  if (!display)
    return std::unexpected(std::move(display.error()));
  return display->replace("\v", "↵");
}

std::expected<void, StarException> TextBoxWidget::renderImpl() {
  float blueRate = m_isHover && !m_isPressed ? 0.2f : 0.0f;
  auto drawOffset = getCursorDrawOffset();
  if (!drawOffset)
    return std::unexpected(std::move(drawOffset.error()));
  float cursorOffset = static_cast<float>(*drawOffset) / context().interfaceScale();
  Vec2F pos(screenPosition());
  if (m_hAnchor == HorizontalAnchor::HMidAnchor)
    pos[0] += size()[0] / 2.0f;
  else if (m_hAnchor == HorizontalAnchor::RightAnchor)
    pos[0] += size()[0];
  auto styled = context().setTextStyle(m_textStyle);
  if (!styled)
    return std::unexpected(std::move(styled.error()));
  if (m_maxWidth != -1 && m_overfillMode)
    pos[0] -= std::max(0.f, cursorOffset - m_maxWidth);
  auto color = Color::rgba(m_textStyle.color);
  if (m_text.empty())
    color = color.mix(Color::rgbf(0.3f, 0.3f, 0.3f), 0.8f);
  auto colored = context().setFontColor(color.mix(Color::rgbf(0, 0, 1), blueRate).toRgba());
  if (!colored)
    return std::unexpected(std::move(colored.error()));
  auto text = m_text.empty() ? std::expected<String, StarException>(m_hint) : displayText();
  if (!text)
    return std::unexpected(std::move(text.error()));
  auto rendered = context().renderInterfaceText(*text, {pos, m_hAnchor, m_vAnchor});
  auto cleared = context().clearTextStyle();
  if (!rendered)
    return std::unexpected(std::move(rendered.error()));
  if (!cleared)
    return std::unexpected(std::move(cleared.error()));
  auto focused = hasFocus();
  if (!focused)
    return std::unexpected(std::move(focused.error()));
  if (*focused) {
    float cc = 0.6f + 0.4f * std::sin(static_cast<double>(Time::monotonicMilliseconds()) / 300.0);
    auto cursorColor = Color::rgbf(cc, cc, cc).toRgba();
    float fontSize = m_textStyle.fontSize;
    auto vertical = context().drawInterfaceLine(
        pos + Vec2F(cursorOffset, fontSize * m_cursorVert[0]),
        pos + Vec2F(cursorOffset, fontSize * m_cursorVert[1]), cursorColor);
    if (!vertical)
      return std::unexpected(std::move(vertical.error()));
    auto horizontal = context().drawInterfaceLine(
        pos + Vec2F(cursorOffset + fontSize * m_cursorHoriz[0], fontSize * m_cursorVert[0]),
        pos + Vec2F(cursorOffset + fontSize * m_cursorHoriz[1], fontSize * m_cursorVert[0]), cursorColor);
    if (!horizontal)
      return std::unexpected(std::move(horizontal.error()));
  }
  if (m_drawBorder)
    return context().drawInterfacePolyLines(PolyF(screenBoundRect()), Color::White.toRgba());
  return {};
}

std::expected<int, StarException> TextBoxWidget::getCursorDrawOffset() const {
  auto styled = context().setTextStyle(m_textStyle);
  if (!styled)
    return std::unexpected(std::move(styled.error()));
  auto text = displayText();
  if (!text)
    return std::unexpected(std::move(text.error()));
  auto tail = text->substr(m_cursorOffset);
  if (!tail)
    return std::unexpected(std::move(tail.error()));
  auto width = context().stringWidth(*text);
  if (!width)
    return std::unexpected(std::move(width.error()));
  auto tailWidth = context().stringWidth(*tail);
  if (!tailWidth)
    return std::unexpected(std::move(tailWidth.error()));
  if (m_hAnchor == HorizontalAnchor::RightAnchor)
    return -*width + *tailWidth;
  float scale;
  if (m_hAnchor == HorizontalAnchor::LeftAnchor)
    scale = 1.0f;
  else if (m_hAnchor == HorizontalAnchor::HMidAnchor)
    scale = 0.5f;
  else
    return std::unexpected(GuiException("Somehow, the value of m_hAnchor became bad"));
  return static_cast<int>(std::ceil(*width * scale - *tailWidth));
}

std::expected<void, StarException> TextBoxWidget::update(float dt) {
  auto updated = Widget::update(dt);
  if (!updated)
    return std::unexpected(std::move(updated.error()));
  if (m_repeatCode != SpecialRepeatKeyCodes::None && Time::monotonicMilliseconds() >= m_repeatKeyThreshold) {
    m_repeatKeyThreshold += 50;
    auto length = m_text.size();
    if (!length)
      return std::unexpected(std::move(length.error()));
    if (m_repeatCode == SpecialRepeatKeyCodes::Delete && std::cmp_less(m_cursorOffset, *length)) {
      auto modified = editText(m_cursorOffset, 1, "");
      if (!modified)
        return std::unexpected(std::move(modified.error()));
    } else if (m_repeatCode == SpecialRepeatKeyCodes::Backspace && m_cursorOffset > 0) {
      auto modified = editText(m_cursorOffset - 1, 1, "");
      if (!modified)
        return std::unexpected(std::move(modified.error()));
      if (*modified)
        --m_cursorOffset;
    } else if (m_repeatCode == SpecialRepeatKeyCodes::Left && m_cursorOffset > 0) {
      --m_cursorOffset;
    } else if (m_repeatCode == SpecialRepeatKeyCodes::Right && std::cmp_less(m_cursorOffset, *length)) {
      ++m_cursorOffset;
    }
  }
  return {};
}

String const& TextBoxWidget::getText() const {
  return m_text;
}

std::expected<bool, StarException> TextBoxWidget::setText(String const& text, bool callback, bool moveCursor) {
  if (m_text == text)
    return true;
  auto valid = newTextValid(text);
  if (!valid)
    return std::unexpected(std::move(valid.error()));
  if (!*valid)
    return false;
  auto length = text.size();
  if (!length)
    return std::unexpected(std::move(length.error()));
  if (*length > std::numeric_limits<int>::max())
    return std::unexpected(GuiException("Text box content is too long"));
  m_text = text;
  if (moveCursor || std::cmp_greater(m_cursorOffset, *length))
    m_cursorOffset = static_cast<int>(*length);
  m_repeatCode = SpecialRepeatKeyCodes::None;
  if (callback) {
    if (!m_callback)
      return std::unexpected(GuiException("Text box callback is not configured"));
    auto called = m_callback(*this);
    if (!called)
      return std::unexpected(std::move(called.error()));
  }
  return true;
}

String const& TextBoxWidget::getHint() const {
  return m_hint;
}

void TextBoxWidget::setHint(String const& hint) {
  m_hint = hint;
}

int const& TextBoxWidget::getCursorPosition() const {
  return m_cursorOffset;
}

std::expected<void, StarException> TextBoxWidget::setCursorPosition(int cursorPosition) {
  auto length = m_text.size();
  if (!length)
    return std::unexpected(std::move(length.error()));
  m_cursorOffset = clamp(cursorPosition, 0, static_cast<int>(std::min<std::size_t>(*length, std::numeric_limits<int>::max())));
  return {};
}

bool TextBoxWidget::getHidden() const {
  return m_textHidden;
}

void TextBoxWidget::setHidden(bool hidden) {
  m_textHidden = hidden;
}

String TextBoxWidget::getRegex() {
  return m_regex;
}

void TextBoxWidget::setRegex(String const& regex) {
  m_regex = regex;
}

void TextBoxWidget::setColor(Color const& color) {
  m_textStyle.color = color.toRgba();
}

void TextBoxWidget::setDirectives(String const& directives) {
  m_textStyle.directives = directives;
}

void TextBoxWidget::setFontSize(int fontSize) {
  m_textStyle.fontSize = fontSize;
}

void TextBoxWidget::setFont(String const& font) {
  m_textStyle.font = font;
}

void TextBoxWidget::setMaxWidth(int maxWidth) {
  m_maxWidth = maxWidth;
  setSize({m_maxWidth + 6, m_textStyle.fontSize + 2});
}

void TextBoxWidget::setOverfillMode(bool overtype) {
  m_overfillMode = overtype;
}

void TextBoxWidget::setOnBlurCallback(WidgetCallbackFunc onBlur) {
  m_onBlur = std::move(onBlur);
}

void TextBoxWidget::setOnEnterKeyCallback(WidgetCallbackFunc onEnterKey) {
  m_onEnterKey = std::move(onEnterKey);
}

void TextBoxWidget::setOnEscapeKeyCallback(WidgetCallbackFunc onEscapeKey) {
  m_onEscapeKey = std::move(onEscapeKey);
}

void TextBoxWidget::setNextFocus(std::optional<String> nextFocus) {
  m_nextFocus = nextFocus;
}

void TextBoxWidget::setPrevFocus(std::optional<String> prevFocus) {
  m_prevFocus = prevFocus;
}

std::expected<void, StarException> TextBoxWidget::mouseOver() {
  auto over = Widget::mouseOver();
  if (!over)
    return std::unexpected(std::move(over.error()));
  m_isHover = true;
  return {};
}

std::expected<void, StarException> TextBoxWidget::mouseOut() {
  auto out = Widget::mouseOut();
  if (!out)
    return std::unexpected(std::move(out.error()));
  m_isHover = false;
  m_isPressed = false;
  return {};
}

std::expected<void, StarException> TextBoxWidget::mouseReturnStillDown() {
  auto returned = Widget::mouseReturnStillDown();
  if (!returned)
    return std::unexpected(std::move(returned.error()));
  m_isHover = true;
  m_isPressed = true;
  return {};
}

std::expected<void, StarException> TextBoxWidget::blur() {
  auto blurred = Widget::blur();
  if (!blurred)
    return std::unexpected(std::move(blurred.error()));
  if (m_onBlur) {
    auto called = m_onBlur(*this);
    if (!called)
      return std::unexpected(std::move(called.error()));
  }
  m_repeatCode = SpecialRepeatKeyCodes::None;
  return {};
}

std::expected<KeyboardCaptureMode, StarException> TextBoxWidget::keyboardCaptureMode() const {
  if (active()) {
    auto focused = hasFocus();
    if (!focused)
      return std::unexpected(std::move(focused.error()));
    if (*focused)
      return KeyboardCaptureMode::TextInput;
  }
  return KeyboardCaptureMode::None;
}

std::expected<std::optional<std::pair<RectI, int>>, StarException> TextBoxWidget::keyboardCaptureArea() const {
  if (active()) {
    auto focused = hasFocus();
    if (!focused)
      return std::unexpected(std::move(focused.error()));
    if (*focused) {
      auto offset = getCursorDrawOffset();
      if (!offset)
        return std::unexpected(std::move(offset.error()));
      return std::make_pair(screenBoundRect().scaled(context().interfaceScale()), *offset);
    }
  }
  return std::nullopt;
}

std::expected<bool, StarException> TextBoxWidget::sendEvent(InputEvent const& event) {
  auto focused = hasFocus();
  if (!focused)
    return std::unexpected(std::move(focused.error()));
  if (!*focused)
    return false;
  if (std::holds_alternative<KeyUpEvent>(event)) {
    m_repeatCode = SpecialRepeatKeyCodes::None;
    if (!m_callback)
      return std::unexpected(GuiException("Text box callback is not configured"));
    auto called = m_callback(*this);
    if (!called)
      return std::unexpected(std::move(called.error()));
    return false;
  }
  auto handled = innerSendEvent(event);
  if (!handled)
    return std::unexpected(std::move(handled.error()));
  if (*handled) {
    if (!m_callback)
      return std::unexpected(GuiException("Text box callback is not configured"));
    auto called = m_callback(*this);
    if (!called)
      return std::unexpected(std::move(called.error()));
  }
  return *handled;
}

std::expected<bool, StarException> TextBoxWidget::innerSendEvent(InputEvent const& event) {
  if (std::holds_alternative<KeyDownEvent>(event)) {
    auto const& keyDown = std::get<KeyDownEvent>(event);
    m_repeatKeyThreshold = Time::monotonicMilliseconds() + 300LL;
    if (keyDown.key == Key::Escape) {
      if (m_onEscapeKey) {
        auto called = m_onEscapeKey(*this);
        if (!called)
          return std::unexpected(std::move(called.error()));
        return true;
      }
      return false;
    }
    if (keyDown.key == Key::Return || keyDown.key == Key::KeypadEnter) {
      if ((keyDown.mods & (KeyMod::LShift | KeyMod::RShift)) != KeyMod::NoMod) {
        auto modified = editText(m_cursorOffset, 0, "\n");
        if (!modified)
          return std::unexpected(std::move(modified.error()));
        if (*modified)
          ++m_cursorOffset;
        return true;
      }
      if (m_onEnterKey) {
        auto called = m_onEnterKey(*this);
        if (!called)
          return std::unexpected(std::move(called.error()));
        return true;
      }
      return false;
    }
    if (keyDown.key == Key::Tab) {
      bool reverse = (keyDown.mods & (KeyMod::LShift | KeyMod::RShift)) != KeyMod::NoMod;
      auto const& target = reverse ? m_prevFocus : m_nextFocus;
      if (auto owner = parent(); target && owner) {
        auto widget = owner->get().fetchChild(*target);
        if (!widget)
          return std::unexpected(std::move(widget.error()));
        if (*widget) {
          auto focused = (*widget)->focus();
          if (!focused)
            return std::unexpected(std::move(focused.error()));
          return true;
        }
      }
      return false;
    }
    if ((keyDown.mods & (KeyMod::LCtrl | KeyMod::RCtrl)) != KeyMod::NoMod) {
      if (keyDown.key == Key::C || keyDown.key == Key::X) {
        context().setClipboard(m_text);
        if (keyDown.key == Key::X) {
          auto modified = modText("");
          if (!modified)
            return std::unexpected(std::move(modified.error()));
          if (*modified)
            m_cursorOffset = 0;
        }
        return true;
      }
      if (keyDown.key == Key::V) {
        auto clipboard = context().getClipboard();
        auto length = clipboard.size();
        if (!length)
          return std::unexpected(std::move(length.error()));
        auto modified = editText(m_cursorOffset, 0, clipboard);
        if (!modified)
          return std::unexpected(std::move(modified.error()));
        if (*modified)
          m_cursorOffset += *length;
        return true;
      }
    }
    auto calculateSteps = [&](bool right) -> std::expected<int, StarException> {
      if ((keyDown.mods & (KeyMod::LCtrl | KeyMod::RCtrl | KeyMod::LAlt | KeyMod::RAlt)) != KeyMod::NoMod) {
        auto boundary = m_text.findNextBoundary(m_cursorOffset, !right);
        if (!boundary)
          return std::unexpected(std::move(boundary.error()));
        int steps = right ? static_cast<int>(*boundary) - m_cursorOffset : m_cursorOffset - static_cast<int>(*boundary);
        return std::max(steps, 1);
      }
      return 1;
    };
    if (keyDown.key == Key::Backspace || keyDown.key == Key::Delete ||
        keyDown.key == Key::Left || keyDown.key == Key::Right) {
      bool right = keyDown.key == Key::Delete || keyDown.key == Key::Right;
      auto steps = calculateSteps(right);
      if (!steps)
        return std::unexpected(std::move(steps.error()));
      auto length = m_text.size();
      if (!length)
        return std::unexpected(std::move(length.error()));
      if (keyDown.key == Key::Backspace) {
        m_repeatCode = SpecialRepeatKeyCodes::Backspace;
        for (int i = 0; i < *steps && m_cursorOffset > 0; ++i) {
          auto modified = editText(m_cursorOffset - 1, 1, "");
          if (!modified)
            return std::unexpected(std::move(modified.error()));
          if (*modified)
            --m_cursorOffset;
        }
      } else if (keyDown.key == Key::Delete) {
        m_repeatCode = SpecialRepeatKeyCodes::Delete;
        for (int i = 0; i < *steps; ++i) {
          auto currentLength = m_text.size();
          if (!currentLength)
            return std::unexpected(std::move(currentLength.error()));
          if (std::cmp_less(m_cursorOffset, *currentLength)) {
            auto modified = editText(m_cursorOffset, 1, "");
            if (!modified)
              return std::unexpected(std::move(modified.error()));
          }
        }
      } else if (keyDown.key == Key::Left) {
        m_repeatCode = SpecialRepeatKeyCodes::Left;
        m_cursorOffset = std::max(0, m_cursorOffset - *steps);
      } else {
        m_repeatCode = SpecialRepeatKeyCodes::Right;
        m_cursorOffset = static_cast<int>(std::min<std::size_t>(*length, static_cast<std::size_t>(m_cursorOffset) + *steps));
      }
      return true;
    }
    if (keyDown.key == Key::Home) {
      m_cursorOffset = 0;
      return true;
    }
    if (keyDown.key == Key::End) {
      auto length = m_text.size();
      if (!length)
        return std::unexpected(std::move(length.error()));
      m_cursorOffset = *length;
      return true;
    }
  }
  if (std::holds_alternative<TextInputEvent>(event)) {
    auto const& textInput = std::get<TextInputEvent>(event);
    auto length = textInput.text.size();
    if (!length)
      return std::unexpected(std::move(length.error()));
    auto modified = editText(m_cursorOffset, 0, textInput.text);
    if (!modified)
      return std::unexpected(std::move(modified.error()));
    if (*modified)
      m_cursorOffset += *length;
    return true;
  }
  return false;
}

void TextBoxWidget::setDrawBorder(bool drawBorder) {
  m_drawBorder = drawBorder;
}

void TextBoxWidget::setTextAlign(HorizontalAnchor hAnchor) {
  m_hAnchor = hAnchor;
}

std::expected<bool, StarException> TextBoxWidget::editText(std::size_t start, std::size_t count, String const& replacement) {
  auto before = m_text.substr(0, start);
  if (!before)
    return std::unexpected(std::move(before.error()));
  auto after = m_text.substr(start + count);
  if (!after)
    return std::unexpected(std::move(after.error()));
  return modText(*before + replacement + *after);
}

std::expected<bool, StarException> TextBoxWidget::modText(String const& text) {
  if (m_text == text)
    return false;
  auto valid = newTextValid(text);
  if (!valid)
    return std::unexpected(std::move(valid.error()));
  if (!*valid)
    return false;
  auto length = text.size();
  if (!length)
    return std::unexpected(std::move(length.error()));
  if (*length > std::numeric_limits<int>::max())
    return std::unexpected(GuiException("Text box content is too long"));
  m_text = text;
  return true;
}

std::expected<bool, StarException> TextBoxWidget::newTextValid(String const& text) const {
  auto matched = text.regexMatch(m_regex);
  if (!matched)
    return std::unexpected(std::move(matched.error()));
  if (!*matched)
    return false;
  if (m_maxWidth != -1 && !m_overfillMode) {
    auto styled = context().setTextStyle(m_textStyle);
    if (!styled)
      return std::unexpected(std::move(styled.error()));
    auto width = context().stringInterfaceWidth(text);
    if (!width)
      return std::unexpected(std::move(width.error()));
    return *width <= m_maxWidth;
  }
  return true;
}

}
