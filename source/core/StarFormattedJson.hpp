#pragma once

#include <list>

#include "StarJson.hpp"

namespace Star {

class FormattedJson;
using FormattedJsonPtr = SharedPtr<FormattedJson>;

struct ObjectElement;
struct ObjectKeyElement;
struct ValueElement;
struct WhitespaceElement;
struct ColonElement;
struct CommaElement;

using JsonElement = Variant<ValueElement, ObjectKeyElement, WhitespaceElement, ColonElement, CommaElement>;

struct ValueElement {
  ValueElement(FormattedJson const& json);

  FormattedJsonPtr value;

  bool operator==(ValueElement const& v) const;
};

struct ObjectKeyElement {
  String key;

  bool operator==(ObjectKeyElement const& v) const;
};

struct WhitespaceElement {
  String whitespace;

  bool operator==(WhitespaceElement const& v) const;
};

struct ColonElement {
  bool operator==(ColonElement const&) const;
};

struct CommaElement {
  bool operator==(CommaElement const&) const;
};

std::ostream& operator<<(std::ostream& os, JsonElement const& elem);

// Class representing formatted JSON data. Preserves whitespace and comments.
class FormattedJson {
public:
  using ElementList = List<JsonElement>;
  using ElementLocation = size_t;

  [[nodiscard]] static FormattedJson parse(String const& string);
  [[nodiscard]] static FormattedJson parseJson(String const& string);

  [[nodiscard]] static FormattedJson ofType(Json::Type type);

  FormattedJson();
  FormattedJson(Json const&);

  [[nodiscard]] Json const& toJson() const;

  [[nodiscard]] FormattedJson get(String const& key) const;
  [[nodiscard]] FormattedJson get(size_t index) const;

  // Returns a new FormattedJson with the given values added or erased.
  // Prepend, insert and append update the value in-place if the key already
  // exists.
  [[nodiscard]] FormattedJson prepend(String const& key, FormattedJson const& value) const;
  [[nodiscard]] FormattedJson insertBefore(String const& key, FormattedJson const& value, String const& beforeKey) const;
  [[nodiscard]] FormattedJson insertAfter(String const& key, FormattedJson const& value, String const& afterKey) const;
  [[nodiscard]] FormattedJson append(String const& key, FormattedJson const& value) const;
  [[nodiscard]] FormattedJson set(String const& key, FormattedJson const& value) const;
  [[nodiscard]] FormattedJson eraseKey(String const& key) const;

  [[nodiscard]] FormattedJson insert(size_t index, FormattedJson const& value) const;
  [[nodiscard]] FormattedJson append(FormattedJson const& value) const;
  [[nodiscard]] FormattedJson set(size_t index, FormattedJson const& value) const;
  [[nodiscard]] FormattedJson eraseIndex(size_t index) const;

  // Returns the number of elements in a Json array, or entries in an object.
  [[nodiscard]] size_t size() const;

  [[nodiscard]] bool contains(String const& key) const;

  [[nodiscard]] Json::Type type() const;
  [[nodiscard]] bool isType(Json::Type type) const;
  [[nodiscard]] String typeName() const;

  [[nodiscard]] String toFormattedDouble() const;
  [[nodiscard]] String toFormattedInt() const;

  [[nodiscard]] String repr() const;
  [[nodiscard]] String printJson() const;

  [[nodiscard]] ElementList const& elements() const;

  // Equality ignores whitespace and formatting. It just compares the Json
  // values.
  [[nodiscard]] bool operator==(FormattedJson const& v) const;
  [[nodiscard]] bool operator!=(FormattedJson const& v) const;

private:
  friend class FormattedJsonBuilderStream;

  [[nodiscard]] static FormattedJson object(ElementList const& elements);
  [[nodiscard]] static FormattedJson array(ElementList const& elements);

  [[nodiscard]] FormattedJson objectInsert(String const& key, FormattedJson const& value, ElementLocation loc) const;
  void appendElement(JsonElement const& elem);

  [[nodiscard]] FormattedJson const& getFormattedJson(ElementLocation loc) const;
  [[nodiscard]] FormattedJson formattedAs(String const& formatting) const;

  Json m_jsonValue;
  ElementList m_elements;
  // Used to preserve the formatting of numbers, i.e. -0 vs 0, 1.0 vs 1:
  Maybe<String> m_formatting;

  Maybe<ElementLocation> m_lastKey, m_lastValue;
  struct ObjectEntryLocation {
    ElementLocation key;
    ElementLocation value;
  };
  Map<String, ObjectEntryLocation> m_objectEntryLocations;
  List<ElementLocation> m_arrayElementLocations;
};

std::ostream& operator<<(std::ostream& os, FormattedJson const& json);

}

template <> struct std::formatter<Star::FormattedJson> : Star::OstreamFormatter {};
