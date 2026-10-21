#include <stdio.h>

import std;
import star.string;
import star.list;
import star.format;
import star.json;
import star.file;
import star.logging;
import star.json_tool;
import star.json_editor;
import star.lexical_cast;
import star.formatted_json;
import star.json_path;
import star.native_runtime;

// Tool for scripting and mass-editing of JSON+Comments files without affecting
// formatting.

import star.algorithm;
using namespace Star;

FormattedJson GenericInputFormat::toJson(String const& input) const {
  return FormattedJson::parse(input);
}

String GenericInputFormat::fromJson(FormattedJson const& json) const {
  return json.repr();
}

FormattedJson GenericInputFormat::getDefault() const {
  return FormattedJson::ofType(Json::Type::Null);
}

FormattedJson CommaSeparatedStrings::toJson(String const& input) const {
  if (input.trim() == "")
    return FormattedJson::ofType(Json::Type::Array);
  StringList strings = input.split(',');
  JsonArray array = strings.transformed(std::bind(&String::trim, std::placeholders::_1, "")).transformed(construct<Json>());
  return Json(array);
}

String CommaSeparatedStrings::fromJson(FormattedJson const& json) const {
  StringList strings = json.toJson().toArray().transformed(std::bind(&Json::toString, std::placeholders::_1));
  return strings.join(", ");
}

FormattedJson CommaSeparatedStrings::getDefault() const {
  return FormattedJson::ofType(Json::Type::Array);
}

FormattedJson StringInputFormat::toJson(String const& input) const {
  return FormattedJson(Json(input));
}

String StringInputFormat::fromJson(FormattedJson const& json) const {
  return json.toJson().toString();
}

FormattedJson StringInputFormat::getDefault() const {
  return FormattedJson(Json(""));
}

String Star::reprWithLineEnding(FormattedJson const& json) {
  // Append a newline, preserving the newline style of the file, e.g windows or
  // unix.
  String repr = json.repr();
  if (repr.contains("\r"))
    return strf("{}\r\n", repr);
  return strf("{}\n", repr);
}

void OutputOnSeparateLines::out(FormattedJson const& json) {
  coutf("{}", reprWithLineEnding(json));
}

void OutputOnSeparateLines::flush() {}

void ArrayOutput::out(FormattedJson const& json) {
  if (!m_unique || !m_results.contains(json))
    m_results.append(json);
}

void ArrayOutput::flush() {
  FormattedJson array = FormattedJson::ofType(Json::Type::Array);
  for (FormattedJson const& result : m_results) {
    array = array.append(result);
  }
  coutf("{}", reprWithLineEnding(array));
}

FormattedJson Star::addOrSet(bool add,
    std::shared_ptr<::Star::JsonPath::Path> path,
    FormattedJson const& input,
    InsertLocation insertLocation,
    FormattedJson const& value) {
  JsonPath::EmptyPathOp<FormattedJson> emptyPathOp = [&value, add](FormattedJson const& document) {
    if (!add || document.type() == Json::Type::Null)
      return value;
    throw JsonException("Cannot add a value to the entire document, it is not empty.");
  };
  JsonPath::ObjectOp<FormattedJson> objectOp = [&value, &insertLocation](
      FormattedJson const& object, String const& key) {
    if (std::holds_alternative<AtBeginning>(insertLocation))
      return object.prepend(key, value);
    if (std::holds_alternative<AtEnd>(insertLocation))
      return object.append(key, value);
    if (std::holds_alternative<BeforeKey>(insertLocation))
      return object.insertBefore(key, value, std::get<BeforeKey>(insertLocation).key);
    if (std::holds_alternative<AfterKey>(insertLocation))
      return object.insertAfter(key, value, std::get<AfterKey>(insertLocation).key);
    return object.set(key, value);
  };
  JsonPath::ArrayOp<FormattedJson> arrayOp = [&value, add](FormattedJson const& array, std::optional<std::size_t> i) {
    if (i.has_value()) {
      if (add)
        return array.insert(*i, value);
      return array.set(*i, value);
    }
    return array.append(value);
  };
  return path->apply(input, emptyPathOp, objectOp, arrayOp);
}

