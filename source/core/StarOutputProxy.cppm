module;
import std;
import star.stream_formatter;
import star.type_identity;
import star.optional;

namespace Star {

namespace OutputAnyDetail {
  template<typename T, typename CharT, typename Traits>
  std::basic_string<CharT, Traits> string(T const& t) {
    return std::format("<type {} at address: {}>", typeName<T>(), (void*)&t);
  }

  template<typename T, typename CharT, typename Traits>
  std::basic_ostream<CharT, Traits>& output(std::basic_ostream<CharT, Traits>& os, T const& t) {
    return os << string<T, CharT, Traits>(t);
  }

  namespace Operator {
    template<typename T, typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, T const& t) {
      return output(os, t);
    }
  }

  template <typename T>
  struct Wrapper {
    T const& wrapped;
  };

  template <typename T>
  std::ostream& operator<<(std::ostream& os, Wrapper<T> const& wrapper) {
    using namespace Operator;
    if constexpr (requires { outputOptional(wrapper.wrapped); })
      return os << outputOptional(wrapper.wrapped);
    else
      return os << wrapper.wrapped;
  }
}

// Wraps a type so that is printable no matter what..  If no operator<< is
// defined for a type, then will print <type [type name] at address: [address]>
template <typename T>
OutputAnyDetail::Wrapper<T> outputAny(T const& t) {
  return OutputAnyDetail::Wrapper<T>{t};
}

struct OutputProxy {
  typedef std::function<void(std::ostream&)> PrintFunction;

  OutputProxy(PrintFunction p)
    : print(std::move(p)) {}

  PrintFunction print;
};

inline std::ostream& operator<<(std::ostream& os, OutputProxy const& p) {
  p.print(os);
  return os;
}

}

export module star.output_proxy;

export namespace Star {
  using ::Star::outputAny;
  using ::Star::OutputProxy;
  using ::Star::operator<<;
}

export namespace Star::OutputAnyDetail {
  using ::Star::OutputAnyDetail::string;
  using ::Star::OutputAnyDetail::output;
  using ::Star::OutputAnyDetail::Wrapper;
  using ::Star::OutputAnyDetail::operator<<;
}

export namespace Star::OutputAnyDetail::Operator {
  using ::Star::OutputAnyDetail::Operator::operator<<;
}

export template <typename T>
struct std::formatter<Star::OutputAnyDetail::Wrapper<T>> : Star::StreamFormatter {};
export template <> struct std::formatter<Star::OutputProxy> : Star::StreamFormatter {};
