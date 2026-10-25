module;
import std;

import star.type_identity;

namespace Star {
struct OutputProxy;
#ifndef NDEBUG
inline constexpr bool DebugEnabled = true;
#else
inline constexpr bool DebugEnabled = false;
#endif



template <typename... T>
std::string strf(std::format_string<T...> fmt, T&&... args);


template <typename Char, typename... T>
std::string strfChecked(std::basic_format_string<Char, T...> fmt, T const&... args);

class StarException;

// Owning diagnostics remain valid after the originating error value is gone.
class ExceptionDiagnostic {
public:
  ExceptionDiagnostic(StarException const& exception);

  friend void printException(std::ostream& os, ExceptionDiagnostic const& e, bool fullStacktrace);
  friend OutputProxy outputException(ExceptionDiagnostic const& e, bool fullStacktrace);

private:
  friend class StarException;
  std::function<void(std::ostream&, bool)> m_printException;
};

class StarException {
public:
  template <typename... Args>
  static StarException format(std::format_string<Args...> fmt, Args const&... args);

  StarException() noexcept;
  ~StarException() noexcept;
  StarException(StarException const&) = default;
  StarException& operator=(StarException const&) = default;
  StarException(StarException&&) noexcept = default;
  StarException& operator=(StarException&&) noexcept = default;

  explicit StarException(std::string message, bool genStackTrace = true) noexcept;
  explicit StarException(ExceptionDiagnostic cause) noexcept;
  StarException(std::string message, ExceptionDiagnostic cause) noexcept;

  char const* what() const noexcept;

  template <typename Error>
  bool is() const noexcept {
    return m_matchesErrorType(typeKey<Error>());
  }

  static bool matchesErrorType(TypeKey key) noexcept {
    return key == typeKey<StarException>();
  }

  StarException withContext(std::string context) const& {
    return StarException(
        [context = std::move(context), cause = m_printException](std::ostream& os, bool fullStacktrace) {
          os << context << "\nCaused by: ";
          cause(os, fullStacktrace);
        },
        m_matchesErrorType);
  }

  StarException withContext(std::string context) && {
    return StarException(
        [context = std::move(context), cause = std::move(m_printException)](std::ostream& os, bool fullStacktrace) {
          os << context << "\nCaused by: ";
          cause(os, fullStacktrace);
        },
        m_matchesErrorType);
  }

  friend class ExceptionDiagnostic;

protected:
  StarException(std::string_view type, std::string message, bool genStackTrace = true) noexcept;
  StarException(std::string_view type, std::string message, ExceptionDiagnostic cause) noexcept;

  template <typename Error>
  void setErrorType() noexcept {
    m_matchesErrorType = &Error::matchesErrorType;
  }

private:
  bool (*m_matchesErrorType)(TypeKey) noexcept = &StarException::matchesErrorType;
  StarException(std::function<void(std::ostream&, bool)> printer,
      bool (*matcher)(TypeKey) noexcept) noexcept
    : m_matchesErrorType(matcher), m_printException(std::move(printer)) {}
  // Takes the ostream to print to, whether to print the full stacktrace.  Must
  // not bind 'this', may outlive the exception in the case of chained
  // exception causes.
  std::function<void(std::ostream&, bool)> m_printException;

  // m_printException will be called without the stack-trace to print
  // m_whatBuffer, if the what() method is invoked.
  mutable std::string m_whatBuffer;
};

void printException(std::ostream& os, ExceptionDiagnostic const& e, bool fullStacktrace);
std::string printException(ExceptionDiagnostic const& e, bool fullStacktrace);
OutputProxy outputException(ExceptionDiagnostic const& e, bool fullStacktrace);

void printStack(std::string_view message);

// Log error and stack-trace and possibly show a dialog box if available, then
// abort.
void fatalError(std::string_view message, bool showStackTrace);

// The file and line of an assertion are a language construct now
// (std::source_location) instead of __FILE__/__LINE__ stringification.
// Callers guard this function with `if constexpr (Star::DebugEnabled)` so a
// release build never evaluates the assertion operand.
// The condition is a forwarding reference, not a bool: it has to go through the
// same contextual conversion the old macro's `if (COND)` performed, otherwise
// types with an explicit operator bool (std::unique_ptr, for instance) stop
// compiling.
template <typename T>
inline void starAssertFailed(T&& condition, std::source_location location = std::source_location::current()) {
#ifndef NDEBUG
  if (condition)
    return;

  std::string const message = strf("assert failure in file {} line {}", location.file_name(), location.line());
  Star::fatalError(message, true);
#else
  (void)condition;
  (void)location;
#endif
}

inline void debugPrintStack(std::source_location location = std::source_location::current()) {
#ifndef NDEBUG
  // The message outlives the call: printStack may hand it on before returning.
  std::string const message = strf("Debug: file {} line {}", location.file_name(), location.line());
  Star::printStack(message);
#else
  (void)location;
#endif
}


// Domain error values preserve their category when stored as StarException.
template <typename Tag, typename Base>
class StarError : public Base {
public:
  template <typename... Args>
  static StarError format(std::format_string<Args...> fmt, Args const&... args) {
    return StarError(strfChecked(fmt, args...));
  }


  static bool matchesErrorType(TypeKey key) noexcept {
    return key == typeKey<StarError>() || Base::matchesErrorType(key);
  }

