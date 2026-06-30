#pragma once

#include "StarJson.hpp"

namespace Star {

struct JsonPatchExceptionTag { static constexpr char const* typeName = "JsonPatchException"; };
using JsonPatchException = TypedException<JsonException, JsonPatchExceptionTag>;
struct JsonPatchTestFailTag { static constexpr char const* typeName = "JsonPatchTestFail"; };
using JsonPatchTestFail = TypedException<StarException, JsonPatchTestFailTag>;

// Applies the given RFC6902 compliant patch to the base and returns the result
// Throws JsonPatchException on patch failure.
[[nodiscard]] Json jsonPatch(Json const& base, JsonArray const& patch);

namespace JsonPatching {
  // Applies the given single operation
  [[nodiscard]] Json applyOperation(Json const& base, Json const& op, Maybe<Json> const& external = {});

  // Tests for "value" at "path"
  // Returns base or throws JsonPatchException
  [[nodiscard]] Json applyTestOperation(Json const& base, Json const& op);

  // Removes the value at "path"
  [[nodiscard]] Json applyRemoveOperation(Json const& base, Json const& op);

  // Adds "value" at "path"
  [[nodiscard]] Json applyAddOperation(Json const& base, Json const& op);

  // Replaces "path" with "value"
  [[nodiscard]] Json applyReplaceOperation(Json const& base, Json const& op);

  // Moves "from" to "path"
  [[nodiscard]] Json applyMoveOperation(Json const& base, Json const& op);

  // Copies "from" to "path"
  [[nodiscard]] Json applyCopyOperation(Json const& base, Json const& op);

  // Merges "value" at "path"
  [[nodiscard]] Json applyMergeOperation(Json const& base, Json const& op);
  }

}
