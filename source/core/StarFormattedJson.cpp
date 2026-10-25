import std;
import star.memory;
import star.string;
import star.list;
import star.format;
import star.json;
import star.lexical_cast;
import star.unicode;

import star.json_parser;
import star.json_builder;
import star.formatted_json;

namespace Star {

// Mutable state is confined to construction. Every completed FormattedJson owns
// one frozen semantic container, while duplicate object tokens remain intact and
// the semantic payload and lookup index retain their last occurrence.
class FormattedJson::Builder {
public:
  explicit Builder(bool object) : m_object(object) {
    if (object)
      m_values.emplace<JsonObject>();
  }
  Builder(Builder&&) = default;
  Builder& operator=(Builder&&) = default;
  Builder(Builder const&) = delete;
  Builder& operator=(Builder const&) = delete;

  std::expected<void, StarException> append(JsonElement elem) {
    auto loc = m_formatted.m_elements.size();
    if (std::holds_alternative<ObjectKeyElement>(elem)) {
      if (!m_object)
        return std::unexpected(JsonException("Invalid FormattedJson object key outside object"));
      m_lastKey = loc;
    } else if (std::holds_alternative<ValueElement>(elem)) {
      auto const& value = std::get<ValueElement>(elem);
      if (!value.value)
        return std::unexpected(JsonException("Invalid empty FormattedJson value element"));
      if (m_lastKey) {
        if (!m_object || *m_lastKey >= m_formatted.m_elements.size())
          return std::unexpected(JsonException("Invalid FormattedJson object entry"));
        auto const& keyElement = m_formatted.m_elements[*m_lastKey];
        if (!std::holds_alternative<ObjectKeyElement>(keyElement))
          return std::unexpected(JsonException("Invalid FormattedJson object key element"));
        auto const& key = std::get<ObjectKeyElement>(keyElement);
        std::get<JsonObject>(m_values)[key.key] = value.value->toJson();
        m_formatted.m_objectEntryLocations[key.key] = {*m_lastKey, loc};
        m_lastKey.reset();
      } else {
        if (m_object)
          return std::unexpected(JsonException("Invalid FormattedJson array entry"));
        std::get<JsonArray>(m_values).append(value.value->toJson());
        m_formatted.m_arrayElementLocations.append(loc);
      }
    }
    m_formatted.m_elements.append(std::move(elem));
    return {};
  }