void forEachFileRecursive(Logger& logger, String const& directory, std::function<void(String)> func) {
  for (std::pair<String, bool> entry : File::dirList(directory)) {
    String filename = File::relativeTo(directory, entry.first);
    // Never follow a symlinked directory: it can point outside of the tree, or
    // at one of its own parents (which would recurse until the OS errors out).
    if (entry.second && File::isSymlink(filename)) {
      logger.warn("json_tool: Not descending into '{}' because it is a symlink", filename);
      continue;
    }

    if (entry.second)
      forEachFileRecursive(logger, filename, func);
    else
      func(filename);
  }
}

StringList Star::findFiles(Logger& logger, FindInput const& findArgs) {
  StringList matches;
  forEachFileRecursive(logger, findArgs.directory,
      [&findArgs, &matches](String const& filename) {
        if (filename.endsWith(findArgs.filenameSuffix))
          matches.append(filename);
      });
  return matches;
}

void forEachChild(FormattedJson const& parent, std::function<void(FormattedJson const&)> func) {
  if (parent.isType(Json::Type::Object)) {
    for (String const& key : parent.toJson().toObject().keys()) {
      func(parent.get(key));
    }
  } else if (parent.isType(Json::Type::Array)) {
    for (std::size_t i = 0; i < parent.size(); ++i) {
      func(parent.get(i));
    }
  } else {
    throw JsonPath::TraversalException::format(
        "Cannot get the children of Json type {}, must be either Array or Object", parent.typeName());
  }
}

bool process(std::function<void(FormattedJson const&)> output,
    Command const& command,
    Options const& options,
    FormattedJson const& input) {
  if (std::holds_alternative<GetCommand>(command)) {
    GetCommand const& getCmd = std::get<GetCommand>(command);
    try {
      FormattedJson value = getCmd.path->get(input);
      if (getCmd.children) {
        forEachChild(value, output);
      } else {
        output(value);
      }
    } catch (JsonPath::TraversalException const& e) {
      if (!getCmd.opt)
        throw e;
    }

  } else if (std::holds_alternative<SetCommand>(command)) {
    SetCommand const& setCmd = std::get<SetCommand>(command);
    output(addOrSet(false, setCmd.path, input, options.insertLocation, setCmd.value));

  } else if (std::holds_alternative<AddCommand>(command)) {
    AddCommand const& addCmd = std::get<AddCommand>(command);
    output(addOrSet(true, addCmd.path, input, options.insertLocation, addCmd.value));

  } else if (std::holds_alternative<RemoveCommand>(command)) {
    output(std::get<RemoveCommand>(command).path->remove(input));

  } else {
    if constexpr (Star::DebugEnabled) {
      Star::starAssertFailed((std::holds_alternative<std::monostate>(command)));
    }
    output(input);
  }
  return true;
}

bool process(
    std::function<void(FormattedJson const&)> output, Command const& command, Options const& options, String const& input) {
  FormattedJson inJson = FormattedJson::parse(input);
  return process(output, command, options, inJson);
}

bool process(Logger& logger,
    std::function<void(FormattedJson const&)> output, Command const& command, Options const& options, Input const& input) {
  if (std::holds_alternative<JsonLiteralInput>(input)) {
    return process(output, command, options, std::get<JsonLiteralInput>(input).json);
  }

  bool success = true;
  StringList files;

  if (std::holds_alternative<FileInput>(input)) {
    files = StringList{std::get<FileInput>(input).filename};
  } else {
    files = findFiles(logger, std::get<FindInput>(input));
  }

  for (String const& file : files) {
    if (options.inPlace) {
      output = [&file](FormattedJson const& json) { File::writeFile(reprWithLineEnding(json), file); };
    }
    success &= process(output, command, options, File::readFileString(file));
  }
  return success;
}

std::shared_ptr<::Star::JsonPath::Path> parsePath(String const& path) {
  if (path.beginsWith("/"))
    return std::make_shared<JsonPath::Pointer>(path);
  return std::make_shared<JsonPath::QueryPath>(path);
}

std::pair<std::shared_ptr<::Star::JsonPath::Path>, bool> parseGetPath(String path) {
  // --get and --opt have a special syntax for getting the child values of
  // the value at the given path. These end with *, e.g.:
  //    /foo/bar/*
  //    foo.bar.*
  //    foo.bar[*]

  bool children = false;
  if (path.endsWith("/*") || path.endsWith(".*")) {
    path = path.substr(0, path.size() - 2);
    children = true;
  } else if (path.endsWith("[*]")) {
    path = path.substr(0, path.size() - 3);
    children = true;
  }
  return std::make_pair(parsePath(path), children);
}

