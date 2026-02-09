#include "StarTextPainter.hpp"

#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

TextPositioning::TextPositioning() {
  pos = Vec2F();
  hAnchor = HorizontalAnchor::LeftAnchor;
  vAnchor = VerticalAnchor::BottomAnchor;
}

TextPositioning::TextPositioning(Vec2F pos, HorizontalAnchor hAnchor, VerticalAnchor vAnchor,
                                 std::optional<unsigned> wrapWidth, std::optional<unsigned> charLimit)
    : pos(pos), hAnchor(hAnchor), vAnchor(vAnchor), wrapWidth(wrapWidth), charLimit(charLimit) {}

TextPositioning::TextPositioning(Json const& v) {
  pos = v.opt("position").transform(jsonToVec2F).value();
  hAnchor = HorizontalAnchorNames.getLeft(v.getString("horizontalAnchor", "left"));
  vAnchor = VerticalAnchorNames.getLeft(v.getString("verticalAnchor", "top"));
  wrapWidth = v.optUInt("wrapWidth");
  charLimit = v.optUInt("charLimit");
}

auto TextPositioning::toJson() const -> Json {
  return JsonObject{
    {"position", jsonFromVec2F(pos)},
    {"horizontalAnchor", HorizontalAnchorNames.getRight(hAnchor)},
    {"verticalAnchor", VerticalAnchorNames.getRight(vAnchor)},
    {"wrapWidth", jsonFromMaybe(wrapWidth)}};
}

auto TextPositioning::translated(Vec2F translation) const -> TextPositioning {
  return {pos + translation, hAnchor, vAnchor, wrapWidth, charLimit};
}

TextPainter::TextPainter(Ptr<Renderer> renderer, Ptr<TextureGroup> textureGroup)
    : m_renderer(std::move(renderer)),
      m_fontTextureGroup(textureGroup),
      m_defaultRenderSettings(),
      m_renderSettings(),
      m_savedRenderSettings() {
  reloadFonts();
  m_reloadTracker = std::make_shared<TrackerListener>();
  Root::singleton().registerReloadListener(m_reloadTracker);
}

auto TextPainter::renderText(StringView s, TextPositioning const& position) -> RectF {
  RectF rect;
  if (position.charLimit) {
    unsigned charLimit = *position.charLimit;
    rect = doRenderText(s, position, true, &charLimit);
  } else {
    rect = doRenderText(s, position, true, nullptr);
  }
  renderPrimitives();
  return rect;
}

auto TextPainter::renderLine(StringView s, TextPositioning const& position) -> RectF {
  RectF rect;
  if (position.charLimit) {
    unsigned charLimit = *position.charLimit;
    rect = doRenderLine(s, position, true, &charLimit);
  } else {
    rect = doRenderLine(s, position, true, nullptr);
  }
  renderPrimitives();
  return rect;
}

auto TextPainter::renderGlyph(String::Char c, TextPositioning const& position) -> RectF {
  auto rect = doRenderGlyph(c, position, true);
  renderPrimitives();
  return rect;
}

auto TextPainter::determineTextSize(StringView s, TextPositioning const& position) -> RectF {
  return doRenderText(s, position, false, nullptr);
}

auto TextPainter::determineLineSize(StringView s, TextPositioning const& position) -> RectF {
  return doRenderLine(s, position, false, nullptr);
}

auto TextPainter::determineGlyphSize(String::Char c, TextPositioning const& position) -> RectF {
  return doRenderGlyph(c, position, false);
}

auto TextPainter::glyphWidth(String::Char c) -> int {
  return m_fontTextureGroup.glyphWidth(c, m_renderSettings.fontSize);
}

auto TextPainter::stringWidth(StringView s, unsigned charLimit) -> int {
  if (s.empty())
    return 0;

  String font = m_renderSettings.font, setFont = font;
  m_fontTextureGroup.switchFont(font);

  Text::CommandsCallback commandsCallback = [&](StringView commands) -> bool {
    commands.forEachSplitView(",", [&](StringView command, size_t, size_t) -> void {
      if (command == "reset") {
        m_fontTextureGroup.switchFont(font = setFont);
      } else if (command == "set") {
        setFont = font;
      } else if (command.beginsWith("font=")) {
        m_fontTextureGroup.switchFont(font = command.substr(5));
      }
    });
    return true;
  };

  int width = 0;
  Text::TextCallback textCallback = [&](StringView text) -> bool {
    for (String::Char c : text) {
      width += glyphWidth(c);
      if (charLimit && --charLimit == 0)
        return false;
    }
    return true;
  };

  Text::processText(s, textCallback, commandsCallback);

  return width;
}

