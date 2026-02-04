#pragma once

#include <format>
#include <sstream>

namespace Star {

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

}