  FormattedJson finish() {
    m_formatted.m_jsonValue = std::visit([](auto& values) { return Json(std::move(values)); }, m_values);
    return std::move(m_formatted);
  }

private:
  bool m_object;
  FormattedJson m_formatted;
  std::variant<JsonArray, JsonObject> m_values;
  std::optional<ElementLocation> m_lastKey;
};

class FormattedJsonBuilderStream : public JsonStream {
public:
  std::expected<void, StarException> beginObject() override;
  std::expected<void, StarException> objectKey(std::span<String::Char const> s) override;
  std::expected<void, StarException> objectKeyUtf8(std::string_view s) override;
  std::expected<void, StarException> endObject() override;
  std::expected<void, StarException> beginArray() override;
  std::expected<void, StarException> endArray() override;
  std::expected<void, StarException> putString(std::span<String::Char const> s) override;
  std::expected<void, StarException> putStringUtf8(std::string_view s) override;
  std::expected<void, StarException> putDouble(std::string_view s) override;
  std::expected<void, StarException> putInteger(std::string_view s) override;
  std::expected<void, StarException> putBoolean(bool b) override;
  std::expected<void, StarException> putNull() override;
  std::expected<void, StarException> putWhitespace(std::span<String::Char const> s) override;
  std::expected<void, StarException> putComma() override;
  std::expected<void, StarException> putColon() override;
  std::expected<FormattedJson, StarException> takeTop();
private:
  void push(bool object);
  std::expected<FormattedJson, StarException> pop();
  std::expected<std::reference_wrapper<FormattedJson::Builder>, StarException> current();
  std::expected<void, StarException> putValue(Json const& value, std::optional<String> formatting = {});
  std::optional<FormattedJson> m_root;
  List<FormattedJson::Builder> m_stack;
};

template <>
class JsonStreamer<FormattedJson> {
public:
  static std::expected<void, StarException> toJsonStream(FormattedJson const& val, JsonStream& stream, bool sort);
};

ValueElement::ValueElement(FormattedJson json) : value(std::make_shared<FormattedJson>(std::move(json))) {}

bool ValueElement::operator==(ValueElement const& v) const {
  return *value == *v.value;
}

bool ObjectKeyElement::operator==(ObjectKeyElement const& v) const {
  return key == v.key;
}

bool WhitespaceElement::operator==(WhitespaceElement const& v) const {
  return whitespace == v.whitespace;
}

bool ColonElement::operator==(ColonElement const&) const {
  return true;
}

bool CommaElement::operator==(CommaElement const&) const {
  return true;
}

std::expected<FormattedJson, StarException> FormattedJson::parse(String const& string) {
  auto const& bytes = string.utf8();
  return inputUtf8Json<std::string::const_iterator, FormattedJsonBuilderStream, FormattedJson>(
      bytes.begin(), bytes.end(), JsonParseType::Value);
}

std::expected<FormattedJson, StarException> FormattedJson::parseJson(String const& string) {
  auto const& bytes = string.utf8();
  return inputUtf8Json<std::string::const_iterator, FormattedJsonBuilderStream, FormattedJson>(
      bytes.begin(), bytes.end(), JsonParseType::Top);
}

FormattedJson FormattedJson::ofType(Json::Type type) {
  FormattedJson json;
  json.m_jsonValue = Json::ofType(type);
  return json;
}

FormattedJson::FormattedJson() = default;

FormattedJson::FormattedJson(FormattedJson&& other)
    : m_jsonValue(std::move(other.m_jsonValue)), m_elements(std::move(other.m_elements)),
      m_formatting(std::move(other.m_formatting)),
      m_objectEntryLocations((other.m_formatting.reset(), std::move(other.m_objectEntryLocations))),
      m_arrayElementLocations(std::move(other.m_arrayElementLocations)) {}

FormattedJson& FormattedJson::operator=(FormattedJson&& other) {
  if (this != &other) {
    m_jsonValue = std::move(other.m_jsonValue);
    m_elements = std::move(other.m_elements);
    m_formatting = std::move(other.m_formatting);
    other.m_formatting.reset();
    m_objectEntryLocations = std::move(other.m_objectEntryLocations);
    m_arrayElementLocations = std::move(other.m_arrayElementLocations);
  }
  return *this;
}

std::expected<FormattedJson, StarException> FormattedJson::create(Json const& json) {
  if (json.type() == Json::Type::Object || json.type() == Json::Type::Array) {
    FormattedJsonBuilderStream stream;
    auto written = JsonStreamer<Json>::toJsonStream(json, stream, false);
    if (!written)
      return std::unexpected(std::move(written.error()));
    auto parsed = stream.takeTop();
    if (!parsed)
      return std::unexpected(std::move(parsed.error()));
    // Preserve the already-owned JSON payload, including exact numeric values.
    parsed->m_jsonValue = json;
    return parsed;
  }
  FormattedJson result;
  result.m_jsonValue = json;
  return result;
}

Json const& FormattedJson::toJson() const {
  return m_jsonValue;
}

std::expected<FormattedJson, StarException> FormattedJson::get(String const& key) const {
  if (type() != Json::Type::Object)
    return std::unexpected(JsonException::format("Cannot call get with key on FormattedJson type {}, must be Object type", typeName()));
  auto entry = m_objectEntryLocations.maybe(key);
  if (!entry)
    return std::unexpected(JsonException::format("No such key in FormattedJson::get(\"{}\")", key));
  auto value = getFormattedJson(entry->second);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->get();
}

std::expected<FormattedJson, StarException> FormattedJson::get(std::size_t index) const {
  if (type() != Json::Type::Array)
    return std::unexpected(JsonException::format("Cannot call get with index on FormattedJson type {}, must be Array type", typeName()));
  if (index >= m_arrayElementLocations.size())
    return std::unexpected(JsonException::format("FormattedJson::get({}) out of range", index));
  auto value = getFormattedJson(m_arrayElementLocations[index]);
  if (!value)
    return std::unexpected(std::move(value.error()));
  return value->get();
}

struct WhitespaceStyle {
  String beforeKey;
  String beforeColon;
  String beforeValue;
  String beforeComma;
};

template <class ElementType>
FormattedJson::ElementLocation indexOf(FormattedJson::ElementList const& elements, FormattedJson::ElementLocation pos) {
  for (; pos < elements.size(); ++pos) {
    if (std::holds_alternative<ElementType>(elements[pos]))
      return pos;
  }
  return NPos;
}

template <class ElementType>
FormattedJson::ElementLocation lastIndexOf(
    FormattedJson::ElementList const& elements, FormattedJson::ElementLocation pos) {
  while (pos > 0) {
    --pos;
    if (std::holds_alternative<ElementType>(elements[pos]))
      return pos;
  }
  return NPos;
}

String concatWhitespace(FormattedJson::ElementList const& elements, FormattedJson::ElementLocation from,
    FormattedJson::ElementLocation to) {
  String whitespace;
  for (auto i = from; i < to; ++i) {
    if (std::holds_alternative<WhitespaceElement>(elements[i]))
      whitespace += std::get<WhitespaceElement>(elements[i]).whitespace;
  }
  return whitespace;
}

std::expected<WhitespaceStyle, StarException> detectWhitespace(FormattedJson::ElementList const& elements,
    FormattedJson::ElementLocation insertLoc, bool array) {
  WhitespaceStyle style;

  // Find a nearby value as a reference location to learn whitespace from.
  FormattedJson::ElementLocation valueLoc = lastIndexOf<ValueElement>(elements, insertLoc);
  if (valueLoc == NPos)
    valueLoc = indexOf<ValueElement>(elements, insertLoc);

  if (valueLoc == NPos) {
    // This object/array is empty. Pre-key/value whitespace will be the total of
    // the whitespace already present, plus some guessed indentation if it
    // contained a newline.
    String beforeValue = concatWhitespace(elements, 0, elements.size());
    auto newline = beforeValue.find('\n');
    if (!newline)
      return std::unexpected(std::move(newline.error()));
    if (*newline != NPos)
      beforeValue += "  ";
    if (array)
      return WhitespaceStyle{"", "", beforeValue, ""};
    return WhitespaceStyle{beforeValue, "", "", ""};
  }

  FormattedJson::ElementLocation commaLoc = indexOf<CommaElement>(elements, valueLoc);
  if (commaLoc != NPos) {
    style.beforeComma = concatWhitespace(elements, valueLoc + 1, commaLoc);
  }

  FormattedJson::ElementLocation colonLoc = lastIndexOf<ColonElement>(elements, valueLoc);
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed(((colonLoc == NPos) == array));
  }
  if (colonLoc != NPos) {
    style.beforeValue = concatWhitespace(elements, colonLoc + 1, valueLoc);

    FormattedJson::ElementLocation keyLoc = lastIndexOf<ObjectKeyElement>(elements, colonLoc);
    if constexpr (Star::DebugEnabled) {
      Star::starAssertFailed((keyLoc != NPos));
    }
    style.beforeColon = concatWhitespace(elements, keyLoc + 1, colonLoc);

    FormattedJson::ElementLocation prevValueLoc = lastIndexOf<ValueElement>(elements, keyLoc);
    if (prevValueLoc == NPos)
      prevValueLoc = 0;
    style.beforeKey = concatWhitespace(elements, prevValueLoc, keyLoc);

  } else {
    FormattedJson::ElementLocation prevValueLoc = lastIndexOf<ValueElement>(elements, valueLoc);
    if (prevValueLoc == NPos)
      prevValueLoc = 0;
    style.beforeValue = concatWhitespace(elements, prevValueLoc, valueLoc);
  }

