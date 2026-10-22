module;
import std;
import star.string;
import star.format;
import star.json;

import star.parametric_function;
import star.multi_array_interpolator;
import star.multi_table;

namespace Star {
class Assets;


struct StoredFunctionExceptionTag {
  static constexpr char const* name() { return "StoredFunctionException"; }
};
using StoredFunctionException = StarError<StoredFunctionExceptionTag, StarException>;

enum class Monotonicity { Flat, Increasing, Decreasing, None };

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

  [[nodiscard]] static std::expected<std::shared_ptr<StoredFunction>, StarException> create(ParametricFunction<double, double> data);

  Monotonicity monotonicity() const;

  [[nodiscard]] std::expected<double, StarException> evaluate(double input) const;

  // Search for the input which would give the target value.  Will only work if
  // the function Monotonicity is Increasing or Decreasing.
  [[nodiscard]] std::expected<SearchResult, StarException> search(double targetValue, double valueTolerance = DefaultSearchTolerance) const;

private:
  StoredFunction(ParametricFunction<double, double> data, Monotonicity monotonicity);
  Monotonicity m_monotonicity;
  ParametricFunction<double, double> m_function;
};

// Interpolated function from two inputs to an output read from configuration.
class StoredFunction2 {
public:
  StoredFunction2(MultiTable2D table);

  [[nodiscard]] std::expected<double, StarException> evaluate(double x, double y) const;

private:
  MultiTable2D table;
};

// Function from a single input to some generic configuration.
class StoredConfigFunction {
public:
  StoredConfigFunction(ParametricTable<int, Json> data);

  [[nodiscard]] std::expected<Json, StarException> get(double input) const;

private:
  ParametricTable<int, Json> m_data;
};

class FunctionDatabase {
public:
  [[nodiscard]] static std::expected<std::shared_ptr<FunctionDatabase>, StarException> create(std::shared_ptr<Assets const> const& assets);

  StringList namedFunctions() const;
  StringList namedFunctions2() const;
  StringList namedConfigFunctions() const;

  // If configOrName is a string, loads the named function.  If it is an inline
  // config, reads the inline config.
  [[nodiscard]] std::expected<std::shared_ptr<StoredFunction>, StarException> function(Json const& configOrName) const;
  [[nodiscard]] std::expected<std::shared_ptr<StoredFunction2>, StarException> function2(Json const& configOrName) const;
  [[nodiscard]] std::expected<std::shared_ptr<StoredConfigFunction>, StarException> configFunction(Json const& configOrName) const;

private:
  FunctionDatabase() = default;
  [[nodiscard]] static std::expected<ParametricFunction<double, double>, StarException> parametricFunctionFromConfig(Json descriptor);
  [[nodiscard]] static std::expected<ParametricTable<int, Json>, StarException> parametricTableFromConfig(Json descriptor);
  [[nodiscard]] static std::expected<MultiTable2D, StarException> multiTable2DFromConfig(Json descriptor);

  StringMap<std::shared_ptr<StoredFunction>> m_functions;
  StringMap<std::shared_ptr<StoredFunction2>> m_functions2;
  StringMap<std::shared_ptr<StoredConfigFunction>> m_configFunctions;
};

}

export module star.stored_functions;

export namespace Star {
  using ::Star::StoredFunctionExceptionTag;
  using ::Star::StoredFunctionException;
  using ::Star::Monotonicity;
  using ::Star::StoredFunction;
  using ::Star::StoredFunction2;
  using ::Star::StoredConfigFunction;
  using ::Star::FunctionDatabase;
}
