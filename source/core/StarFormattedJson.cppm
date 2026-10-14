module;
import star.string;
import star.list;
import star.map;
import star.variant;
import star.maybe;
import star.stream_formatter;

import std;

import star.json;

namespace Star {

class FormattedJson;
using FormattedJsonPtr = std::shared_ptr<FormattedJson>;
using FormattedJsonConstPtr = std::shared_ptr<const FormattedJson>;
using FormattedJsonWeakPtr = std::weak_ptr<FormattedJson>;
using FormattedJsonConstWeakPtr = std::weak_ptr<const FormattedJson>;
using FormattedJsonUPtr = std::unique_ptr<FormattedJson>;
using FormattedJsonConstUPtr = std::unique_ptr<const FormattedJson>;

struct ObjectElement;
struct ObjectKeyElement;
struct ValueElement;
struct WhitespaceElement;
struct ColonElement;
struct CommaElement;

typedef Variant<ValueElement, ObjectKeyElement, WhitespaceElement, ColonElement, CommaElement> JsonElement;

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
  typedef List<JsonElement> ElementList;
  typedef std::size_t ElementLocation;

  static FormattedJson parse(String const& string);
  static FormattedJson parseJson(String const& string);

  static FormattedJson ofType(Json::Type type);

  FormattedJson();
  FormattedJson(Json const&);

  Json const& toJson() const;

  FormattedJson get(String const& key) const;
  FormattedJson get(std::size_t index) const;

  // Returns a new FormattedJson with the given values added or erased.
  // Prepend, insert and append update the value in-place if the key already
  // exists.
  FormattedJson prepend(String const& key, FormattedJson const& value) const;
  FormattedJson insertBefore(String const& key, FormattedJson const& value, String const& beforeKey) const;
  FormattedJson insertAfter(String const& key, FormattedJson const& value, String const& afterKey) const;
  FormattedJson append(String const& key, FormattedJson const& value) const;
  FormattedJson set(String const& key, FormattedJson const& value) const;
  FormattedJson eraseKey(String const& key) const;

  FormattedJson insert(std::size_t index, FormattedJson const& value) const;
  FormattedJson append(FormattedJson const& value) const;
  FormattedJson set(std::size_t index, FormattedJson const& value) const;
  FormattedJson eraseIndex(std::size_t index) const;

  // Returns the number of elements in a Json array, or entries in an object.
  std::size_t size() const;

  bool contains(String const& key) const;

  Json::Type type() const;
  bool isType(Json::Type type) const;
  String typeName() const;

  String toFormattedDouble() const;
  String toFormattedInt() const;

  String repr() const;
  String printJson() const;

  ElementList const& elements() const;

  // Equality ignores whitespace and formatting. It just compares the Json
  // values.
  bool operator==(FormattedJson const& v) const;
  bool operator!=(FormattedJson const& v) const;

private:
  friend class FormattedJsonBuilderStream;

  static FormattedJson object(ElementList const& elements);
  static FormattedJson array(ElementList const& elements);

  FormattedJson objectInsert(String const& key, FormattedJson const& value, ElementLocation loc) const;
  void appendElement(JsonElement const& elem);

  FormattedJson const& getFormattedJson(ElementLocation loc) const;
  FormattedJson formattedAs(String const& formatting) const;

  Json m_jsonValue;
  ElementList m_elements;
  // Used to preserve the formatting of numbers, i.e. -0 vs 0, 1.0 vs 1:
  Maybe<String> m_formatting;

  Maybe<ElementLocation> m_lastKey, m_lastValue;
  Map<String, std::pair<ElementLocation, ElementLocation>> m_objectEntryLocations;
  List<ElementLocation> m_arrayElementLocations;
};

std::ostream& operator<<(std::ostream& os, FormattedJson const& json);

}

export module star.formatted_json;

export template <> struct std::formatter<Star::FormattedJson> : Star::StreamFormatter {};

export namespace Star {
  using ::Star::FormattedJson;
  using ::Star::FormattedJsonPtr;
  using ::Star::FormattedJsonConstPtr;
  using ::Star::FormattedJsonWeakPtr;
  using ::Star::FormattedJsonConstWeakPtr;
  using ::Star::FormattedJsonUPtr;
  using ::Star::FormattedJsonConstUPtr;
  using ::Star::JsonElement;
  using ::Star::ObjectElement;
  using ::Star::ObjectKeyElement;
  using ::Star::ValueElement;
  using ::Star::WhitespaceElement;
  using ::Star::ColonElement;
  using ::Star::CommaElement;
  using ::Star::operator<<;
}