  return style;
}

void insertWhitespace(FormattedJson::ElementList& destination, FormattedJson::ElementLocation& at, String const& whitespace) {
  if (whitespace == "")
    return;
  destination.insertAt(at++, WhitespaceElement{whitespace});
}

void insertWithWhitespace(FormattedJson::ElementList& destination, WhitespaceStyle const& style,
    FormattedJson::ElementLocation& at, JsonElement const& element) {
  if (std::holds_alternative<ValueElement>(element))
    insertWhitespace(destination, at, style.beforeValue);
  if (std::holds_alternative<ObjectKeyElement>(element))
    insertWhitespace(destination, at, style.beforeKey);
  if (std::holds_alternative<ColonElement>(element))
    insertWhitespace(destination, at, style.beforeColon);
  if (std::holds_alternative<CommaElement>(element))
    insertWhitespace(destination, at, style.beforeComma);
  destination.insertAt(at++, element);
}

std::expected<void, StarException> insertWithCommaAndFormatting(FormattedJson::ElementList& destination, FormattedJson::ElementLocation at,
    bool array, FormattedJson::ElementList const& elements) {
  // Find the previous value we're inserting after, if any.
  at = lastIndexOf<ValueElement>(destination, at);
  if (at == NPos)
    at = 0;
  else
    at += 1;
  bool empty = lastIndexOf<ValueElement>(destination, destination.size()) == NPos;
  bool appendComma = at == 0 && !empty;
  bool prependComma = !appendComma && !empty;

  auto style = detectWhitespace(destination, at, array);
  if (!style)
    return std::unexpected(std::move(style.error()));

  if (prependComma) {
    // Inserting into a non-empty object/array. Prepend a comma
    insertWithWhitespace(destination, *style, at, CommaElement{});
  }
  for (JsonElement const& elem : elements) {
    insertWithWhitespace(destination, *style, at, elem);
  }
  if (appendComma) {
    insertWithWhitespace(destination, *style, at, CommaElement{});
  }
  return {};
}

