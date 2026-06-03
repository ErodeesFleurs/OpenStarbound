#pragma once

#include "StarJsonParser.hpp"
#include "StarJson.hpp"

namespace Star {

class JsonBuilderStream : public JsonStream {
public:
  virtual void beginObject();
  virtual void objectKey(char32_t const* s, size_t len);
  virtual void endObject();

  virtual void beginArray();
  virtual void endArray();

  virtual void putString(char32_t const* s, size_t len);
  virtual void putDouble(char32_t const* s, size_t len);
  virtual void putInteger(char32_t const* s, size_t len);
  virtual void putBoolean(bool b);
  virtual void putNull();

  virtual void putWhitespace(char32_t const* s, size_t len);
  virtual void putColon();
  virtual void putComma();

  size_t stackSize();
  Json takeTop();

private:
  void push(Json v);
  Json pop();
  void set(Json v);
  void pushSentry();
  bool isSentry();

  List<Maybe<Json>> m_stack;
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
Json inputUtf8Json(InputIterator begin, InputIterator end, JsonParseType parseType) {
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
Jsonlike inputUtf32Json(InputIterator begin, InputIterator end, JsonParseType parseType) {
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

}