auto TextPainter::processWrapText(StringView text, unsigned* wrapWidth, WrapTextCallback textFunc) -> bool {
  String font = m_renderSettings.font, setFont = font;
  m_fontTextureGroup.switchFont(font);
  auto iterator = text.begin(), end = text.end();

  unsigned lines = 0;
  auto lineStartIterator = iterator, splitIterator = end;
  unsigned linePixelWidth = 0, splitPixelWidth = 0;
  size_t commandStart = std::numeric_limits<std::size_t>::max(), commandEnd = std::numeric_limits<std::size_t>::max();
  bool finished = true;

  auto slice = [](StringView::const_iterator a, StringView::const_iterator b) -> StringView {
    return {&*a.base(), static_cast<std::size_t>(b.base() - a.base())};
  };

  while (iterator != end) {
    auto character = *iterator;
    finished = false;// assume at least one character if we get here
    bool noMoreCommands = commandStart != std::numeric_limits<std::size_t>::max() && commandEnd == std::numeric_limits<std::size_t>::max();
    if (!noMoreCommands && Text::isEscapeCode(character)) {
      size_t index = &*iterator.base() - text.utf8Ptr();
      if (commandStart == std::numeric_limits<std::size_t>::max()) {
        for (size_t escOrEnd = commandStart = index;
             (escOrEnd = text.utf8().find_first_of(Text::AllEscEnd, escOrEnd + 1)) != std::numeric_limits<std::size_t>::max();) {
          if (text.utf8().at(escOrEnd) != Text::EndEsc)
            commandStart = escOrEnd;
          else {
            commandEnd = escOrEnd;
            break;
          }
        }
      }
      if (commandStart == index && commandEnd != std::numeric_limits<std::size_t>::max()) {
        const char* commandStr = text.utf8Ptr() + ++commandStart;
        StringView inner(commandStr, commandEnd - commandStart);
        inner.forEachSplitView(",", [&](StringView command, size_t, size_t) -> void {
          if (command == "reset") {
            m_fontTextureGroup.switchFont(font = setFont);
          } else if (command == "set") {
            setFont = font;
          } else if (command.beginsWith("font=")) {
            m_fontTextureGroup.switchFont(font = command.substr(5));
          }
        });
        // jump the iterator to the character after the command
        iterator = text.utf8().begin() + commandEnd + 1;
        commandStart = commandEnd = std::numeric_limits<std::size_t>::max();
        continue;
      }
    }
    // is this a linefeed / cr / whatever that forces a line split ?
    if (character == '\n' || character == '\v') {
      // knock one off the end because we don't render the CR
      if (!textFunc(slice(lineStartIterator, iterator), lines++))
        return false;

      lineStartIterator = iterator;
      ++lineStartIterator;
      // next line starts after the CR with no characters in it and no known splits.
      linePixelWidth = 0;
      splitIterator = end;
      finished = true;
    } else {
      int characterWidth = glyphWidth(character);
      // is it a place where we might want to split the line ?
      if (character == ' ' || character == '\t') {
        splitIterator = iterator;
        splitPixelWidth = linePixelWidth + characterWidth;
      }

      // would the line be too long if we render this next character ?
      if (wrapWidth && (linePixelWidth + characterWidth) > *wrapWidth) {
        // did we find somewhere to split the line ?
        if (splitIterator != end) {
          if (!textFunc(slice(lineStartIterator, splitIterator), lines++))
            return false;
          // do not include the split character on the next line
          unsigned stringWidth = linePixelWidth - splitPixelWidth;
          linePixelWidth = stringWidth + characterWidth;
          lineStartIterator = ++splitIterator;
          splitIterator = end;
        } else {
          if (!textFunc(slice(lineStartIterator, iterator), lines++))
            return false;
          // include that character on the next line
          lineStartIterator = iterator;
          linePixelWidth = characterWidth;
          finished = false;
        }
      } else {
        linePixelWidth += characterWidth;
      }
    }

    ++iterator;
  };

  // if we hit the end of the string before hitting the end of the line
  return finished || textFunc(slice(lineStartIterator, end), lines);
}