std::expected<FormattedJson, StarException> FormattedJson::prepend(String const& key, FormattedJson const& value) const {
  return objectInsert(key, value, 0);
}

std::expected<FormattedJson, StarException> FormattedJson::insertBefore(String const& key, FormattedJson const& value, String const& beforeKey) const {
  auto entry = m_objectEntryLocations.maybeRef(beforeKey);
  if (!entry)
    return std::unexpected(JsonException::format("Cannot insert before key \"{}\", which does not exist", beforeKey));
  return objectInsert(key, value, entry->get().first);
}

std::expected<FormattedJson, StarException> FormattedJson::insertAfter(String const& key, FormattedJson const& value, String const& afterKey) const {
  auto entry = m_objectEntryLocations.maybeRef(afterKey);
  if (!entry)
    return std::unexpected(JsonException::format("Cannot insert after key \"{}\", which does not exist", afterKey));
  return objectInsert(key, value, entry->get().second + 1);
}

std::expected<FormattedJson, StarException> FormattedJson::append(String const& key, FormattedJson const& value) const {
  return objectInsert(key, value, m_elements.size());
}

std::expected<FormattedJson, StarException> FormattedJson::set(String const& key, FormattedJson const& value) const {
  return objectInsert(key, value, m_elements.size());
}

void removeValueFromArray(List<JsonElement>& elements, std::size_t loc) {
  // Remove the value itself, the comma following and the whitespace up to the
  // next value.
  // If it's the last value, it removes the value, and the preceding whitespace
  // and comma.
  std::size_t commaLoc = elements.indexOf(CommaElement{}, loc);
  if (commaLoc != NPos) {
    elements.eraseAt(loc, commaLoc + 1);
    while (loc < elements.size() && std::holds_alternative<WhitespaceElement>(elements[loc]))
      elements.eraseAt(loc);
  } else {
    commaLoc = elements.lastIndexOf(CommaElement{}, loc);
    if (commaLoc == NPos)
      commaLoc = 0;
    elements.eraseAt(commaLoc, loc + 1);
  }
}

