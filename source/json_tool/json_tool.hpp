#pragma once

#include "StarFormattedJson.hpp"
#include "StarJsonPath.hpp"

namespace Star {

struct GetCommand {
  JsonPath::PathPtr path;
  bool opt;
  bool children;
};

struct SetCommand {
  JsonPath::PathPtr path;
  FormattedJson value;
};

struct AddCommand {
  JsonPath::PathPtr path;
  FormattedJson value;
};

struct RemoveCommand {
  JsonPath::PathPtr path;
};

struct EditCommand {
  JsonPath::PathPtr path;
};

using Command = MVariant<GetCommand, SetCommand, AddCommand, RemoveCommand, EditCommand>;

struct AtBeginning {};

struct AtEnd {};

struct BeforeKey {
  String key;
};

struct AfterKey {
  String key;
};

using InsertLocation = MVariant<AtBeginning, AtEnd, BeforeKey, AfterKey>;

class JsonInputFormat;
using JsonInputFormatPtr = SharedPtr<JsonInputFormat>;
using JsonInputFormatConstPtr = SharedPtr<JsonInputFormat const>;
using JsonInputFormatWeakPtr = WeakPtr<JsonInputFormat>;
using JsonInputFormatConstWeakPtr = WeakPtr<JsonInputFormat const>;

class JsonInputFormat {
public:
  virtual ~JsonInputFormat() = default;
  virtual FormattedJson toJson(String const& input) const = 0;
  virtual String fromJson(FormattedJson const& json) const = 0;
  virtual FormattedJson getDefault() const = 0;
};

class GenericInputFormat : public JsonInputFormat {
public:
  FormattedJson toJson(String const& input) const override;
  String fromJson(FormattedJson const& json) const override;
  FormattedJson getDefault() const override;
};

class CommaSeparatedStrings : public JsonInputFormat {
public:
  FormattedJson toJson(String const& input) const override;
  String fromJson(FormattedJson const& json) const override;
  FormattedJson getDefault() const override;
};

class StringInputFormat : public JsonInputFormat {
public:
  FormattedJson toJson(String const& input) const override;
  String fromJson(FormattedJson const& json) const override;
  FormattedJson getDefault() const override;
};

class Output;
using OutputPtr = SharedPtr<Output>;
using OutputConstPtr = SharedPtr<Output const>;
using OutputWeakPtr = WeakPtr<Output>;
using OutputConstWeakPtr = WeakPtr<Output const>;

class Output {
public:
  virtual ~Output() = default;
  virtual void out(FormattedJson const& json) = 0;
  virtual void flush() = 0;

  function<void(FormattedJson const& json)> toFunction() {
    return [this](FormattedJson const& json) { this->out(json); };
  }
};

class OutputOnSeparateLines : public Output {
public:
  void out(FormattedJson const& json) override;
  void flush() override;
};

class ArrayOutput : public Output {
public:
  ArrayOutput(bool unique) : m_unique(unique) {}

  void out(FormattedJson const& json) override;
  void flush() override;

private:
  bool m_unique;
  List<FormattedJson> m_results;
};

struct Options {
  bool inPlace = false;
  InsertLocation insertLocation;
  JsonInputFormatPtr editFormat = nullptr;
  List<JsonPath::PathPtr> editorImages;
  OutputPtr output;
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

using Input = MVariant<JsonLiteralInput, FileInput, FindInput>;

struct ParsedArgs {
  List<Input> inputs;
  Command command;
  Options options;
};

FormattedJson addOrSet(bool add, JsonPath::PathPtr path, FormattedJson const& input, InsertLocation insertLocation, FormattedJson const& value);
String reprWithLineEnding(FormattedJson const& json);
StringList findFiles(FindInput const& findArgs);

}
