module;
import std;
import star.json;
import star.string;
import star.list;
import star.lexical_cast;
import star.format;

import star.formatted_json;
import star.json_path;

namespace Star {

class Logger;

struct GetCommand {
  std::shared_ptr<JsonPath::Path> path;
  bool opt;
  bool children;
};

struct SetCommand {
  std::shared_ptr<JsonPath::Path> path;
  FormattedJson value;
};

struct AddCommand {
  std::shared_ptr<JsonPath::Path> path;
  FormattedJson value;
};

struct RemoveCommand {
  std::shared_ptr<JsonPath::Path> path;
};

struct EditCommand {
  std::shared_ptr<JsonPath::Path> path;
};

using Command = std::variant<std::monostate, GetCommand, SetCommand, AddCommand, RemoveCommand, EditCommand>;

struct AtBeginning {};

struct AtEnd {};

struct BeforeKey {
  String key;
};

struct AfterKey {
  String key;
};

using InsertLocation = std::variant<std::monostate, AtBeginning, AtEnd, BeforeKey, AfterKey>;

class JsonInputFormat {
public:
  virtual ~JsonInputFormat() {}
  [[nodiscard]] virtual std::expected<FormattedJson, StarException> toJson(String const& input) const = 0;
  [[nodiscard]] virtual std::expected<String, StarException> fromJson(FormattedJson const& json) const = 0;
  [[nodiscard]] virtual std::expected<FormattedJson, StarException> getDefault() const = 0;
};

class GenericInputFormat : public JsonInputFormat {
public:
  [[nodiscard]] std::expected<FormattedJson, StarException> toJson(String const& input) const override;
  [[nodiscard]] std::expected<String, StarException> fromJson(FormattedJson const& json) const override;
  [[nodiscard]] std::expected<FormattedJson, StarException> getDefault() const override;
};

class CommaSeparatedStrings : public JsonInputFormat {
public:
  [[nodiscard]] std::expected<FormattedJson, StarException> toJson(String const& input) const override;
  [[nodiscard]] std::expected<String, StarException> fromJson(FormattedJson const& json) const override;
  [[nodiscard]] std::expected<FormattedJson, StarException> getDefault() const override;
};

class StringInputFormat : public JsonInputFormat {
public:
  [[nodiscard]] std::expected<FormattedJson, StarException> toJson(String const& input) const override;
  [[nodiscard]] std::expected<String, StarException> fromJson(FormattedJson const& json) const override;
  [[nodiscard]] std::expected<FormattedJson, StarException> getDefault() const override;
};

class Output {
public:
  virtual ~Output() {}
  [[nodiscard]] virtual std::expected<void, StarException> out(FormattedJson const& json) = 0;
  [[nodiscard]] virtual std::expected<void, StarException> flush() = 0;

  std::function<std::expected<void, StarException>(FormattedJson const&)> toFunction() {
    return [&output = *this](FormattedJson const& json) { return output.out(json); };
  }
};

class OutputOnSeparateLines : public Output {
public:
  [[nodiscard]] std::expected<void, StarException> out(FormattedJson const& json) override;
  [[nodiscard]] std::expected<void, StarException> flush() override;
};

class ArrayOutput : public Output {
public:
  ArrayOutput(bool unique) : m_unique(unique), m_results() {}

  [[nodiscard]] std::expected<void, StarException> out(FormattedJson const& json) override;
  [[nodiscard]] std::expected<void, StarException> flush() override;

private:
  bool m_unique;
  List<FormattedJson> m_results;
};

struct Options {
  Options() : inPlace(false), insertLocation(), editFormat(nullptr), editorImages(), output() {}

  bool inPlace;
  InsertLocation insertLocation;
  std::shared_ptr<JsonInputFormat> editFormat;
  List<std::shared_ptr<JsonPath::Path>> editorImages;
  std::shared_ptr<Output> output;
};

struct JsonLiteralInput {
  String json;
};

struct FileInput {
  String filename;
};

struct FindInput {
  String directory;
  String filenameSuffix;
};

using Input = std::variant<std::monostate, JsonLiteralInput, FileInput, FindInput>;

struct ParsedArgs {
  ParsedArgs() : inputs(), command(), options() {}

  List<Input> inputs;
  Command command;
  Options options;
};

[[nodiscard]] std::expected<FormattedJson, StarException> addOrSet(bool add, std::shared_ptr<JsonPath::Path> path, FormattedJson const& input, InsertLocation insertLocation, FormattedJson const& value);
[[nodiscard]] std::expected<String, StarException> reprWithLineEnding(FormattedJson const& json);
[[nodiscard]] std::expected<StringList, StarException> findFiles(Logger& logger, FindInput const& findArgs);

}


export module star.json_tool;

export namespace Star {
  using ::Star::GetCommand;
  using ::Star::SetCommand;
  using ::Star::AddCommand;
  using ::Star::RemoveCommand;
  using ::Star::EditCommand;
  using ::Star::Command;
  using ::Star::AtBeginning;
  using ::Star::AtEnd;
  using ::Star::BeforeKey;
  using ::Star::AfterKey;
  using ::Star::InsertLocation;
  using ::Star::JsonInputFormat;
  using ::Star::GenericInputFormat;
  using ::Star::CommaSeparatedStrings;
  using ::Star::StringInputFormat;
  using ::Star::Output;
  using ::Star::OutputOnSeparateLines;
  using ::Star::ArrayOutput;
  using ::Star::Options;
  using ::Star::JsonLiteralInput;
  using ::Star::FileInput;
  using ::Star::FindInput;
  using ::Star::Input;
  using ::Star::ParsedArgs;
  using ::Star::addOrSet;
  using ::Star::reprWithLineEnding;
  using ::Star::findFiles;
}