std::expected<FormattedJson, StarException> FormattedJson::eraseKey(String const& key) const {
  if (type() != Json::Type::Object)
    return std::unexpected(JsonException::format("Cannot call erase with key on FormattedJson type {}, must be Object type", typeName()));

  std::optional<std::pair<ElementLocation, ElementLocation>> maybeEntry = m_objectEntryLocations.maybe(key);
  if (!maybeEntry.has_value())
    return *this;

  ElementLocation loc = maybeEntry->first;
  ElementList elements = m_elements;
  elements.eraseAt(loc, maybeEntry->second); // Remove key, colon and whitespace up to the value
  removeValueFromArray(elements, loc);
  return object(std::move(elements));
}

std::expected<FormattedJson, StarException> FormattedJson::insert(std::size_t index, FormattedJson const& value) const {
  if (type() != Json::Type::Array)
    return std::unexpected(JsonException::format("Cannot call insert with index on FormattedJson type {}, must be Array type", typeName()));
  if (index > m_arrayElementLocations.size())
    return std::unexpected(JsonException::format("FormattedJson::insert({}) out of range", index));
  ElementList elements = m_elements;
  ElementLocation insertPosition = index < m_arrayElementLocations.size() ? m_arrayElementLocations[index] : elements.size();
  auto inserted = insertWithCommaAndFormatting(elements, insertPosition, true, {ValueElement{value}});
  if (!inserted)
    return std::unexpected(std::move(inserted.error()));
  return array(std::move(elements));
}

std::expected<FormattedJson, StarException> FormattedJson::append(FormattedJson const& value) const {
  if (type() != Json::Type::Array)
    return std::unexpected(JsonException::format("Cannot call append on FormattedJson type {}, must be Array type", typeName()));
  ElementList elements = m_elements;
  auto inserted = insertWithCommaAndFormatting(elements, elements.size(), true, {ValueElement{value}});
  if (!inserted)
    return std::unexpected(std::move(inserted.error()));
  return array(std::move(elements));
}

std::expected<FormattedJson, StarException> FormattedJson::set(std::size_t index, FormattedJson const& value) const {
  if (type() != Json::Type::Array)
    return std::unexpected(JsonException::format("Cannot call set with index on FormattedJson type {}, must be Array type", typeName()));
  if (index >= m_arrayElementLocations.size())
    return std::unexpected(JsonException::format("FormattedJson::set({}) out of range", index));
  ElementList elements = m_elements;
  elements[m_arrayElementLocations[index]] = ValueElement{value};
  return array(std::move(elements));
}

std::expected<FormattedJson, StarException> FormattedJson::eraseIndex(std::size_t index) const {
  if (type() != Json::Type::Array)
    return std::unexpected(JsonException::format("Cannot call set with index on FormattedJson type {}, must be Array type", typeName()));
  if (index >= m_arrayElementLocations.size())
    return std::unexpected(JsonException::format("FormattedJson::eraseIndex({}) out of range", index));
  ElementList elements = m_elements;
  removeValueFromArray(elements, m_arrayElementLocations[index]);
  return array(std::move(elements));
}

std::expected<std::size_t, StarException> FormattedJson::size() const {
  return m_jsonValue.size();
}

std::expected<bool, StarException> FormattedJson::contains(String const& key) const {
  return m_jsonValue.contains(key);
}

Json::Type FormattedJson::type() const {
  return m_jsonValue.type();
}

bool FormattedJson::isType(Json::Type type) const {
  return m_jsonValue.isType(type);
}

String FormattedJson::typeName() const {
  return m_jsonValue.typeName();
}

std::expected<String, StarException> FormattedJson::toFormattedDouble() const {
  if (!isType(Json::Type::Float))
    return std::unexpected(JsonException::format("Cannot call toFormattedDouble on Json type {}, must be Float", typeName()));
  if (m_formatting.has_value())
    return *m_formatting;
  return toJson().repr();
}

