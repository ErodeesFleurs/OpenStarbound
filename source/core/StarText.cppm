module;

#include "StarString.hpp"
#include "StarVector.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarThread.hpp"
import star.directives;

namespace Star {

class Json;

inline constexpr unsigned DefaultFontSize = 8;
inline constexpr float DefaultLineSpacing = 1.3f;

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
  TextStyle& loadJson(Json const& config);
};

namespace Text {
  inline constexpr unsigned char StartEsc = '\x1b';
  inline constexpr unsigned char EndEsc = ';';
  inline constexpr unsigned char CmdEsc = '^';
  inline constexpr unsigned char SpecialCharLimit = ' ';
  extern std::string const AllEsc;
  extern std::string const AllEscEnd;

  String stripEscapeCodes(String const& s);
  inline bool isEscapeCode(Utf32Type c) { return c == CmdEsc || c == StartEsc; }

  typedef function<bool(StringView text)> TextCallback;
  typedef function<bool(StringView commands)> CommandsCallback;
  bool processText(StringView text, TextCallback textFunc, CommandsCallback commandsFunc = CommandsCallback(), bool includeCommandSides = false);
}

}

export module star.text;

export namespace Star {
  using ::Star::DefaultFontSize;
  using ::Star::DefaultLineSpacing;
  using ::Star::TextStyle;
}

export namespace Star::Text {
  using ::Star::Text::StartEsc;
  using ::Star::Text::EndEsc;
  using ::Star::Text::CmdEsc;
  using ::Star::Text::SpecialCharLimit;
  using ::Star::Text::AllEsc;
  using ::Star::Text::AllEscEnd;
  using ::Star::Text::stripEscapeCodes;
  using ::Star::Text::isEscapeCode;
  using ::Star::Text::TextCallback;
  using ::Star::Text::CommandsCallback;
  using ::Star::Text::processText;
}
