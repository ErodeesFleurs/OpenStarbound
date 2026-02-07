#include "StarText.hpp"
#include "StarJsonExtra.hpp"

import re2_module;
import std;

namespace Star {

TextStyle::TextStyle(Json const& config) : TextStyle() {
  if (config.isType(Json::Type::String))
    font = config.toString();
  else
    loadJson(config);
}
auto TextStyle::loadJson(Json const& config) -> TextStyle& {
  if (!config)
    return *this;

  lineSpacing = config.getFloat("lineSpacing", lineSpacing);
  if (auto jColor = config.opt("color"))
    color = jsonToColor(*jColor).toRgba();
  if (auto jShadow = config.opt("shadow"))
    shadow = jsonToColor(*jShadow).toRgba();
  fontSize = config.getUInt("fontSize", fontSize);
  if (auto jFont = config.optString("font"))
    font = *jFont;
  if (auto jDirectives = config.optString("directives"))
    directives = *jDirectives;
  if (auto jBackDirectives = config.optString("backDirectives"))
    backDirectives = *jBackDirectives;

  return *this;
}

namespace Text {
std::string const AllEsc = strf("{:c}{:c}", CmdEsc, StartEsc);
std::string const AllEscEnd = strf("{:c}{:c}{:c}", CmdEsc, StartEsc, EndEsc);

static re2::RE2 stripEscapeRegex = strf("\\{:c}[^;]*{:c}", CmdEsc, EndEsc);
auto stripEscapeCodes(String const& s) -> String {
  if (s.empty())
    return s;
  std::string result = s.utf8();
  re2::RE2::GlobalReplace(&result, stripEscapeRegex, "");
  return {std::move(result)};
}

auto processText(StringView text, TextCallback textFunc, CommandsCallback commandsFunc, bool includeCommandSides) -> bool {
  std::string_view str = text.utf8();
  while (true) {
    std::size_t escape = str.find_first_of(AllEsc);
    if (escape != std::numeric_limits<std::size_t>::max()) {
      escape = str.find_first_not_of(AllEsc, escape) - 1;// jump to the last ^

      std::size_t end = str.find_first_of(EndEsc, escape);
      if (end != std::numeric_limits<std::size_t>::max()) {
        if (escape && !textFunc(str.substr(0, escape)))
          return false;
        if (commandsFunc) {
          StringView commands = includeCommandSides
            ? str.substr(escape, end - escape + 1)
            : str.substr(escape + 1, end - escape - 1);
          if (!commands.empty() && !commandsFunc(commands))
            return false;
        }
        str = str.substr(end + 1);
        continue;
      }
    }

    if (!str.empty())
      return textFunc(str);

    return true;
  }
}

// The below two functions aren't used anymore, not bothering with StringView for them
auto preprocessEscapeCodes(String const& s) -> String {
  bool escape = false;
  std::string result = s.utf8();

  std::size_t escapeStartIdx = 0;
  for (std::size_t i = 0; i < result.size(); i++) {
    auto& c = result[i];
    if (isEscapeCode(c)) {
      escape = true;
      escapeStartIdx = i;
    }
    if ((c <= SpecialCharLimit) && !(c == StartEsc))
      escape = false;
    if ((c == EndEsc) && escape)
      result[escapeStartIdx] = StartEsc;
  }
  return {result};
}

auto extractCodes(String const& s) -> String {
  bool escape = false;
  StringList result;
  String escapeCode;
  for (auto c : preprocessEscapeCodes(s)) {
    if (c == StartEsc)
      escape = true;
    if (c == EndEsc) {
      escape = false;
      for (auto command : escapeCode.split(','))
        result.append(command);
      escapeCode = "";
    }
    if (escape && (c != StartEsc))
      escapeCode.append(c);
  }
  if (!result.size())
    return "";
  return "^" + result.join(",") + ";";
}
}// namespace Text

}// namespace Star