auto TextPainter::wrapTextViews(StringView s, std::optional<unsigned> wrapWidth) -> List<StringView> {
  List<StringView> views = {};
  auto last = views.end();
  unsigned curLine = 0;
  TextPainter::WrapTextCallback textCallback = [&](StringView text, unsigned line) -> bool {
    if (line == curLine && last != views.end() && last->end() == text.begin()) {
      *last = StringView(last->utf8Ptr(), last->utf8Size() + text.utf8Size());
    } else {
      last = views.insert(views.end(), text);
      curLine = line;
    }
    return true;
  };

  processWrapText(s, &wrapWidth.value(), textCallback);

  return views;
}

auto TextPainter::wrapText(StringView s, std::optional<unsigned> wrapWidth) -> StringList {
  StringList result;

  String current;
  unsigned lastLine = 0;
  TextPainter::WrapTextCallback textCallback = [&](StringView text, unsigned line) -> bool {
    if (lastLine != line) {
      result.append(std::move(current));
      lastLine = line;
    }
    current += text;
    return true;
  };

  processWrapText(s, &wrapWidth.value(), textCallback);

  if (!current.empty())
    result.append(std::move(current));

  return result;
};

auto TextPainter::fontSize() const -> unsigned {
  return m_renderSettings.fontSize;
}

void TextPainter::setFontSize(unsigned size) {
  m_renderSettings.fontSize = size;
}

void TextPainter::setLineSpacing(float lineSpacing) {
  m_renderSettings.lineSpacing = lineSpacing;
}

void TextPainter::setMode(FontMode mode) {
  m_renderSettings.shadow = fontModeToColor(mode).toRgba();
}

void TextPainter::setFontColor(Vec4B color) {
  m_renderSettings.color = std::move(color);
}

void TextPainter::setProcessingDirectives(StringView directives, bool back) {
  Directives& target = back ? m_renderSettings.backDirectives : m_renderSettings.directives;
  modifyDirectives(target = String(directives));
}

void TextPainter::setFont(String const& font) {
  m_fontTextureGroup.switchFont(m_renderSettings.font = font);
}

auto TextPainter::setTextStyle(TextStyle const& textStyle) -> TextStyle& {
  TextStyle& style = m_renderSettings = textStyle;
  modifyDirectives(style.directives);
  modifyDirectives(style.backDirectives);
  m_fontTextureGroup.switchFont(style.font);
  return style;
}

void TextPainter::clearTextStyle() {
  m_renderSettings = m_defaultRenderSettings;
  m_fontTextureGroup.switchFont(m_renderSettings.font);
}

void TextPainter::addFont(Ptr<Font> const& font, String const& name) {
  m_fontTextureGroup.addFont(font, name);
}

void TextPainter::reloadFonts() {
  m_fontTextureGroup.clearFonts();
  m_fontTextureGroup.cleanup(0);
  auto assets = Root::singleton().assets();
  auto loadFontsByExtension = [&](String const& ext) -> void {
    for (auto& fontPath : assets->scanExtension(ext)) {
      auto font = assets->font(fontPath);
      auto name = AssetPath::filename(fontPath);
      name = name.substr(0, name.findLast("."));
      addFont(loadFont(fontPath, name), name);
    }
  };
  loadFontsByExtension("ttf");
  loadFontsByExtension("woff2");
  m_fontTextureGroup.setFixedFonts(
    assets->json("/interface.config:font.defaultFont").toString(),
    assets->json("/interface.config:font.fallbackFont").toString(),
    assets->json("/interface.config:font.emojiFont").toString());
}

void TextPainter::cleanup(std::int64_t timeout) {
  m_fontTextureGroup.cleanup(timeout);
}