std::expected<String, StarException> FormattedJson::toFormattedInt() const {
  if (!isType(Json::Type::Int))
    return std::unexpected(JsonException::format("Cannot call toFormattedInt on Json type {}, must be Int", typeName()));
  if (m_formatting.has_value())
    return *m_formatting;
  return toJson().repr();
}

std::expected<String, StarException> FormattedJson::repr() const {
  if (m_formatting.has_value())
    return *m_formatting;
  std::string result;
  JsonWriter<std::back_insert_iterator<std::string>, true> writer(std::back_inserter(result));
  auto written = JsonStreamer<FormattedJson>::toJsonStream(*this, writer, false);
  if (!written)
    return std::unexpected(std::move(written.error()));
  return String(std::move(result));
}

std::expected<String, StarException> FormattedJson::printJson() const {
  if (type() != Json::Type::Object && type() != Json::Type::Array)
    return std::unexpected(JsonException("printJson called on non-top-level JSON type"));
  return repr();
}

FormattedJson::ElementList const& FormattedJson::elements() const {
  return m_elements;
}

bool FormattedJson::operator==(FormattedJson const& v) const {
  return m_jsonValue == v.m_jsonValue;
}

bool FormattedJson::operator!=(FormattedJson const& v) const {
  return !(*this == v);
}

std::expected<FormattedJson, StarException> FormattedJson::object(ElementList elements) {
  Builder builder(true);
  for (auto& elem : elements) {
    auto appended = builder.append(std::move(elem));
    if (!appended)
      return std::unexpected(std::move(appended.error()));
  }
  return builder.finish();
}

std::expected<FormattedJson, StarException> FormattedJson::array(ElementList elements) {
  Builder builder(false);
  for (auto& elem : elements) {
    if (std::holds_alternative<ColonElement>(elem) || std::holds_alternative<ObjectKeyElement>(elem))
      return std::unexpected(JsonException("Invalid FormattedJson element in Json array"));
    auto appended = builder.append(std::move(elem));
    if (!appended)
      return std::unexpected(std::move(appended.error()));
  }
  return builder.finish();
}

std::expected<FormattedJson, StarException> FormattedJson::objectInsert(String const& key, FormattedJson const& value, ElementLocation loc) const {
  if (type() != Json::Type::Object)
    return std::unexpected(JsonException::format("Cannot call set with key on FormattedJson type {}, must be Object type", typeName()));
  auto entry = m_objectEntryLocations.maybe(key);
  ElementList elements = m_elements;
  if (entry) {
    elements[entry->second] = ValueElement{value};
  } else {
    auto inserted = insertWithCommaAndFormatting(elements, loc, false, {ObjectKeyElement{key}, ColonElement{}, ValueElement{value}});
    if (!inserted)
      return std::unexpected(std::move(inserted.error()));
  }
  return object(std::move(elements));
}

std::expected<std::reference_wrapper<FormattedJson const>, StarException> FormattedJson::getFormattedJson(ElementLocation loc) const {
  if (loc >= m_elements.size())
    return std::unexpected(JsonException("FormattedJson element location out of range"));
  if (!std::holds_alternative<ValueElement>(m_elements[loc]))
    return std::unexpected(JsonException("Invalid FormattedJson value element"));
  auto const& value = std::get<ValueElement>(m_elements[loc]).value;
  if (!value)
    return std::unexpected(JsonException("Invalid FormattedJson value element"));
  return std::cref(*value);
}


std::expected<void, StarException> FormattedJsonBuilderStream::beginObject() {
  push(true);
  return {};
}

std::expected<void, StarException> FormattedJsonBuilderStream::objectKey(std::span<String::Char const> s) {
  auto key = String::create(std::u32string_view(s.data(), s.size()));
  if (!key)
    return std::unexpected(std::move(key.error()));
  auto container = current();
  if (!container)
    return std::unexpected(std::move(container.error()));
  return container->get().append(ObjectKeyElement{std::move(*key)});
}

