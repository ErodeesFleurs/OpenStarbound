module;
import std;
import star.string;
import star.map;
import star.format;
import star.json;
import star.json_path;

export module star.json_patch;

export namespace Star {
struct JsonPatchExceptionTag {
  static constexpr std::string_view name() { return "JsonPatchException"; }
};
using JsonPatchException = StarError<JsonPatchExceptionTag, JsonException>;
struct JsonPatchTestFailTag {
  static constexpr std::string_view name() { return "JsonPatchTestFail"; }
};
using JsonPatchTestFail = StarError<JsonPatchTestFailTag, StarException>;

// Applies RFC6902 operations, preserving patch diagnostics and test-failure categories.
std::expected<Json, StarException> jsonPatch(Json const& base, JsonArray const& patch);
namespace JsonPatching {
  std::expected<Json, StarException> applyOperation(Json const& base, Json const& op, std::optional<Json> const& external = {});
  std::expected<Json, StarException> applyTestOperation(Json const& base, Json const& op);
  std::expected<Json, StarException> applyRemoveOperation(Json const& base, Json const& op);
  std::expected<Json, StarException> applyAddOperation(Json const& base, Json const& op);
  std::expected<Json, StarException> applyReplaceOperation(Json const& base, Json const& op);
  std::expected<Json, StarException> applyMoveOperation(Json const& base, Json const& op);
  std::expected<Json, StarException> applyCopyOperation(Json const& base, Json const& op);
  std::expected<Json, StarException> applyMergeOperation(Json const& base, Json const& op);
}
}

namespace Star {
std::expected<Json, StarException> jsonPatch(Json const& base, JsonArray const& patch) {
  Json result = base;
  for (auto const& operation : patch) {
    auto applied = JsonPatching::applyOperation(result, operation);
    if (!applied) {
      if (applied.error().is<JsonException>())
        return std::unexpected(JsonPatchException(strf("Could not apply patch to base. {}", applied.error().what()), false));
      return std::unexpected(std::move(applied.error()));
    }
    result = std::move(*applied);
  }
  return result;
}

// Zero means absent; otherwise the result is the matching index plus one.
std::expected<std::size_t, StarException> findJsonMatch(Json const& searchable, Json const& value, JsonPath::Pointer const& pointer) {
  if (!searchable.isType(Json::Type::Array))
    return std::unexpected(JsonPatchException(strf("Search operation failure, value at '{}' is not an array.", pointer.path()), false));
  auto array = searchable.arrayPtr();
  if (!array)
    return std::unexpected(std::move(array.error()));
  for (std::size_t i = 0; i < (*array)->size(); ++i)
    if (jsonPartialMatch((**array)[i], value))
      return i + 1;
  return 0;
}

namespace JsonPatching {
  using Operation = std::reference_wrapper<std::expected<Json, StarException>(Json const&, Json const&)>;
  using OperationEntry = std::pair<std::string_view, Operation>;
  static constexpr std::array<OperationEntry, 7> functionMap{{
      {"test", std::ref(applyTestOperation)}, {"remove", std::ref(applyRemoveOperation)},
      {"add", std::ref(applyAddOperation)}, {"replace", std::ref(applyReplaceOperation)},
      {"move", std::ref(applyMoveOperation)}, {"copy", std::ref(applyCopyOperation)},
      {"merge", std::ref(applyMergeOperation)},
  }};

  std::expected<Json, StarException> applyOperation(Json const& base, Json const& op, std::optional<Json> const&) {
    auto operation = op.getString("op");
    if (!operation) {
      if (operation.error().is<JsonException>())
        return std::unexpected(JsonPatchException(strf("Could not apply operation to base. {}", operation.error().what()), false));
      return std::unexpected(std::move(operation.error()));
    }
    auto function = std::ranges::find(functionMap, operation->utf8(), &OperationEntry::first);
    if (function == functionMap.end())
      return std::unexpected(JsonPatchException(strf("Invalid operation: {}", *operation), false));
    auto applied = function->second(base, op);
    if (!applied && applied.error().is<JsonException>())
      return std::unexpected(JsonPatchException(strf("Could not apply operation to base. {}", applied.error().what()), false));
    return applied;
  }

