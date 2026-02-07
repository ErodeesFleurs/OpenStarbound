#pragma once

#include "StarJson.hpp"
#include "StarJsonParser.hpp"

import std;

namespace Star {

class JsonBuilderStream : public JsonStream {
public:
  void beginObject() override;
  void objectKey(char32_t const* s, size_t len) override;
  void endObject() override;

  void beginArray() override;
  void endArray() override;

  void putString(char32_t const* s, size_t len) override;
  void putDouble(char32_t const* s, size_t len) override;
  void putInteger(char32_t const* s, size_t len) override;
  void putBoolean(bool b) override;
  void putNull() override;

  void putWhitespace(char32_t const* s, size_t len) override;
  void putColon() override;
  void putComma() override;

  auto stackSize() -> size_t;
  auto takeTop() -> Json;

private:
  void push(Json v);
  auto pop() -> Json;
  void set(Json v);
  void pushSentry();
  auto isSentry() -> bool;

  List<std::optional<Json>> m_stack;
};

template <typename Jsonlike>
class JsonStreamer {
public:
  static void toJsonStream(Jsonlike const& val, JsonStream& stream, bool sort);
};

template <>
class JsonStreamer<Json> {
public:
  static void toJsonStream(Json const& val, JsonStream& stream, bool sort);
};

template <typename InputIterator>
auto inputUtf8Json(InputIterator begin, InputIterator end, JsonParseType parseType) -> Json {
  using Utf32Input = U8ToU32Iterator<InputIterator>;
  using Parser = JsonParser<Utf32Input>;

  JsonBuilderStream stream;
  Parser parser(stream);
  Utf32Input wbegin(begin);
  Utf32Input wend(end);
  Utf32Input pend = parser.parse(wbegin, wend, parseType);

  if (parser.error())
    throw JsonParsingException(strf("Error parsing json: {} at {}:{}", parser.error(), parser.line(), parser.column()));
  else if (pend != wend)
    throw JsonParsingException(strf("Error extra data at end of input at {}:{}", parser.line(), parser.column()));

  return stream.takeTop();
}

template <typename OutputIterator>
void outputUtf8Json(Json const& val, OutputIterator out, int pretty, bool sort) {
  using Utf8Output = Utf8OutputIterator<OutputIterator>;
  using Writer = JsonWriter<Utf8Output>;
  Writer writer(Utf8Output(out), pretty);
  JsonStreamer<Json>::toJsonStream(val, writer, sort);
}

template <typename InputIterator, typename Stream = JsonBuilderStream, typename Jsonlike = Json>
auto inputUtf32Json(InputIterator begin, InputIterator end, JsonParseType parseType) -> Jsonlike {
  Stream stream;
  JsonParser<InputIterator> parser(stream);

  InputIterator pend = parser.parse(begin, end, parseType);

  if (parser.error()) {
    throw JsonParsingException(strf("Error parsing json: {} at {}:{}", parser.error(), parser.line(), parser.column()));
  } else if (pend != end) {
    throw JsonParsingException(strf("Error extra data at end of input at {}:{}", parser.line(), parser.column()));
  }

  return stream.takeTop();
}

template <typename OutputIterator, typename Jsonlike = Json>
void outputUtf32Json(Jsonlike const& val, OutputIterator out, int pretty, bool sort) {
  JsonWriter<OutputIterator> writer(out, pretty);
  JsonStreamer<Jsonlike>::toJsonStream(val, writer, sort);
}

}// namespace Star