std::expected<void, StarException> FormattedJsonBuilderStream::objectKeyUtf8(std::string_view s) {
  auto valid = utf8Length(s.data(), s.size());
  if (!valid)
    return std::unexpected(std::move(valid.error()));
  auto container = current();
  if (!container)
    return std::unexpected(std::move(container.error()));
  return container->get().append(ObjectKeyElement{String(s)});
}

std::expected<void, StarException> FormattedJsonBuilderStream::endObject() {
  auto value = pop();
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (!m_stack.empty())
    return m_stack.back().append(ValueElement{std::move(*value)});
  m_root = std::move(*value);
  return {};
}

std::expected<void, StarException> FormattedJsonBuilderStream::beginArray() {
  push(false);
  return {};
}

std::expected<void, StarException> FormattedJsonBuilderStream::endArray() {
  return endObject();
}

std::expected<void, StarException> FormattedJsonBuilderStream::putString(std::span<String::Char const> s) {
  auto text = String::create(std::u32string_view(s.data(), s.size()));
  if (!text)
    return std::unexpected(std::move(text.error()));
  return putValue(Json(std::move(*text)));
}

std::expected<void, StarException> FormattedJsonBuilderStream::putStringUtf8(std::string_view s) {
  auto valid = utf8Length(s.data(), s.size());
  if (!valid)
    return std::unexpected(std::move(valid.error()));
  return putValue(Json(String(s)));
}

std::expected<void, StarException> FormattedJsonBuilderStream::putDouble(std::string_view s) {
  auto number = lexicalCast<double>(s);
  if (!number)
    return std::unexpected(JsonParserException("bad double", ExceptionDiagnostic(number.error())));
  return putValue(Json(*number), String(s));
}

std::expected<void, StarException> FormattedJsonBuilderStream::putInteger(std::string_view s) {
  auto number = lexicalCast<long long>(s);
  if (!number)
    return std::unexpected(JsonParserException("bad integer", ExceptionDiagnostic(number.error())));
  return putValue(Json(*number), String(s));
}

std::expected<void, StarException> FormattedJsonBuilderStream::putBoolean(bool b) {
  return putValue(Json(b));
}

std::expected<void, StarException> FormattedJsonBuilderStream::putNull() {
  return putValue(Json());
}

std::expected<void, StarException> FormattedJsonBuilderStream::putWhitespace(std::span<String::Char const> s) {
  if (m_stack.empty())
    return {};
  auto whitespace = String::create(std::u32string_view(s.data(), s.size()));
  if (!whitespace)
    return std::unexpected(std::move(whitespace.error()));
  return m_stack.back().append(WhitespaceElement{std::move(*whitespace)});
}

std::expected<void, StarException> FormattedJsonBuilderStream::putColon() {
  auto container = current();
  if (!container)
    return std::unexpected(std::move(container.error()));
  return container->get().append(ColonElement{});
}

std::expected<void, StarException> FormattedJsonBuilderStream::putComma() {
  auto container = current();
  if (!container)
    return std::unexpected(std::move(container.error()));
  return container->get().append(CommaElement{});
}

std::expected<FormattedJson, StarException> FormattedJsonBuilderStream::takeTop() {
  if (!m_root || !m_stack.empty())
    return std::unexpected(JsonParsingException("Incomplete formatted JSON builder stream"));
  FormattedJson result = std::move(*m_root);
  m_root.reset();
  return result;
}

void FormattedJsonBuilderStream::push(bool object) { m_stack.emplace_back(object); }

std::expected<FormattedJson, StarException> FormattedJsonBuilderStream::pop() {
  if (m_stack.empty())
    return std::unexpected(JsonParsingException("Missing formatted JSON builder container"));
  FormattedJson result = m_stack.back().finish();
  m_stack.pop_back();
  return result;
}

std::expected<std::reference_wrapper<FormattedJson::Builder>, StarException> FormattedJsonBuilderStream::current() {
  if (m_stack.empty())
    return std::unexpected(JsonParsingException("Missing formatted JSON builder container"));
  return std::ref(m_stack.back());
}