  std::expected<Json, StarException> applyTestOperation(Json const& base, Json const& op) {
    auto path = op.getString("path");
    if (!path)
      return std::unexpected(std::move(path.error()));
    auto inverse = op.getBool("inverse", false);
    if (!inverse)
      return std::unexpected(std::move(inverse.error()));
    JsonPath::Pointer pointer(*path);
    auto tested = [&]() -> std::expected<Json, StarException> {
      auto search = op.contains("search");
      if (!search)
        return std::unexpected(std::move(search.error()));
      if (*search) {
        auto searchable = pointer.get(base);
        if (!searchable)
          return std::unexpected(std::move(searchable.error()));
        auto value = op.get("search");
        if (!value)
          return std::unexpected(std::move(value.error()));
        auto found = findJsonMatch(*searchable, *value, pointer);
        if (!found)
          return std::unexpected(std::move(found.error()));
        if ((*found != 0) == *inverse) {
          auto text = value->repr();
          if (!text)
            return std::unexpected(std::move(text.error()));
          if (*inverse)
            return std::unexpected(JsonPatchTestFail(strf("Test operation failure, expected {} to be missing.", *text), false));
          return std::unexpected(JsonPatchTestFail(strf("Test operation failure, could not find {}.", *text), false));
        }
        return base;
      }
      auto value = op.opt("value");
      if (!value)
        return std::unexpected(std::move(value.error()));
      auto actual = pointer.get(base);
      if (!actual)
        return std::unexpected(std::move(actual.error()));
      if (!*value) {
        if (*inverse)
          return std::unexpected(JsonPatchTestFail(strf("Test operation failure, expected {} to be missing.", *path), false));
        return base;
      }
      if ((**value == *actual) != *inverse)
        return base;
      auto expectedText = (**value).repr();
      if (!expectedText)
        return std::unexpected(std::move(expectedText.error()));
      auto actualText = actual->repr();
      if (!actualText)
        return std::unexpected(std::move(actualText.error()));
      return std::unexpected(JsonPatchTestFail(strf("Test operation failure, expected Just ({}) found {}.", *expectedText, *actualText), false));
    }();
    if (!tested && tested.error().is<JsonPath::TraversalException>()) {
      if (*inverse)
        return base;
      return std::unexpected(JsonPatchTestFail(strf("Test operation failure: {}", tested.error().what()), false));
    }
    return tested;
  }

  std::expected<Json, StarException> applyRemoveOperation(Json const& base, Json const& op) {
    auto path = op.getString("path");
    if (!path)
      return std::unexpected(std::move(path.error()));
    JsonPath::Pointer pointer(*path);
    auto search = op.contains("search");
    if (!search)
      return std::unexpected(std::move(search.error()));
    if (!*search)
      return pointer.remove(base);
    auto searchable = pointer.get(base);
    if (!searchable)
      return std::unexpected(std::move(searchable.error()));
    auto value = op.get("search");
    if (!value)
      return std::unexpected(std::move(value.error()));
    auto index = findJsonMatch(*searchable, *value, pointer);
    if (!index)
      return std::unexpected(std::move(index.error()));
    if (!*index)
      return base;
    auto removed = pointer.remove(base);
    if (!removed)
      return std::unexpected(std::move(removed.error()));
    auto changed = searchable->eraseIndex(*index - 1);
    if (!changed)
      return std::unexpected(std::move(changed.error()));
    return pointer.add(*removed, *changed);
  }

  std::expected<Json, StarException> applyAddOperation(Json const& base, Json const& op) {
    auto path = op.getString("path");
    if (!path)
      return std::unexpected(std::move(path.error()));
    auto value = op.get("value");
    if (!value)
      return std::unexpected(std::move(value.error()));
    JsonPath::Pointer pointer(*path);
    auto search = op.contains("search");
    if (!search)
      return std::unexpected(std::move(search.error()));
    if (!*search)
      return pointer.add(base, *value);
    auto searchable = pointer.get(base);
    if (!searchable)
      return std::unexpected(std::move(searchable.error()));
    auto searchValue = op.get("search");
    if (!searchValue)
      return std::unexpected(std::move(searchValue.error()));
    auto index = findJsonMatch(*searchable, *searchValue, pointer);
    if (!index)
      return std::unexpected(std::move(index.error()));
    if (!*index)
      return base;
    auto removed = pointer.remove(base);
    if (!removed)
      return std::unexpected(std::move(removed.error()));
    auto changed = searchable->insert(*index - 1, *value);
    if (!changed)
      return std::unexpected(std::move(changed.error()));
    return pointer.add(*removed, *changed);
  }

  std::expected<Json, StarException> applyReplaceOperation(Json const& base, Json const& op) {
    auto path = op.getString("path");
    if (!path)
      return std::unexpected(std::move(path.error()));
    auto value = op.get("value");
    if (!value)
      return std::unexpected(std::move(value.error()));
    JsonPath::Pointer pointer(*path);
    auto search = op.contains("search");
    if (!search)
      return std::unexpected(std::move(search.error()));
    if (*search) {
      auto searchable = pointer.get(base);
      if (!searchable)
        return std::unexpected(std::move(searchable.error()));
      auto searchValue = op.get("search");
      if (!searchValue)
        return std::unexpected(std::move(searchValue.error()));
      auto index = findJsonMatch(*searchable, *searchValue, pointer);
      if (!index)
        return std::unexpected(std::move(index.error()));
      if (!*index)
        return base;
      auto removed = pointer.remove(base);
      if (!removed)
        return std::unexpected(std::move(removed.error()));
      auto changed = searchable->set(*index - 1, *value);
      if (!changed)
        return std::unexpected(std::move(changed.error()));
      return pointer.add(*removed, *changed);
    }
    auto removed = pointer.remove(base);
    if (!removed)
      return std::unexpected(std::move(removed.error()));
    return pointer.add(*removed, *value);
  }

