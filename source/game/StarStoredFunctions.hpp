#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarMultiTable.hpp"
#include "StarParametricFunction.hpp"

namespace Star {

using StoredFunctionException = ExceptionDerived<"StoredFunctionException">;

enum class Monotonicity { Flat,
                          Increasing,
                          Decreasing,
                          None };

// Interpolated function from single input to single output read from
// configuration.
class StoredFunction {
public:
  struct SearchResult {
    double targetValue;
    double searchTolerance;

    // Whether or not a solution was found within the given tolerance.
    bool found;
    // The resulting input that was found.
    double solution;
    // The value that is gotten from evaluating the function at the solution.
    double value;
  };

  static double const DefaultSearchTolerance;

  StoredFunction(ParametricFunction<double, double> data);

  [[nodiscard]] auto monotonicity() const -> Monotonicity;

  [[nodiscard]] auto evaluate(double input) const -> double;

  // Search for the input which would give the target value.  Will only work if
  // the function Monotonicity is Increasing or Decreasing.
  [[nodiscard]] auto search(double targetValue, double valueTolerance = DefaultSearchTolerance) const -> SearchResult;

private:
  Monotonicity m_monotonicity;
  ParametricFunction<double, double> m_function;
};

// Interpolated function from two inputs to an output read from configuration.
class StoredFunction2 {
public:
  StoredFunction2(MultiTable2D table);

  [[nodiscard]] auto evaluate(double x, double y) const -> double;

private:
  MultiTable2D table;
};

// Function from a single input to some generic configuration.
class StoredConfigFunction {
public:
  StoredConfigFunction(ParametricTable<int, Json> data);

  [[nodiscard]] auto get(double input) const -> Json;

private:
  ParametricTable<int, Json> m_data;
};

class FunctionDatabase {
public:
  FunctionDatabase();

  [[nodiscard]] auto namedFunctions() const -> StringList;
  [[nodiscard]] auto namedFunctions2() const -> StringList;
  [[nodiscard]] auto namedConfigFunctions() const -> StringList;

  // If configOrName is a string, loads the named function.  If it is an inline
  // config, reads the inline config.
  [[nodiscard]] auto function(Json const& configOrName) const -> Ptr<StoredFunction>;
  [[nodiscard]] auto function2(Json const& configOrName) const -> Ptr<StoredFunction2>;
  [[nodiscard]] auto configFunction(Json const& configOrName) const -> Ptr<StoredConfigFunction>;

private:
  static auto parametricFunctionFromConfig(Json descriptor) -> ParametricFunction<double, double>;
  static auto parametricTableFromConfig(Json descriptor) -> ParametricTable<int, Json>;
  static auto multiTable2DFromConfig(Json descriptor) -> MultiTable2D;

  StringMap<Ptr<StoredFunction>> m_functions;
  StringMap<Ptr<StoredFunction2>> m_functions2;
  StringMap<Ptr<StoredConfigFunction>> m_configFunctions;
};

}// namespace Star