std::optional<ParsedArgs> parseArgs(int argc, char** argv) {
  // Skip the program name
  Deque<String> args = Deque<String>(argv + 1, argv + argc);

  ParsedArgs parsed;

  // Parse option arguments
  while (!args.empty()) {
    String const& arg = args.takeFirst();
    if (arg == "--get" || arg == "--opt") {
      // Retrieve values at a given path in the Json document
      if (!std::holds_alternative<std::monostate>(parsed.command) || args.empty())
        return {};
      std::shared_ptr<::Star::JsonPath::Path> path;
      bool children = false;
      std::tie(path, children) = parseGetPath(args.takeFirst());
      parsed.command = GetCommand{path, arg == "--opt", children};

    } else if (arg == "--set") {
      // Set the value at the given path in the Json document
      if (!std::holds_alternative<std::monostate>(parsed.command) || args.size() < 2)
        return {};
      std::shared_ptr<::Star::JsonPath::Path> path = parsePath(args.takeFirst());
      FormattedJson value = FormattedJson::parse(args.takeFirst());
      parsed.command = SetCommand{path, value};

    } else if (arg == "--add") {
      // Add (insert) a path to a Json document
      if (!std::holds_alternative<std::monostate>(parsed.command) || args.size() < 2)
        return {};
      std::shared_ptr<::Star::JsonPath::Path> path = parsePath(args.takeFirst());
      FormattedJson value = FormattedJson::parse(args.takeFirst());
      parsed.command = AddCommand{path, value};

    } else if (arg == "--remove") {
      // Remove a path from a Json document
      if (!std::holds_alternative<std::monostate>(parsed.command) || args.empty())
        return {};
      parsed.command = RemoveCommand{parsePath(args.takeFirst())};

    } else if (arg == "--edit") {
      // Interactive bullk Json editor
      if (!std::holds_alternative<std::monostate>(parsed.command) || args.empty())
        return {};
      parsed.command = EditCommand{parsePath(args.takeFirst())};

    } else if (arg == "--editor-image") {
      if (args.empty())
        return {};
      parsed.options.editorImages.append(parsePath(args.takeFirst()));

    } else if (arg == "--input") {
      // Configure the input syntax for --edit
      if (args.empty())
        return {};
      String format = args.takeFirst();
      if (format == "json" || format == "generic")
        parsed.options.editFormat = std::make_shared<GenericInputFormat>();
      else if (format == "css" || format == "csv")
        parsed.options.editFormat = std::make_shared<CommaSeparatedStrings>();
      else if (format == "string")
        parsed.options.editFormat = std::make_shared<StringInputFormat>();
      else
        return {};

    } else if (arg == "--array") {
      // Output multiple results as a single array
      parsed.options.output = std::make_shared<ArrayOutput>(false);

    } else if (arg == "--array-unique") {
      // Output multiple results as a single array, with duplicate results
      // removed
      parsed.options.output = std::make_shared<ArrayOutput>(true);

    } else if (arg == "--help") {
      return {};

    } else if (arg == "-j") {
      // Use command line argument as input
      parsed.inputs.append(JsonLiteralInput{args.takeFirst()});

    } else if (arg == "--find") {
      // Search for files recursively in the given directory with a given
      // suffix.
      if (args.size() < 2)
        return {};
      String directory = args.takeFirst();
      String suffix = args.takeFirst();
      parsed.inputs.append(FindInput{directory, suffix});

    } else if (arg == "-i") {
      // Update files in place rather than print to stdout
      parsed.options.inPlace = true;

    } else if (arg == "--at") {
      // Insert new object keys at the beginning or end of the document
      if (!std::holds_alternative<std::monostate>(parsed.options.insertLocation) || args.size() < 1)
        return {};
      String pos = args.takeFirst();
      if (pos == "beginning" || pos == "start")
        parsed.options.insertLocation = AtBeginning{};
      else if (pos == "end")
        parsed.options.insertLocation = AtEnd{};
      else
        return {};

    } else if (arg == "--before") {
      // Insert new object keys before the given key
      if (!std::holds_alternative<std::monostate>(parsed.options.insertLocation) || args.size() < 1)
        return {};
      parsed.options.insertLocation = BeforeKey{args.takeFirst()};

    } else if (arg == "--after") {
      // Insert new object keys after the given key
      if (!std::holds_alternative<std::monostate>(parsed.options.insertLocation) || args.size() < 1)
        return {};
      parsed.options.insertLocation = AfterKey{args.takeFirst()};

    } else {
      if (!File::exists(arg)) {
        cerrf("File {} doesn't exist\n", arg);
        return {};
      }
      parsed.inputs.append(FileInput{arg});
    }
  }

  if (!parsed.options.output)
    parsed.options.output = std::make_shared<OutputOnSeparateLines>();

  bool anyFileInputs = false, anyNonFileInputs = false;
  for (Input const& input : parsed.inputs) {
    if (std::holds_alternative<FindInput>(input) || std::holds_alternative<FileInput>(input)) {
      anyFileInputs = true;
    } else {
      anyNonFileInputs = true;
    }
  }
  if (std::holds_alternative<EditCommand>(parsed.command) && !anyFileInputs) {
    cerrf("Files to edit must be supplied when using --edit.\n");
    return {};
  }

  if (parsed.options.inPlace && !anyFileInputs) {
    cerrf("In-place writing (-i) can only be used with files specified on the command line.\n");
    return {};
  }
  if (parsed.options.inPlace && std::holds_alternative<EditCommand>(parsed.command)) {
    cerrf("Interactive edit (--edit) is always in-place. Explicitly specifying -i is not needed.\n");
    return {};
  }
  if (std::holds_alternative<EditCommand>(parsed.command) && anyNonFileInputs) {
    cerrf("Interactie edit (--edit) can only be used with file input sources.\n");
    return {};
  }

  if (parsed.options.editFormat && !std::holds_alternative<EditCommand>(parsed.command)) {
    cerrf("--input can only be used with --edit.\n");
    return {};
  } else if (!parsed.options.editFormat && std::holds_alternative<EditCommand>(parsed.command)) {
    parsed.options.editFormat = std::make_shared<GenericInputFormat>();
  }

  return parsed;
}

