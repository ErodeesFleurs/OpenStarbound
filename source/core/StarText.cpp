import std;
import star.memory;
import star.format;
import star.json;
#include "StarJsonExtra.hpp"
import star.string;
import star.string_view;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.thread;
import star.directives;
import star.text;
#include <re2/re2.h>

namespace Star {

TextStyle::TextStyle(Json const& config) : TextStyle() {
  if (config.isType(Json::Type::String))
    font = config.toString();
  else
    loadJson(config);
}
TextStyle& TextStyle::loadJson(Json const& config) {
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
  extern std::string const AllEsc = strf("{:c}{:c}", CmdEsc, StartEsc);
  extern std::string const AllEscEnd = strf("{:c}{:c}{:c}", CmdEsc, StartEsc, EndEsc);

  static RE2 stripEscapeRegex = strf("\\{:c}[^;{:c}]*{:c}", CmdEsc, CmdEsc, EndEsc);
  String stripEscapeCodes(String const& s) {
    if (s.empty())
      return s;
    std::string result = s.utf8();
    RE2::GlobalReplace(&result, stripEscapeRegex, "");
    return String(std::move(result));
  }

  bool processText(StringView text, TextCallback textFunc, CommandsCallback commandsFunc, bool includeCommandSides) {
    std::string_view str = text.utf8();
    std::size_t start = 0;
    while (true) {
      std::size_t escape = str.find_first_of(AllEsc, start);
      start = 0;
      if (escape != NPos) {
        escape = str.find_first_not_of(AllEsc, escape) - 1; // jump to the last ^
        std::size_t end = str.find_first_of(AllEscEnd, escape + 1);
        if (end != NPos) {
          if (str.at(end) != EndEsc) {
            start = end;
            continue;
          }
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
}

}