#pragma once

#include "StarException.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

using JsonPatchException = ExceptionDerived<"JsonPatchException", JsonException>;
using JsonPatchTestFail = ExceptionDerived<"JsonPatchTestFail">;

// Applies the given RFC6902 compliant patch to the base and returns the result
// Throws JsonPatchException on patch failure.
auto jsonPatch(Json const& base, JsonArray const& patch) -> Json;

namespace JsonPatching {
  // Applies the given single operation
  auto applyOperation(Json const& base, Json const& op, std::optional<Json> const& external = {}) -> Json;

  // Tests for "value" at "path"
  // Returns base or throws JsonPatchException
  auto applyTestOperation(Json const& base, Json const& op) -> Json;

  // Removes the value at "path"
  auto applyRemoveOperation(Json const& base, Json const& op) -> Json;

  // Adds "value" at "path"
  auto applyAddOperation(Json const& base, Json const& op) -> Json;

  // Replaces "path" with "value"
  auto applyReplaceOperation(Json const& base, Json const& op) -> Json;

  // Moves "from" to "path"
  auto applyMoveOperation(Json const& base, Json const& op) -> Json;

  // Copies "from" to "path"
  auto applyCopyOperation(Json const& base, Json const& op) -> Json;

  // Merges "value" at "path"
  auto applyMergeOperation(Json const& base, Json const& op) -> Json;
  }

}