void TextPainter::applyCommands(StringView unsplitCommands) {
  unsplitCommands.forEachSplitView(",", [&](StringView command, size_t, size_t) -> void {
    try {
      if (command == "reset") {
        m_renderSettings = m_savedRenderSettings;
        m_fontTextureGroup.switchFont(m_renderSettings.font);
      } else if (command == "set") {
        m_savedRenderSettings = m_renderSettings;
      } else if (command.beginsWith("shadow")) {
        if (command.utf8Size() == 6)
          m_renderSettings.shadow = Color::Black.toRgba();
        else if (command.utf8()[6] == '=')
          m_renderSettings.shadow = Color(command.substr(7)).toRgba();
      } else if (command == "noshadow") {
        m_renderSettings.shadow = Color::Clear.toRgba();
      } else if (command.beginsWith("font=")) {
        setFont(m_renderSettings.font = command.substr(5));
      } else if (command.beginsWith("directives=")) {
        setProcessingDirectives(command.substr(11));
      } else if (command.beginsWith("backdirectives=")) {
        setProcessingDirectives(command.substr(15), true);
      } else {
        // expects both #... sequences and plain old color names.
        auto c = Color(command);
        c.setAlphaF(c.alphaF() * ((float)m_savedRenderSettings.color[3]) / 255);
        m_renderSettings.color = c.toRgba();
      }
    } catch (JsonException&) {
    } catch (ColorException&) {
    }
  });
}

void TextPainter::modifyDirectives(Directives& directives) {
  if (directives) {
    directives.loadOperations();
    for (auto& entry : directives->entries) {
      if (auto border = entry.operation.ptr<BorderImageOperation>())
        border->includeTransparent = true;
    }
  }
}

auto TextPainter::doRenderText(StringView s, TextPositioning const& position, bool reallyRender, unsigned* charLimit) -> RectF {
  Vec2F pos = position.pos;
  if (s.empty())
    return {pos, pos};

  List<StringView> lines = wrapTextViews(s, position.wrapWidth);

  TextStyle backup = m_savedRenderSettings = m_renderSettings;
  int height = (lines.size() - 1) * backup.lineSpacing * backup.fontSize + backup.fontSize;
  if (position.vAnchor == VerticalAnchor::BottomAnchor)
    pos[1] += (height - backup.fontSize);
  else if (position.vAnchor == VerticalAnchor::VMidAnchor)
    pos[1] += (height - backup.fontSize) / 2;

  RectF bounds = RectF::withSize(pos, Vec2F());
  for (auto& i : lines) {
    bounds.combine(doRenderLine(i, {pos, position.hAnchor, position.vAnchor}, reallyRender, charLimit));
    pos[1] -= m_renderSettings.fontSize * m_renderSettings.lineSpacing;

    if (charLimit && *charLimit == 0)
      break;
  }

  m_renderSettings = std::move(backup);
  m_fontTextureGroup.switchFont(m_renderSettings.font);

  return bounds;
}

auto TextPainter::doRenderLine(StringView text, TextPositioning const& position, bool reallyRender, unsigned* charLimit) -> RectF {
  if (m_reloadTracker->pullTriggered())
    reloadFonts();
  TextPositioning pos = position;

  if (pos.hAnchor == HorizontalAnchor::RightAnchor) {
    StringView trimmedString = charLimit ? text.substr(0, *charLimit) : text;
    pos.pos[0] -= stringWidth(trimmedString);
    pos.hAnchor = HorizontalAnchor::LeftAnchor;
  } else if (pos.hAnchor == HorizontalAnchor::HMidAnchor) {
    StringView trimmedString = charLimit ? text.substr(0, *charLimit) : text;
    pos.pos[0] -= std::floor((float)stringWidth(trimmedString) / 2);
    pos.hAnchor = HorizontalAnchor::LeftAnchor;
  }

  String escapeCode;
  RectF bounds = RectF::withSize(pos.pos, Vec2F());
  Text::TextCallback textCallback = [&](StringView text) -> bool {
    for (String::Char c : text) {
      if (charLimit) {
        if (*charLimit == 0)
          return false;
        else
          --*charLimit;
      }
      RectF glyphBounds = doRenderGlyph(c, pos, reallyRender);
      bounds.combine(glyphBounds);
      pos.pos[0] += glyphBounds.width();
    }
    return true;
  };

  Text::CommandsCallback commandsCallback = [&](StringView commands) -> bool {
    applyCommands(commands);
    return true;
  };

  m_fontTextureGroup.switchFont(m_renderSettings.font);
  Text::processText(text, textCallback, commandsCallback);

  return bounds;
}

