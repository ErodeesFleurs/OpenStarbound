import std;
import star.string;
import star.list;
import star.format;
import star.json;
import star.lexical_cast;
import star.json_parser;
import star.json_builder;

namespace Star {

std::expected<void, StarException> JsonBuilderStream::beginObject() { pushSentry(); return {}; }
std::expected<void, StarException> JsonBuilderStream::objectKey(char32_t const* s, std::size_t len) { return putString(s, len); }
std::expected<void, StarException> JsonBuilderStream::endObject() {
  JsonObject object;
  while (!isSentry()) {
    auto value = pop();
    if (!value) return std::unexpected(std::move(value.error()));
    auto keyJson = pop();
    if (!keyJson) return std::unexpected(std::move(keyJson.error()));
    auto key = keyJson->toString();
    if (!key) return std::unexpected(std::move(key.error()));
    if (!object.insert(*key, std::move(*value)).second)
      return std::unexpected(JsonParsingException(strf("Json object contains a duplicate entry for key '{}'", *key)));
  }
  return set(Json(std::move(object)));
}
std::expected<void, StarException> JsonBuilderStream::beginArray() { pushSentry(); return {}; }
std::expected<void, StarException> JsonBuilderStream::endArray() {
  JsonArray array;
  while (!isSentry()) {
    auto value = pop();
    if (!value) return std::unexpected(std::move(value.error()));
    array.append(std::move(*value));
  }
  array.reverse();
  return set(Json(std::move(array)));
}
std::expected<void, StarException> JsonBuilderStream::putString(char32_t const* s, std::size_t len) {
  auto value = Json::create(s, len);
  if (!value) return std::unexpected(std::move(value.error()));
  push(std::move(*value));
  return {};
}
std::expected<void, StarException> JsonBuilderStream::putDouble(char32_t const* s, std::size_t len) {
  auto text = String::create(s, len);
  if (!text) return std::unexpected(std::move(text.error()));
  auto value = lexicalCast<double>(*text);
  if (!value) return std::unexpected(JsonParserException("bad double", ExceptionDiagnostic(value.error())));
  push(Json(*value));
  return {};
}
std::expected<void, StarException> JsonBuilderStream::putInteger(char32_t const* s, std::size_t len) {
  auto text = String::create(s, len);
  if (!text) return std::unexpected(std::move(text.error()));
  auto value = lexicalCast<long long>(*text);
  if (!value) return std::unexpected(JsonParserException("bad integer", ExceptionDiagnostic(value.error())));
  push(Json(*value));
  return {};
}
std::expected<void, StarException> JsonBuilderStream::putBoolean(bool value) { push(Json(value)); return {}; }
std::expected<void, StarException> JsonBuilderStream::putNull() { push(Json()); return {}; }
std::expected<void, StarException> JsonBuilderStream::putWhitespace(char32_t const*, std::size_t) { return {}; }
std::expected<void, StarException> JsonBuilderStream::putColon() { return {}; }
std::expected<void, StarException> JsonBuilderStream::putComma() { return {}; }
std::size_t JsonBuilderStream::stackSize() { return m_stack.size(); }
std::expected<Json, StarException> JsonBuilderStream::takeTop() {
  if (m_stack.size() != 1 || !m_stack.back())
    return std::unexpected(JsonParsingException("Incomplete JSON builder stream"));
  return pop();
}
void JsonBuilderStream::push(Json value) { m_stack.append(std::move(value)); }
std::expected<Json, StarException> JsonBuilderStream::pop() {
  if (m_stack.empty() || !m_stack.back())
    return std::unexpected(JsonParsingException("Incomplete JSON builder stream"));
  Json value = std::move(*m_stack.back());
  m_stack.pop_back();
  return value;
}
std::expected<void, StarException> JsonBuilderStream::set(Json value) {
  if (!isSentry()) return std::unexpected(JsonParsingException("Missing JSON builder container"));
  m_stack.back() = std::move(value);
  return {};
}
void JsonBuilderStream::pushSentry() { m_stack.append({}); }
bool JsonBuilderStream::isSentry() { return !m_stack.empty() && !m_stack.back(); }

std::expected<void, StarException> JsonStreamer<Json>::toJsonStream(Json const& val, JsonStream& stream, bool sort) {
  switch (val.type()) {
    case Json::Type::Null: return stream.putNull();
    case Json::Type::Bool: {
      auto value = val.toBool();
      if (!value) return std::unexpected(std::move(value.error()));
      return stream.putBoolean(*value);
    }
    case Json::Type::Float: {
      auto value = val.toDouble();
      if (!value) return std::unexpected(std::move(value.error()));
      auto text = String(toString(*value)).wideString();
      if (!text) return std::unexpected(std::move(text.error()));
      return stream.putDouble(text->data(), text->size());
    }
    case Json::Type::Int: {
      auto value = val.toInt();
      if (!value) return std::unexpected(std::move(value.error()));
      auto text = String(toString(*value)).wideString();
      if (!text) return std::unexpected(std::move(text.error()));
      return stream.putInteger(text->data(), text->size());
    }
    case Json::Type::String: {
      auto value = val.toString();
      if (!value) return std::unexpected(std::move(value.error()));
      auto text = value->wideString();
      if (!text) return std::unexpected(std::move(text.error()));
      return stream.putString(text->data(), text->size());
    }
    case Json::Type::Array: {
      auto elements = val.iterateArray();
      if (!elements) return std::unexpected(std::move(elements.error()));
      auto begun = stream.beginArray();
      if (!begun) return std::unexpected(std::move(begun.error()));
      bool first = true;
      for (auto const& element : *elements) {
        if (!first) {
          auto comma = stream.putComma();
          if (!comma) return std::unexpected(std::move(comma.error()));
        }
        first = false;
        auto written = toJsonStream(element, stream, sort);
        if (!written) return std::unexpected(std::move(written.error()));
      }
      return stream.endArray();
    }
    case Json::Type::Object: {
      auto object = val.objectPtr();
      if (!object) return std::unexpected(std::move(object.error()));
      auto begun = stream.beginObject();
      if (!begun) return std::unexpected(std::move(begun.error()));
      auto entry = [&](auto const& pair, bool first) -> std::expected<void, StarException> {
        if (!first) {
          auto comma = stream.putComma();
          if (!comma) return std::unexpected(std::move(comma.error()));
        }
        auto key = pair.first.wideString();
        if (!key) return std::unexpected(std::move(key.error()));
        auto written = stream.objectKey(key->data(), key->size());
        if (!written) return std::unexpected(std::move(written.error()));
        written = stream.putColon();
        if (!written) return std::unexpected(std::move(written.error()));
        return toJsonStream(pair.second, stream, sort);
      };
      bool first = true;
      if (sort) {
        List<JsonObject::const_iterator> iterators;
        iterators.reserve((*object)->size());
        for (auto i = (*object)->begin(); i != (*object)->end(); ++i) iterators.append(i);
        iterators.sort([](auto a, auto b) { return a->first < b->first; });
        for (auto i : iterators) {
          auto written = entry(*i, first);
          if (!written) return std::unexpected(std::move(written.error()));
          first = false;
        }
      } else {
        for (auto const& pair : **object) {
          auto written = entry(pair, first);
          if (!written) return std::unexpected(std::move(written.error()));
          first = false;
        }
      }
      return stream.endObject();
    }
  }
  return std::unexpected(JsonException("Invalid Json type"));
}

}