  StarError() : Base(Tag::name(), std::string(), true) { this->template setErrorType<StarError>(); }
  explicit StarError(std::string message, bool genStackTrace = true)
    : Base(Tag::name(), std::move(message), genStackTrace) { this->template setErrorType<StarError>(); }
  explicit StarError(ExceptionDiagnostic cause) : Base(Tag::name(), std::string(), std::move(cause)) {
    this->template setErrorType<StarError>();
  }
  StarError(std::string message, ExceptionDiagnostic cause) : Base(Tag::name(), std::move(message), std::move(cause)) {
    this->template setErrorType<StarError>();
  }

protected:
  StarError(std::string_view type, std::string message, bool genStackTrace = true)
    : Base(type, std::move(message), genStackTrace) { this->template setErrorType<StarError>(); }
  StarError(std::string_view type, std::string message, ExceptionDiagnostic cause)
    : Base(type, std::move(message), std::move(cause)) { this->template setErrorType<StarError>(); }
};

struct OutOfRangeExceptionTag {
  static constexpr std::string_view name() { return "OutOfRangeException"; }
};
using OutOfRangeException = StarError<OutOfRangeExceptionTag, StarException>;
struct IOExceptionTag {
  static constexpr std::string_view name() { return "IOException"; }
};
using IOException = StarError<IOExceptionTag, StarException>;
struct MemoryExceptionTag {
  static constexpr std::string_view name() { return "MemoryException"; }
};
using MemoryException = StarError<MemoryExceptionTag, StarException>;

template <typename... Args>
StarException StarException::format(std::format_string<Args...> fmt, Args const&... args) {
  return StarException(strfChecked(fmt, args...));
}


}
namespace Star {

struct FormatExceptionTag {
  static constexpr std::string_view name() { return "FormatException"; }
};
using FormatException = StarError<FormatExceptionTag, StarException>;

// Tags are UTF-8 byte sequences; values are copied verbatim, including NULs.
// A doubled '<' is literal. Bindings use the first matching name.
[[nodiscard]] inline std::expected<std::string, StarException> formatTags(
    std::string_view text,
    std::span<std::pair<std::string_view, std::string_view> const> tags) {
  std::string output;
  output.reserve(text.size());
  std::size_t position = 0;
  while (position < text.size()) {
    auto open = text.find('<', position);
    if (open == std::string_view::npos) {
      output.append(text.substr(position));
      break;
    }
    output.append(text.substr(position, open - position));
    if (open + 1 < text.size() && text[open + 1] == '<') {
      output.push_back('<');
      position = open + 2;
      continue;
    }
    auto close = text.find('>', open + 1);
    if (close == std::string_view::npos)
      return std::unexpected(FormatException("Unterminated template tag"));
    auto name = text.substr(open + 1, close - open - 1);
    if (name.empty())
      return std::unexpected(FormatException("Empty template tag"));
    auto binding = std::find_if(tags.begin(), tags.end(),
        [name](auto const& entry) { return entry.first == name; });
    if (binding == tags.end())
      return std::unexpected(FormatException::format("Unknown template tag '{}'", name));
    output.append(binding->second);
    position = close + 1;
  }
  return output;
}

namespace Detail {
  // Only callers holding a compile-time checked format string use this path.
  template <typename... T>
  std::string vformatChecked(std::string_view fmt, T const&... args) {
    return std::vformat(fmt, std::make_format_args(args...));
  }
}

// Consteval validation happens at the caller, before a literal is forwarded.
template <typename... T>
std::string strf(std::format_string<T...> fmt, T&&... args) {
  return Detail::vformatChecked(fmt.get(), args...);
}

// Keep the argument types used to check a wrapper's format string.
template <typename Char, typename... T>
std::string strfChecked(std::basic_format_string<Char, T...> fmt, T const&... args) {
  return Detail::vformatChecked(fmt.get(), args...);
}


template <typename... T>
void format(std::ostream& out, std::format_string<T...> fmt, T&&... args) {
  out << Detail::vformatChecked(fmt.get(), args...);
}


// Automatically flushes; use format to avoid flushing.
template <typename... T>
void coutf(std::format_string<T...> fmt, T&&... args) {
  std::cout << Detail::vformatChecked(fmt.get(), args...);
  std::cout.flush();
}


// Automatically flushes; use format to avoid flushing.
template <typename... T>
void cerrf(std::format_string<T...> fmt, T&&... args) {
  std::cerr << Detail::vformatChecked(fmt.get(), args...);
  std::cerr.flush();
}


template <class Type>
inline std::string toString(Type const& t) {
  return std::format("{}", t);
}

}

export module star.format;

export namespace Star {
using ::Star::DebugEnabled;
using ::Star::ExceptionDiagnostic;
using ::Star::StarException;
using ::Star::StarError;
using ::Star::OutOfRangeExceptionTag;
using ::Star::OutOfRangeException;
using ::Star::IOExceptionTag;
using ::Star::IOException;
using ::Star::MemoryExceptionTag;
using ::Star::MemoryException;
using ::Star::FormatExceptionTag;
using ::Star::FormatException;
using ::Star::printException;
using ::Star::outputException;
using ::Star::printStack;
using ::Star::fatalError;
using ::Star::starAssertFailed;
using ::Star::debugPrintStack;
using ::Star::strf;
using ::Star::strfChecked;
using ::Star::format;
using ::Star::coutf;
using ::Star::cerrf;
using ::Star::toString;
using ::Star::formatTags;
}
