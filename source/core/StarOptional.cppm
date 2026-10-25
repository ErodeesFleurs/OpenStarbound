module;
import std;
import star.stream_formatter;
import star.format;

namespace Star {

// Failed extraction leaves the optional unchanged.
template <typename T>
[[nodiscard]]
std::expected<T, StarException> takeOptional(std::optional<T>& value) {
  if (!value)
    return std::unexpected(OutOfRangeException("takeOptional called on empty optional"));
  T result(std::move(*value));
  value.reset();
  return result;
}

// A nullable borrow never owns storage. Only borrow from a named, live owner.
template <typename T>
std::optional<std::reference_wrapper<T>> optionalRef(std::optional<T>& value) {
  if (value)
    return std::reference_wrapper<T>(*value);
  return std::nullopt;
}

template <typename T>
std::optional<std::reference_wrapper<T const>> optionalRef(std::optional<T> const& value) {
  if (value)
    return std::reference_wrapper<T const>(*value);
  return std::nullopt;
}

template <typename T>
void optionalRef(std::optional<T>&&) = delete;
template <typename T>
void optionalRef(std::optional<T> const&&) = delete;

template <typename Owner>
  requires (std::is_lvalue_reference_v<Owner&&>
      && requires(Owner& owner) { owner.get(); }
      && std::is_pointer_v<decltype(std::declval<Owner&>().get())>)
auto optionalRef(Owner&& owner)
    -> std::optional<std::reference_wrapper<std::remove_pointer_t<decltype(owner.get())>>> {
  using Value = std::remove_pointer_t<decltype(owner.get())>;
  if (owner)
    return std::reference_wrapper<Value>(*owner);
  return std::nullopt;
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
decltype(auto) optionalDiagnosticValue(T const& value) {
  if constexpr (requires { outputOptional(value); })
    return outputOptional(value);
  else
    return (value);
}

template <typename T>
using OptionalDiagnosticResult = decltype(std::declval<std::ostream&>() << optionalDiagnosticValue(std::declval<T const&>()));

template <typename T>
std::ostream& operator<<(std::ostream& output, OptionalOutput<T> value)
    requires std::same_as<OptionalDiagnosticResult<T>, std::ostream&> {
  if (!value.value)
    return output << "Nothing";
  output << "Just (" << optionalDiagnosticValue(*value.value);
  return output << ")";
}

template <typename T>
[[nodiscard]]
std::expected<std::reference_wrapper<std::ostream>, StarException> operator<<(std::ostream& output, OptionalOutput<T> value)
    requires std::same_as<OptionalDiagnosticResult<T>, std::expected<std::reference_wrapper<std::ostream>, StarException>> {
  if (!value.value) {
    output << "Nothing";
    return std::ref(output);
  }
  output << "Just (";
  auto written = output << optionalDiagnosticValue(*value.value);
  if (!written)
    return std::unexpected(std::move(written.error()));
  output << ")";
  return std::ref(output);
}

}

export module star.optional;

export template <typename T>
  requires std::same_as<decltype(std::declval<std::ostream&>() << std::declval<Star::OptionalOutput<T>>()), std::ostream&>
struct std::formatter<Star::OptionalOutput<T>> : Star::StreamFormatter {};

export namespace Star {
  using ::Star::takeOptional;
  using ::Star::optionalRef;
  using ::Star::OptionalOutput;
  using ::Star::outputOptional;
  using ::Star::operator<<;
}