  std::expected<Json, StarException> applyMoveOperation(Json const& base, Json const& op) {
    auto path = op.getString("path");
    if (!path)
      return std::unexpected(std::move(path.error()));
    auto from = op.getString("from");
    if (!from)
      return std::unexpected(std::move(from.error()));
    JsonPath::Pointer toPointer(*path), fromPointer(*from);
    auto search = op.contains("search");
    if (!search)
      return std::unexpected(std::move(search.error()));
    if (*search) {
      auto searchable = fromPointer.get(base);
      if (!searchable)
        return std::unexpected(std::move(searchable.error()));
      auto searchValue = op.get("search");
      if (!searchValue)
        return std::unexpected(std::move(searchValue.error()));
      auto index = findJsonMatch(*searchable, *searchValue, fromPointer);
      if (!index)
        return std::unexpected(std::move(index.error()));
      if (!*index)
        return base;
      auto value = searchable->get(*index - 1);
      if (!value)
        return std::unexpected(std::move(value.error()));
      auto added = toPointer.add(base, *value);
      if (!added)
        return std::unexpected(std::move(added.error()));
      auto changed = searchable->eraseIndex(*index - 1);
      if (!changed)
        return std::unexpected(std::move(changed.error()));
      return fromPointer.add(*added, *changed);
    }
    auto value = fromPointer.get(base);
    if (!value)
      return std::unexpected(std::move(value.error()));
    auto removed = fromPointer.remove(base);
    if (!removed)
      return std::unexpected(std::move(removed.error()));
    return toPointer.add(*removed, *value);
  }

  std::expected<Json, StarException> applyCopyOperation(Json const& base, Json const& op) {
    auto path = op.getString("path");
    if (!path)
      return std::unexpected(std::move(path.error()));
    auto from = op.getString("from");
    if (!from)
      return std::unexpected(std::move(from.error()));
    JsonPath::Pointer toPointer(*path), fromPointer(*from);
    auto search = op.contains("search");
    if (!search)
      return std::unexpected(std::move(search.error()));
    if (*search) {
      auto searchable = fromPointer.get(base);
      if (!searchable)
        return std::unexpected(std::move(searchable.error()));
      auto searchValue = op.get("search");
      if (!searchValue)
        return std::unexpected(std::move(searchValue.error()));
      auto index = findJsonMatch(*searchable, *searchValue, fromPointer);
      if (!index)
        return std::unexpected(std::move(index.error()));
      if (!*index)
        return base;
      auto value = searchable->get(*index - 1);
      if (!value)
        return std::unexpected(std::move(value.error()));
      return toPointer.add(base, *value);
    }
    auto value = fromPointer.get(base);
    if (!value)
      return std::unexpected(std::move(value.error()));
    return toPointer.add(base, *value);
  }

  std::expected<Json, StarException> applyMergeOperation(Json const& base, Json const& op) {
    auto path = op.getString("path");
    if (!path)
      return std::unexpected(std::move(path.error()));
    JsonPath::Pointer pointer(*path);
    auto search = op.contains("search");
    if (!search)
      return std::unexpected(std::move(search.error()));
    if (*search) {
      auto searchable = pointer.get(base);
      if (!searchable)
        return std::unexpected(std::move(searchable.error()));
      auto searchValue = op.get("search");
      if (!searchValue)
        return std::unexpected(std::move(searchValue.error()));
      auto index = findJsonMatch(*searchable, *searchValue, pointer);
      if (!index)
        return std::unexpected(std::move(index.error()));
      if (!*index)
        return base;
      auto oldValue = searchable->get(*index - 1);
      if (!oldValue)
        return std::unexpected(std::move(oldValue.error()));
      auto value = op.get("value");
      if (!value)
        return std::unexpected(std::move(value.error()));
      auto changed = searchable->set(*index - 1, jsonMerge(*oldValue, *value));
      if (!changed)
        return std::unexpected(std::move(changed.error()));
      auto removed = pointer.remove(base);
      if (!removed)
        return std::unexpected(std::move(removed.error()));
      return pointer.add(*removed, *changed);
    }
    auto oldValue = pointer.get(base);
    if (!oldValue)
      return std::unexpected(std::move(oldValue.error()));
    auto value = op.get("value");
    if (!value)
      return std::unexpected(std::move(value.error()));
    auto removed = pointer.remove(base);
    if (!removed)
      return std::unexpected(std::move(removed.error()));
    return pointer.add(*removed, jsonMerge(*oldValue, *value));
  }
}
}