std::expected<void, StarException> FormattedJsonBuilderStream::putValue(Json const& value, std::optional<String> formatting) {
  auto formatted = FormattedJson::create(value);
  if (!formatted)
    return std::unexpected(std::move(formatted.error()));
  if (formatting) {
    if constexpr (DebugEnabled) {
      auto parsed = Json::parse(*formatting);
      if (!parsed)
        return std::unexpected(std::move(parsed.error()));
      starAssertFailed(*parsed == formatted->toJson());
    }
    formatted->m_formatting = std::move(*formatting);
  }
  if (!m_stack.empty())
    return m_stack.back().append(ValueElement{std::move(*formatted)});
  m_root = std::move(*formatted);
  return {};
}

std::expected<void, StarException> JsonStreamer<FormattedJson>::toJsonStream(FormattedJson const& val, JsonStream& stream, bool sort) {
  bool object = val.isType(Json::Type::Object);
  bool array = val.isType(Json::Type::Array);
  if (!object && !array) {
    if (val.isType(Json::Type::Float) || val.isType(Json::Type::Int)) {
      auto formatted = val.isType(Json::Type::Float) ? val.toFormattedDouble() : val.toFormattedInt();
      if (!formatted)
        return std::unexpected(std::move(formatted.error()));
      return val.isType(Json::Type::Float) ? stream.putDouble(formatted->utf8()) : stream.putInteger(formatted->utf8());
    }
    return JsonStreamer<Json>::toJsonStream(val.toJson(), stream, sort);
  }
  auto begun = object ? stream.beginObject() : stream.beginArray();
  if (!begun)
    return std::unexpected(std::move(begun.error()));
  for (auto const& elem : val.elements()) {
    std::expected<void, StarException> written;
    if (std::holds_alternative<ObjectKeyElement>(elem)) {
      written = stream.objectKeyUtf8(std::get<ObjectKeyElement>(elem).key.utf8());
    } else if (std::holds_alternative<WhitespaceElement>(elem)) {
      auto text = std::get<WhitespaceElement>(elem).whitespace.wideString();
      if (!text)
        return std::unexpected(std::move(text.error()));
      written = stream.putWhitespace(*text);
    } else if (std::holds_alternative<ColonElement>(elem)) {
      written = stream.putColon();
    } else if (std::holds_alternative<CommaElement>(elem)) {
      written = stream.putComma();
    } else {
      auto const& value = std::get<ValueElement>(elem).value;
      if (!value)
        return std::unexpected(JsonException("Invalid empty FormattedJson value element"));
      written = toJsonStream(*value, stream, sort);
    }
    if (!written)
      return std::unexpected(std::move(written.error()));
  }
  return object ? stream.endObject() : stream.endArray();
}

std::expected<std::reference_wrapper<std::ostream>, StarException> operator<<(std::ostream& os, JsonElement const& elem) {
  if (std::holds_alternative<ValueElement>(elem))
    os << "ValueElement{" << std::get<ValueElement>(elem).value << "}";
  else if (std::holds_alternative<ObjectKeyElement>(elem))
    os << "ObjectKeyElement{" << std::get<ObjectKeyElement>(elem).key << "}";
  else if (std::holds_alternative<WhitespaceElement>(elem))
    os << "WhitespaceElement{" << std::get<WhitespaceElement>(elem).whitespace << "}";
  else if (std::holds_alternative<ColonElement>(elem))
    os << "ColonElement{}";
  else
    os << "CommaElement{}";
  if (!os)
    return std::unexpected(JsonException("Error writing formatted JSON element"));
  return std::ref(os);
}

std::expected<std::reference_wrapper<std::ostream>, StarException> operator<<(std::ostream& os, FormattedJson const& json) {
  auto text = json.repr();
  if (!text)
    return std::unexpected(std::move(text.error()));
  os << *text;
  if (!os)
    return std::unexpected(JsonException("Error writing formatted JSON"));
  return std::ref(os);
}

}
