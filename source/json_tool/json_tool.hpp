#pragma once

import std;
import star.json;
import star.string;
import star.list;
import star.variant;
import star.lexical_cast;

import star.formatted_json;
import star.json_path;

namespace Star {

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

typedef MVariant<GetCommand, SetCommand, AddCommand, RemoveCommand, EditCommand> Command;

struct AtBeginning {};

struct AtEnd {};

struct BeforeKey {
  String key;
};

struct AfterKey {
  String key;
};

typedef MVariant<AtBeginning, AtEnd, BeforeKey, AfterKey> InsertLocation;

class JsonInputFormat {
public:
  virtual ~JsonInputFormat() {}
  virtual FormattedJson toJson(String const& input) const = 0;
  virtual String fromJson(FormattedJson const& json) const = 0;
  virtual FormattedJson getDefault() const = 0;
};

class GenericInputFormat : public JsonInputFormat {
public:
  virtual FormattedJson toJson(String const& input) const override;
  virtual String fromJson(FormattedJson const& json) const override;
  virtual FormattedJson getDefault() const override;
};

class CommaSeparatedStrings : public JsonInputFormat {
public:
  virtual FormattedJson toJson(String const& input) const override;
  virtual String fromJson(FormattedJson const& json) const override;
  virtual FormattedJson getDefault() const override;
};

class StringInputFormat : public JsonInputFormat {
public:
  virtual FormattedJson toJson(String const& input) const override;
  virtual String fromJson(FormattedJson const& json) const override;
  virtual FormattedJson getDefault() const override;
};

class Output {
public:
  virtual ~Output() {}
  virtual void out(FormattedJson const& json) = 0;
  virtual void flush() = 0;

  std::function<void(FormattedJson const& json)> toFunction() {
    return [this](FormattedJson const& json) { this->out(json); };
  }
};

class OutputOnSeparateLines : public Output {
public:
  virtual void out(FormattedJson const& json) override;
  virtual void flush() override;
};

class ArrayOutput : public Output {
public:
  ArrayOutput(bool unique) : m_unique(unique), m_results() {}

  virtual void out(FormattedJson const& json) override;
  virtual void flush() override;

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

typedef MVariant<JsonLiteralInput, FileInput, FindInput> Input;

struct ParsedArgs {
  ParsedArgs() : inputs(), command(), options() {}

  List<Input> inputs;
  Command command;
  Options options;
};

FormattedJson addOrSet(bool add, std::shared_ptr<JsonPath::Path> path, FormattedJson const& input, InsertLocation insertLocation, FormattedJson const& value);
String reprWithLineEnding(FormattedJson const& json);
StringList findFiles(FindInput const& findArgs);

}
