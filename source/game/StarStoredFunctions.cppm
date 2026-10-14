module;
import std;
import star.string;
import star.format;
import star.json;

import star.parametric_function;
import star.multi_array_interpolator;
import star.multi_table;

namespace Star {

class StoredFunction;
using StoredFunctionPtr = std::shared_ptr<StoredFunction>;
using StoredFunctionConstPtr = std::shared_ptr<const StoredFunction>;
using StoredFunctionWeakPtr = std::weak_ptr<StoredFunction>;
using StoredFunctionConstWeakPtr = std::weak_ptr<const StoredFunction>;
using StoredFunctionUPtr = std::unique_ptr<StoredFunction>;
using StoredFunctionConstUPtr = std::unique_ptr<const StoredFunction>;
class StoredFunction2;
using StoredFunction2Ptr = std::shared_ptr<StoredFunction2>;
using StoredFunction2ConstPtr = std::shared_ptr<const StoredFunction2>;
using StoredFunction2WeakPtr = std::weak_ptr<StoredFunction2>;
using StoredFunction2ConstWeakPtr = std::weak_ptr<const StoredFunction2>;
using StoredFunction2UPtr = std::unique_ptr<StoredFunction2>;
using StoredFunction2ConstUPtr = std::unique_ptr<const StoredFunction2>;
class StoredConfigFunction;
using StoredConfigFunctionPtr = std::shared_ptr<StoredConfigFunction>;
using StoredConfigFunctionConstPtr = std::shared_ptr<const StoredConfigFunction>;
using StoredConfigFunctionWeakPtr = std::weak_ptr<StoredConfigFunction>;
using StoredConfigFunctionConstWeakPtr = std::weak_ptr<const StoredConfigFunction>;
using StoredConfigFunctionUPtr = std::unique_ptr<StoredConfigFunction>;
using StoredConfigFunctionConstUPtr = std::unique_ptr<const StoredConfigFunction>;
class FunctionDatabase;
using FunctionDatabasePtr = std::shared_ptr<FunctionDatabase>;
using FunctionDatabaseConstPtr = std::shared_ptr<const FunctionDatabase>;
using FunctionDatabaseWeakPtr = std::weak_ptr<FunctionDatabase>;
using FunctionDatabaseConstWeakPtr = std::weak_ptr<const FunctionDatabase>;
using FunctionDatabaseUPtr = std::unique_ptr<FunctionDatabase>;
using FunctionDatabaseConstUPtr = std::unique_ptr<const FunctionDatabase>;

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

  StoredFunction(ParametricFunction<double, double> data);

  Monotonicity monotonicity() const;

  double evaluate(double input) const;

  // Search for the input which would give the target value.  Will only work if
  // the function Monotonicity is Increasing or Decreasing.
  SearchResult search(double targetValue, double valueTolerance = DefaultSearchTolerance) const;

private:
  Monotonicity m_monotonicity;
  ParametricFunction<double, double> m_function;
};

// Interpolated function from two inputs to an output read from configuration.
class StoredFunction2 {
public:
  StoredFunction2(MultiTable2D table);

  double evaluate(double x, double y) const;

private:
  MultiTable2D table;
};

// Function from a single input to some generic configuration.
class StoredConfigFunction {
public:
  StoredConfigFunction(ParametricTable<int, Json> data);

  Json get(double input) const;

private:
  ParametricTable<int, Json> m_data;
};

class FunctionDatabase {
public:
  FunctionDatabase();

  StringList namedFunctions() const;
  StringList namedFunctions2() const;
  StringList namedConfigFunctions() const;

  // If configOrName is a string, loads the named function.  If it is an inline
  // config, reads the inline config.
  StoredFunctionPtr function(Json const& configOrName) const;
  StoredFunction2Ptr function2(Json const& configOrName) const;
  StoredConfigFunctionPtr configFunction(Json const& configOrName) const;

private:
  static ParametricFunction<double, double> parametricFunctionFromConfig(Json descriptor);
  static ParametricTable<int, Json> parametricTableFromConfig(Json descriptor);
  static MultiTable2D multiTable2DFromConfig(Json descriptor);

  StringMap<StoredFunctionPtr> m_functions;
  StringMap<StoredFunction2Ptr> m_functions2;
  StringMap<StoredConfigFunctionPtr> m_configFunctions;
};

}

export module star.stored_functions;

export namespace Star {
  using ::Star::StoredFunctionExceptionTag;
  using ::Star::StoredFunctionException;
  using ::Star::Monotonicity;
  using ::Star::StoredFunction;
  using ::Star::StoredFunctionPtr;
  using ::Star::StoredFunctionConstPtr;
  using ::Star::StoredFunctionWeakPtr;
  using ::Star::StoredFunctionConstWeakPtr;
  using ::Star::StoredFunctionUPtr;
  using ::Star::StoredFunctionConstUPtr;
  using ::Star::StoredFunction2;
  using ::Star::StoredFunction2Ptr;
  using ::Star::StoredFunction2ConstPtr;
  using ::Star::StoredFunction2WeakPtr;
  using ::Star::StoredFunction2ConstWeakPtr;
  using ::Star::StoredFunction2UPtr;
  using ::Star::StoredFunction2ConstUPtr;
  using ::Star::StoredConfigFunction;
  using ::Star::StoredConfigFunctionPtr;
  using ::Star::StoredConfigFunctionConstPtr;
  using ::Star::StoredConfigFunctionWeakPtr;
  using ::Star::StoredConfigFunctionConstWeakPtr;
  using ::Star::StoredConfigFunctionUPtr;
  using ::Star::StoredConfigFunctionConstUPtr;
  using ::Star::FunctionDatabase;
  using ::Star::FunctionDatabasePtr;
  using ::Star::FunctionDatabaseConstPtr;
  using ::Star::FunctionDatabaseWeakPtr;
  using ::Star::FunctionDatabaseConstWeakPtr;
  using ::Star::FunctionDatabaseUPtr;
  using ::Star::FunctionDatabaseConstUPtr;
}