auto TextPainter::doRenderGlyph(String::Char c, TextPositioning const& position, bool reallyRender) -> RectF {
  if (c == '\n' || c == '\v' || c == '\r')
    return {};

  int width = glyphWidth(c);
  // Offset left by width if right anchored.
  float hOffset = 0;
  if (position.hAnchor == HorizontalAnchor::RightAnchor)
    hOffset = -width;
  else if (position.hAnchor == HorizontalAnchor::HMidAnchor)
    hOffset = -std::floor((float)width / 2);

  float vOffset = 0;
  if (position.vAnchor == VerticalAnchor::VMidAnchor)
    vOffset = -std::floor((float)m_renderSettings.fontSize / 2);
  else if (position.vAnchor == VerticalAnchor::TopAnchor)
    vOffset = -(float)m_renderSettings.fontSize;

  Directives* directives = m_renderSettings.directives ? &m_renderSettings.directives : nullptr;

  Vec2F pos = position.pos + Vec2F(hOffset, vOffset);
  if (reallyRender) {
    bool hasShadow = m_renderSettings.shadow[3] > 0;
    bool hasBackDirectives = m_renderSettings.backDirectives;
    if (hasShadow) {
      //Kae: unlike vanilla we draw only one shadow glyph instead of two, so i'm tweaking the alpha here
      Vec4B shadow = m_renderSettings.shadow;
      std::uint8_t alphaU = m_renderSettings.color[3] * byteToFloat(shadow[3]);
      if (alphaU != 255) {
        float alpha = byteToFloat(alphaU);
        shadow[3] = floatToByte(alpha * (1.5f - 0.5f * alpha));
      } else
        shadow[3] = alphaU;

      Directives const* shadowDirectives = hasBackDirectives ? &m_renderSettings.backDirectives : directives;
      renderGlyph(c, pos + Vec2F(0, -2), m_shadowPrimitives, m_renderSettings.fontSize, 1, shadow, shadowDirectives);
    }
    if (hasBackDirectives)
      renderGlyph(c, pos, m_backPrimitives, m_renderSettings.fontSize, 1, m_renderSettings.color, &m_renderSettings.backDirectives);

    auto& output = (hasShadow || hasBackDirectives) ? m_frontPrimitives : m_renderer->immediatePrimitives();
    renderGlyph(c, pos, output, m_renderSettings.fontSize, 1, m_renderSettings.color, directives);
  }

  return RectF::withSize(pos, {(float)width, (int)m_renderSettings.fontSize});
}

void TextPainter::renderPrimitives() {
  auto& destination = m_renderer->immediatePrimitives();
  std::ranges::move(m_shadowPrimitives, std::back_inserter(destination));
  m_shadowPrimitives.clear();
  std::ranges::move(m_backPrimitives, std::back_inserter(destination));
  m_backPrimitives.clear();
  std::ranges::move(m_frontPrimitives, std::back_inserter(destination));
  m_frontPrimitives.clear();
}

void TextPainter::renderGlyph(String::Char c, Vec2F const& screenPos, List<RenderPrimitive>& out, unsigned fontSize,
                              float scale, Vec4B color, Directives const* processingDirectives) {
  if (!fontSize)
    return;

  const FontTextureGroup::GlyphTexture& glyphTexture = m_fontTextureGroup.glyphTexture(c, fontSize, processingDirectives);
  if (glyphTexture.colored)
    color[0] = color[1] = color[2] = 255;
  out.emplace_back(std::in_place_type_t<RenderQuad>(),
                   glyphTexture.texture, Vec2F::round(screenPos + glyphTexture.offset * scale), scale, color, 0.0f);
}

auto TextPainter::loadFont(String const& fontPath, std::optional<String> fontName) -> Ptr<Font> {
  if (!fontName) {
    auto name = AssetPath::filename(fontPath);
    fontName.emplace(name.substr(0, name.findLast(".")));
  }

  auto assets = Root::singleton().assets();

  auto font = assets->font(fontPath)->clone();
  if (auto fontConfig = assets->json("/interface.config:font").opt(*fontName)) {
    font->setAlphaThreshold(fontConfig->getUInt("alphaThreshold", 0));
  }
  return font;
}
}// namespace Star
