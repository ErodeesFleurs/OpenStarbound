#pragma once

#include "StarConfig.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

class FormattedJson;

struct ObjectElement;
struct ObjectKeyElement;
struct ValueElement;
struct WhitespaceElement;
struct ColonElement;
struct CommaElement;

using JsonElement = Variant<ValueElement, ObjectKeyElement, WhitespaceElement, ColonElement, CommaElement>;

struct ValueElement {
  ValueElement(FormattedJson const& json);

  Ptr<FormattedJson> value;

  auto operator==(ValueElement const& v) const -> bool;
};

struct ObjectKeyElement {
  String key;

  auto operator==(ObjectKeyElement const& v) const -> bool;
};

struct WhitespaceElement {
  String whitespace;

  auto operator==(WhitespaceElement const& v) const -> bool;
};

struct ColonElement {
  auto operator==(ColonElement const&) const -> bool {
    return true;
  }
};

struct CommaElement {
  auto operator==(CommaElement const&) const -> bool;
};

auto operator<<(std::ostream& os, JsonElement const& elem) -> std::ostream&;

// Class representing formatted JSON data. Preserves whitespace and comments.
class FormattedJson {
public:
  using ElementList = List<JsonElement>;
  using ElementLocation = size_t;

  static auto parse(String const& string) -> FormattedJson;
  static auto parseJson(String const& string) -> FormattedJson;

  static auto ofType(Json::Type type) -> FormattedJson;

  FormattedJson();
  FormattedJson(Json const&);

  [[nodiscard]] auto toJson() const -> Json const&;

  [[nodiscard]] auto get(String const& key) const -> FormattedJson;
  [[nodiscard]] auto get(size_t index) const -> FormattedJson;

  // Returns a new FormattedJson with the given values added or erased.
  // Prepend, insert and append update the value in-place if the key already
  // exists.
  [[nodiscard]] auto prepend(String const& key, FormattedJson const& value) const -> FormattedJson;
  [[nodiscard]] auto insertBefore(String const& key, FormattedJson const& value, String const& beforeKey) const -> FormattedJson;
  [[nodiscard]] auto insertAfter(String const& key, FormattedJson const& value, String const& afterKey) const -> FormattedJson;
  [[nodiscard]] auto append(String const& key, FormattedJson const& value) const -> FormattedJson;
  [[nodiscard]] auto set(String const& key, FormattedJson const& value) const -> FormattedJson;
  [[nodiscard]] auto eraseKey(String const& key) const -> FormattedJson;

  [[nodiscard]] auto insert(size_t index, FormattedJson const& value) const -> FormattedJson;
  [[nodiscard]] auto append(FormattedJson const& value) const -> FormattedJson;
  [[nodiscard]] auto set(size_t index, FormattedJson const& value) const -> FormattedJson;
  [[nodiscard]] auto eraseIndex(size_t index) const -> FormattedJson;

  // Returns the number of elements in a Json array, or entries in an object.
  [[nodiscard]] auto size() const -> std::size_t;

  [[nodiscard]] auto contains(String const& key) const -> bool;

  [[nodiscard]] auto type() const -> Json::Type;
  [[nodiscard]] auto isType(Json::Type type) const -> bool;
  [[nodiscard]] auto typeName() const -> String;

  [[nodiscard]] auto toFormattedDouble() const -> String;
  [[nodiscard]] auto toFormattedInt() const -> String;

  [[nodiscard]] auto repr() const -> String;
  [[nodiscard]] auto printJson() const -> String;

  [[nodiscard]] auto elements() const -> ElementList const&;

  // Equality ignores whitespace and formatting. It just compares the Json
  // values.
  auto operator==(FormattedJson const& v) const -> bool;
  auto operator!=(FormattedJson const& v) const -> bool;

private:
  friend class FormattedJsonBuilderStream;

  static auto object(ElementList const& elements) -> FormattedJson;
  static auto array(ElementList const& elements) -> FormattedJson;

  [[nodiscard]] auto objectInsert(String const& key, FormattedJson const& value, ElementLocation loc) const -> FormattedJson;
  void appendElement(JsonElement const& elem);

  [[nodiscard]] auto getFormattedJson(ElementLocation loc) const -> FormattedJson const&;
  [[nodiscard]] auto formattedAs(String const& formatting) const -> FormattedJson;

  Json m_jsonValue;
  ElementList m_elements;
  // Used to preserve the formatting of numbers, i.e. -0 vs 0, 1.0 vs 1:
  std::optional<String> m_formatting;

  std::optional<ElementLocation> m_lastKey, m_lastValue;
  Map<String, std::pair<ElementLocation, ElementLocation>> m_objectEntryLocations;
  List<ElementLocation> m_arrayElementLocations;
};

auto operator<<(std::ostream& os, FormattedJson const& json) -> std::ostream&;

}

template <> struct std::formatter<Star::FormattedJson> : Star::ostream_formatter {};
