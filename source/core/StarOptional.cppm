module;
import std;
import star.stream_formatter;

namespace Star {

// Extract only after a successful move; a throwing move leaves the optional engaged.
template <typename T>
T takeOptional(std::optional<T>& value) {
  T result(std::move(value.value()));
  value.reset();
  return result;
}

template <typename T>
T* optionalPtr(std::optional<T>& value) {
  return value ? std::addressof(*value) : nullptr;
}

template <typename T>
T const* optionalPtr(std::optional<T> const& value) {
  return value ? std::addressof(*value) : nullptr;
}

// Explicit diagnostic view: std::optional itself has no ostream/formatter hook.
// The referenced optional must outlive the view, including deferred formatting.
template <typename T>
struct OptionalOutput {
  std::optional<T> const& value;
};

template <typename T>
OptionalOutput<T> outputOptional(std::optional<T> const& value) {
  return {value};
}

template <typename T>
std::ostream& operator<<(std::ostream& output, OptionalOutput<T> value) {
  if (!value.value)
    return output << "Nothing";
  output << "Just (";
  if constexpr (requires { typename T::value_type; } && requires { outputOptional(*value.value); })
    output << outputOptional(*value.value);
  else
    output << *value.value;
  return output << ")";
}

}

export module star.optional;

export template <typename T>
struct std::formatter<Star::OptionalOutput<T>> : Star::StreamFormatter {};

export namespace Star {
  using ::Star::takeOptional;
  using ::Star::optionalPtr;
  using ::Star::OptionalOutput;
  using ::Star::outputOptional;
  using ::Star::operator<<;
}
