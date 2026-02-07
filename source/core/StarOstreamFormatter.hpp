#pragma once

import std;

namespace Star {

template <typename T>
auto operator<<(std::ostream& os, std::optional<T> const& opt) -> std::ostream& {
  if (opt)
    return os << "Just (" << *opt << ")";
  else
    return os << "Nothing";
}

struct ostream_formatter {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) const {
    return ctx.begin();
  }

  template <typename T, typename FormatContext>
  auto format(T const& t, FormatContext& ctx) const {
    std::ostringstream oss;
    oss << t;
    return std::format_to(ctx.out(), "{}", oss.str());
  }
};

}// namespace Star
