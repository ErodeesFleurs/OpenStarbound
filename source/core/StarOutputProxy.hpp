#pragma once

#include "StarOstreamFormatter.hpp"

import std;

namespace Star {

namespace OutputAnyDetail {
template <typename T, typename CharT, typename Traits>
auto string(T const& t) -> std::basic_string<CharT, Traits> {
  return std::format("<type {} at address: {}>", typeid(T).name(), (void*)&t);
}

template <typename T, typename CharT, typename Traits>
auto output(std::basic_ostream<CharT, Traits>& os, T const& t) -> std::basic_ostream<CharT, Traits>& {
  return os << string<T, CharT, Traits>(t);
}

namespace Operator {
template <typename T, typename CharT, typename Traits>
auto operator<<(std::basic_ostream<CharT, Traits>& os, T const& t) -> std::basic_ostream<CharT, Traits>& {
  return output(os, t);
}
}// namespace Operator

template <typename T>
struct Wrapper {
  T const& wrapped;
};

template <typename T>
auto operator<<(std::ostream& os, Wrapper<T> const& wrapper) -> std::ostream& {
  using namespace Operator;
  return os << wrapper.wrapped;
}
}// namespace OutputAnyDetail

// Wraps a type so that is printable no matter what..  If no operator<< is
// defined for a type, then will print <type [typeid] at address: [address]>
template <typename T>
auto outputAny(T const& t) -> OutputAnyDetail::Wrapper<T> {
  return OutputAnyDetail::Wrapper<T>{t};
}

struct OutputProxy {
  using PrintFunction = std::function<void(std::ostream&)>;

  OutputProxy(PrintFunction p)
      : print(std::move(p)) {}

  PrintFunction print;
};

inline auto operator<<(std::ostream& os, OutputProxy const& p) -> std::ostream& {
  p.print(os);
  return os;
}

}// namespace Star

template <typename T>
struct std::formatter<Star::OutputAnyDetail::Wrapper<T>> : Star::ostream_formatter {};
template <>
struct std::formatter<Star::OutputProxy> : Star::ostream_formatter {};