String readStdin() {
  String result;
  char buffer[1024];
  while (!std::feof(stdin)) {
    std::size_t readBytes = std::fread(buffer, 1, 1024, stdin);
    result.append(buffer, readBytes);
  }
  return result;
}

int main(int argc, char** argv) {
  [[maybe_unused]] Star::NativeRuntime nativeRuntime;
  try {
    Logger logger;
    std::optional<ParsedArgs> parsedArgs = parseArgs(argc, argv);

    if (!parsedArgs) {
      cerrf("Usage: {} [--get <json-path>] (-j <json> | <json-file>*)\n", argv[0]);
      cerrf(
          "Usage: {} --set <json-path> <json> [-i] [(--at (beginning|end) | --before <key> | --after <key>)] (-j "
          "<json> | <json-file>*)\n",
          argv[0]);
      cerrf(
          "Usage: {} --add <json-path> <json> [-i] [(--at (beginning|end) | --before <key> | --after <key>)] (-j "
          "<json> | <json-file>*)\n",
          argv[0]);
      cerrf(
          "Usage: {} --edit <json-path> [(--at (beginning|end) | --before <key> | --after <key>)] [--input "
          "(csv|json|string)] <json-file>+\n",
          argv[0]);
      cerrf("\n");
      cerrf("Example: {} --get /dialog/0/message guard.npctype\n", argv[0]);
      cerrf("Example: {} --get 'foo[0]' -j '{{\"foo\":[0,1,2,3]}}'\n", argv[0]);
      cerrf("Example: {} --edit /tags --input csv --find ../assets/ .object\n", argv[0]);
      return 1;
    }

    std::shared_ptr<::Star::Output> output = parsedArgs->options.output;
    bool success = true;

    if (std::holds_alternative<EditCommand>(parsedArgs->command)) {
      return edit(logger, argc, argv, std::get<EditCommand>(parsedArgs->command).path, parsedArgs->options, parsedArgs->inputs);

    } else if (parsedArgs->inputs.size() == 0) {
      // No files were provided. Reading from stdin
      success &= process(output->toFunction(), parsedArgs->command, parsedArgs->options, readStdin());
    } else {
      for (Input const& input : parsedArgs->inputs) {
        success &= process(logger, output->toFunction(), parsedArgs->command, parsedArgs->options, input);
      }
    }

    output->flush();

    if (!success)
      return 1;
    return 0;

  } catch (JsonParsingException const& e) {
    cerrf("{}\n", e.what());
    return 1;

  } catch (JsonException const& e) {
    cerrf("{}\n", e.what());
    return 1;

  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    cerrf("Exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
