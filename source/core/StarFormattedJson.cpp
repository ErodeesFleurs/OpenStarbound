import std;
import star.memory;
import star.string;
import star.list;
import star.format;
import star.json;
import star.lexical_cast;

import star.json_parser;
import star.json_builder;
import star.formatted_json;

namespace Star {

class FormattedJsonBuilderStream : public JsonStream {
public:
  std::expected<void, StarException> beginObject() override;
  std::expected<void, StarException> objectKey(std::span<String::Char const> s) override;
  std::expected<void, StarException> endObject() override;
  std::expected<void, StarException> beginArray() override;
  std::expected<void, StarException> endArray() override;
  std::expected<void, StarException> putString(std::span<String::Char const> s) override;
  std::expected<void, StarException> putDouble(std::span<String::Char const> s) override;
  std::expected<void, StarException> putInteger(std::span<String::Char const> s) override;
  std::expected<void, StarException> putBoolean(bool b) override;
  std::expected<void, StarException> putNull() override;
  std::expected<void, StarException> putWhitespace(std::span<String::Char const> s) override;
  std::expected<void, StarException> putComma() override;
  std::expected<void, StarException> putColon() override;
  std::expected<FormattedJson, StarException> takeTop();
private:
  void push(FormattedJson const& v);
  std::expected<FormattedJson, StarException> pop();
  std::expected<std::reference_wrapper<FormattedJson>, StarException> current();
  std::expected<void, StarException> putValue(Json const& value, std::optional<String> formatting = {});
  std::optional<FormattedJson> m_root;
  List<FormattedJson> m_stack;
};

template <>
class JsonStreamer<FormattedJson> {
public:
  static std::expected<void, StarException> toJsonStream(FormattedJson const& val, JsonStream& stream, bool sort);
};

ValueElement::ValueElement(FormattedJson const& json) : value(std::make_shared<FormattedJson>(json)) {}

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
  : m_jsonValue(std::move(other.m_jsonValue)),
    m_elements(std::move(other.m_elements)),
    m_formatting(std::move(other.m_formatting)),
    m_lastKey((other.m_formatting.reset(), std::move(other.m_lastKey))),
    m_lastValue((other.m_lastKey.reset(), std::move(other.m_lastValue))),
    m_objectEntryLocations((other.m_lastValue.reset(), std::move(other.m_objectEntryLocations))),
    m_arrayElementLocations(std::move(other.m_arrayElementLocations)) {}

FormattedJson& FormattedJson::operator=(FormattedJson&& other) {
  if (this != &other) {
    m_jsonValue = std::move(other.m_jsonValue);
    m_elements = std::move(other.m_elements);
    m_formatting = std::move(other.m_formatting);
    other.m_formatting.reset();
    m_lastKey = std::move(other.m_lastKey);
    other.m_lastKey.reset();
    m_lastValue = std::move(other.m_lastValue);
    other.m_lastValue.reset();
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
  for (JsonElement const& elem : elements.slice(from, to)) {
    if (std::holds_alternative<WhitespaceElement>(elem))
      whitespace += std::get<WhitespaceElement>(elem).whitespace;
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
  return object(elements);
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
  return array(elements);
}

std::expected<FormattedJson, StarException> FormattedJson::append(FormattedJson const& value) const {
  if (type() != Json::Type::Array)
    return std::unexpected(JsonException::format("Cannot call append on FormattedJson type {}, must be Array type", typeName()));
  ElementList elements = m_elements;
  auto inserted = insertWithCommaAndFormatting(elements, elements.size(), true, {ValueElement{value}});
  if (!inserted)
    return std::unexpected(std::move(inserted.error()));
  return array(elements);
}

std::expected<FormattedJson, StarException> FormattedJson::set(std::size_t index, FormattedJson const& value) const {
  if (type() != Json::Type::Array)
    return std::unexpected(JsonException::format("Cannot call set with index on FormattedJson type {}, must be Array type", typeName()));
  if (index >= m_arrayElementLocations.size())
    return std::unexpected(JsonException::format("FormattedJson::set({}) out of range", index));
  ElementList elements = m_elements;
  elements[m_arrayElementLocations[index]] = ValueElement{value};
  return array(elements);
}

std::expected<FormattedJson, StarException> FormattedJson::eraseIndex(std::size_t index) const {
  if (type() != Json::Type::Array)
    return std::unexpected(JsonException::format("Cannot call set with index on FormattedJson type {}, must be Array type", typeName()));
  if (index >= m_arrayElementLocations.size())
    return std::unexpected(JsonException::format("FormattedJson::eraseIndex({}) out of range", index));
  ElementList elements = m_elements;
  removeValueFromArray(elements, m_arrayElementLocations[index]);
  return array(elements);
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
  String result;
  auto written = outputUtf32Json<JsonStringOutputIterator, FormattedJson>(*this, JsonStringOutputIterator(result), 0, false);
  if (!written)
    return std::unexpected(std::move(written.error()));
  return result;
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

std::expected<FormattedJson, StarException> FormattedJson::object(ElementList const& elements) {
  FormattedJson json = ofType(Json::Type::Object);
  for (JsonElement const& elem : elements) {
    auto appended = json.appendElement(elem);
    if (!appended)
      return std::unexpected(std::move(appended.error()));
  }
  return json;
}

std::expected<FormattedJson, StarException> FormattedJson::array(ElementList const& elements) {
  FormattedJson json = ofType(Json::Type::Array);
  for (JsonElement const& elem : elements) {
    if (std::holds_alternative<ColonElement>(elem) || std::holds_alternative<ObjectKeyElement>(elem))
      return std::unexpected(JsonException("Invalid FormattedJson element in Json array"));
    auto appended = json.appendElement(elem);
    if (!appended)
      return std::unexpected(std::move(appended.error()));
  }
  return json;
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
  return object(elements);
}

std::expected<void, StarException> FormattedJson::appendElement(JsonElement const& elem) {
  ElementLocation loc = m_elements.size();
  if (std::holds_alternative<ObjectKeyElement>(elem)) {
    if (!isType(Json::Type::Object))
      return std::unexpected(JsonException("Invalid FormattedJson object key outside object"));
    m_lastKey = loc;
  } else if (std::holds_alternative<ValueElement>(elem)) {
    auto const& value = std::get<ValueElement>(elem);
    if (!value.value)
      return std::unexpected(JsonException("Invalid empty FormattedJson value element"));
    if (m_lastKey) {
      if (!isType(Json::Type::Object) || *m_lastKey >= m_elements.size())
        return std::unexpected(JsonException("Invalid FormattedJson object entry"));
      if (!std::holds_alternative<ObjectKeyElement>(m_elements[*m_lastKey]))
        return std::unexpected(JsonException("Invalid FormattedJson object key element"));
      auto const& key = std::get<ObjectKeyElement>(m_elements[*m_lastKey]);
      auto updated = m_jsonValue.set(key.key, value.value->toJson());
      if (!updated)
        return std::unexpected(std::move(updated.error()));
      m_jsonValue = std::move(*updated);
      m_objectEntryLocations[key.key] = std::make_pair(*m_lastKey, loc);
      m_lastKey.reset();
    } else {
      if (!isType(Json::Type::Array))
        return std::unexpected(JsonException("Invalid FormattedJson array entry"));
      auto updated = m_jsonValue.append(value.value->toJson());
      if (!updated)
        return std::unexpected(std::move(updated.error()));
      m_jsonValue = std::move(*updated);
      m_arrayElementLocations.append(loc);
    }
    m_lastValue = loc;
  }
  m_elements.append(elem);
  return {};
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

std::expected<FormattedJson, StarException> FormattedJson::formattedAs(String const& formatting) const {
  if constexpr (Star::DebugEnabled) {
    auto parsed = Json::parse(formatting);
    if (!parsed)
      return std::unexpected(std::move(parsed.error()));
    Star::starAssertFailed((*parsed == toJson()));
  }
  FormattedJson json = *this;
  json.m_formatting = formatting;
  return json;
}

std::expected<void, StarException> FormattedJsonBuilderStream::beginObject() {
  push(FormattedJson::ofType(Json::Type::Object));
  return {};
}

std::expected<void, StarException> FormattedJsonBuilderStream::objectKey(std::span<String::Char const> s) {
  auto key = String::create(std::u32string_view(s.data(), s.size()));
  if (!key)
    return std::unexpected(std::move(key.error()));
  auto container = current();
  if (!container)
    return std::unexpected(std::move(container.error()));
  return container->get().appendElement(ObjectKeyElement{std::move(*key)});
}

std::expected<void, StarException> FormattedJsonBuilderStream::endObject() {
  auto value = pop();
  if (!value)
    return std::unexpected(std::move(value.error()));
  if (!m_stack.empty())
    return m_stack.back().appendElement(ValueElement{*value});
  m_root = std::move(*value);
  return {};
}

std::expected<void, StarException> FormattedJsonBuilderStream::beginArray() {
  push(FormattedJson::ofType(Json::Type::Array));
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

std::expected<void, StarException> FormattedJsonBuilderStream::putDouble(std::span<String::Char const> s) {
  auto text = String::create(std::u32string_view(s.data(), s.size()));
  if (!text)
    return std::unexpected(std::move(text.error()));
  auto number = lexicalCast<double>(*text);
  if (!number)
    return std::unexpected(JsonParserException("bad double", ExceptionDiagnostic(number.error())));
  return putValue(Json(*number), std::move(*text));
}

std::expected<void, StarException> FormattedJsonBuilderStream::putInteger(std::span<String::Char const> s) {
  auto text = String::create(std::u32string_view(s.data(), s.size()));
  if (!text)
    return std::unexpected(std::move(text.error()));
  auto number = lexicalCast<long long>(*text);
  if (!number)
    return std::unexpected(JsonParserException("bad integer", ExceptionDiagnostic(number.error())));
  return putValue(Json(*number), std::move(*text));
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
  return m_stack.back().appendElement(WhitespaceElement{std::move(*whitespace)});
}

std::expected<void, StarException> FormattedJsonBuilderStream::putColon() {
  auto container = current();
  if (!container)
    return std::unexpected(std::move(container.error()));
  return container->get().appendElement(ColonElement{});
}

std::expected<void, StarException> FormattedJsonBuilderStream::putComma() {
  auto container = current();
  if (!container)
    return std::unexpected(std::move(container.error()));
  return container->get().appendElement(CommaElement{});
}

std::expected<FormattedJson, StarException> FormattedJsonBuilderStream::takeTop() {
  if (!m_root || !m_stack.empty())
    return std::unexpected(JsonParsingException("Incomplete formatted JSON builder stream"));
  FormattedJson result = std::move(*m_root);
  m_root.reset();
  return result;
}

void FormattedJsonBuilderStream::push(FormattedJson const& value) {
  m_stack.push_back(value);
}

std::expected<FormattedJson, StarException> FormattedJsonBuilderStream::pop() {
  if (m_stack.empty())
    return std::unexpected(JsonParsingException("Missing formatted JSON builder container"));
  FormattedJson result = std::move(m_stack.back());
  m_stack.pop_back();
  return result;
}

std::expected<std::reference_wrapper<FormattedJson>, StarException> FormattedJsonBuilderStream::current() {
  if (m_stack.empty())
    return std::unexpected(JsonParsingException("Missing formatted JSON builder container"));
  return std::ref(m_stack.back());
}

std::expected<void, StarException> FormattedJsonBuilderStream::putValue(Json const& value, std::optional<String> formatting) {
  auto formatted = FormattedJson::create(value);
  if (!formatted)
    return std::unexpected(std::move(formatted.error()));
  if (formatting) {
    auto updated = formatted->formattedAs(*formatting);
    if (!updated)
      return std::unexpected(std::move(updated.error()));
    *formatted = std::move(*updated);
  }
  if (!m_stack.empty())
    return m_stack.back().appendElement(ValueElement{*formatted});
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
      auto text = formatted->wideString();
      if (!text)
        return std::unexpected(std::move(text.error()));
      return val.isType(Json::Type::Float) ? stream.putDouble(*text) : stream.putInteger(*text);
    }
    return JsonStreamer<Json>::toJsonStream(val.toJson(), stream, sort);
  }
  auto begun = object ? stream.beginObject() : stream.beginArray();
  if (!begun)
    return std::unexpected(std::move(begun.error()));
  for (auto const& elem : val.elements()) {
    std::expected<void, StarException> written;
    if (std::holds_alternative<ObjectKeyElement>(elem)) {
      auto text = std::get<ObjectKeyElement>(elem).key.wideString();
      if (!text)
        return std::unexpected(std::move(text.error()));
      written = stream.objectKey(*text);
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
