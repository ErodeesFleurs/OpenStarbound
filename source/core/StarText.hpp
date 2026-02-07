#pragma once

#include "StarDirectives.hpp"
#include "StarJson.hpp"
#include "StarString.hpp"
#include "StarStringView.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

unsigned const DefaultFontSize = 8;
float const DefaultLineSpacing = 1.3f;

struct TextStyle {
  float lineSpacing = DefaultLineSpacing;
  Vec4B color = Vec4B::filled(255);
  Vec4B shadow = Vec4B::filled(0);
  unsigned fontSize = DefaultFontSize;
  String font = "";
  Directives directives;
  Directives backDirectives;

  TextStyle() = default;
  TextStyle(Json const& config);
  auto loadJson(Json const& config) -> TextStyle&;
};

namespace Text {
unsigned char const StartEsc = '\x1b';
unsigned char const EndEsc = ';';
unsigned char const CmdEsc = '^';
unsigned char const SpecialCharLimit = ' ';
extern std::string const AllEsc;
extern std::string const AllEscEnd;

auto stripEscapeCodes(String const& s) -> String;
inline auto isEscapeCode(Utf32Type c) -> bool { return c == CmdEsc || c == StartEsc; }

using TextCallback = std::function<bool(StringView text)>;
using CommandsCallback = std::function<bool(StringView commands)>;
auto processText(StringView text, TextCallback textFunc, CommandsCallback commandsFunc = CommandsCallback(), bool includeCommandSides = false) -> bool;
auto preprocessEscapeCodes(String const& s) -> String;
auto extractCodes(String const& s) -> String;
}// namespace Text

}// namespace Star
