module;
import std;

import star.output_proxy;

namespace Star {
#ifndef NDEBUG
inline constexpr bool DebugEnabled = true;
#else
inline constexpr bool DebugEnabled = false;
#endif


template <typename S>
struct IsFormatStringType : std::false_type {};
template <typename C, typename... A>
struct IsFormatStringType<std::basic_format_string<C, A...>> : std::true_type {};

template <typename S, typename = void>
struct HasUtf8Ptr : std::false_type {};
template <typename S>
struct HasUtf8Ptr<S, std::void_t<decltype(std::declval<S const&>().utf8Ptr())>> : std::true_type {};

// Runtime text excludes literals and already checked std::format_string values.
// Star::String participates through utf8Ptr(), without copying its UTF-8 bytes.
template <typename S>
concept RuntimeFormatString = !std::is_array_v<std::remove_reference_t<S>>
    && !IsFormatStringType<std::remove_cv_t<std::remove_reference_t<S>>>::value
    && (std::is_convertible_v<S const&, std::string_view>
        || HasUtf8Ptr<std::remove_reference_t<S>>::value);

template <typename... T>
std::string strf(std::format_string<T...> fmt, T&&... args);

// Declare the runtime overload before StarError: scalar arguments cannot use
// ADL to find an overload declared after the template definition.
template <typename S, typename... T>
  requires RuntimeFormatString<S>
std::string strf(S const& fmt, T&&... args);

template <typename Char, typename... T>
std::string strfChecked(std::basic_format_string<Char, T...> fmt, T const&... args);

class StarException;

// An owning diagnostic captured before an exception's static type is erased.
// exception_ptr is classified by the exception-handling machinery, not RTTI.
// A bare std::exception reference cannot identify an arbitrary dynamic object.
class ExceptionDiagnostic {
public:
  ExceptionDiagnostic(StarException const& exception);
  ExceptionDiagnostic(std::exception_ptr exception);

  friend void printException(std::ostream& os, ExceptionDiagnostic const& e, bool fullStacktrace);
  friend OutputProxy outputException(ExceptionDiagnostic const& e, bool fullStacktrace);

private:
  friend class StarException;
  std::function<void(std::ostream&, bool)> m_printException;
};

class StarException : public std::exception {
public:
  template <typename... Args>
  static StarException format(std::format_string<Args...> fmt, Args const&... args);
  template <typename S, typename... Args>
  requires RuntimeFormatString<S>
  static StarException format(S const& fmt, Args const&... args);

  StarException() noexcept;
  virtual ~StarException() noexcept;

  explicit StarException(std::string message, bool genStackTrace = true) noexcept;
  explicit StarException(ExceptionDiagnostic cause) noexcept;
  StarException(std::string message, ExceptionDiagnostic cause) noexcept;

  virtual char const* what() const noexcept override;

  friend class ExceptionDiagnostic;

protected:
  StarException(char const* type, std::string message, bool genStackTrace = true) noexcept;
  StarException(char const* type, std::string message, ExceptionDiagnostic cause) noexcept;

private:
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

void printStack(char const* message);

// Log error and stack-trace and possibly show a dialog box if available, then
// abort.
void fatalError(char const* message, bool showStackTrace);
void fatalException(ExceptionDiagnostic const& e, bool showStackTrace);

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
  Star::fatalError(message.c_str(), true);
#else
  (void)condition;
  (void)location;
#endif
}

inline void debugPrintStack(std::source_location location = std::source_location::current()) {
#ifndef NDEBUG
  // The message outlives the call: printStack may hand it on before returning.
  std::string const message = strf("Debug: file {} line {}", location.file_name(), location.line());
  Star::printStack(message.c_str());
#else
  (void)location;
#endif
}


// An exception class used to be generated by the STAR_EXCEPTION macro.  It is a
// template now, so that a module could export it.  The tag names one exception
// (with the alias below every one stays a distinct type, exactly as a generated
// class was) and supplies the type string that #ClassName provided.
template <typename Tag, typename Base>
class StarError : public Base {
public:
  template <typename... Args>
  static StarError format(std::format_string<Args...> fmt, Args const&... args) {
    return StarError(strfChecked(fmt, args...));
  }

  template <typename S, typename... Args>
    requires RuntimeFormatString<S>
  static StarError format(S const& fmt, Args const&... args) {
    return StarError(strf(fmt, args...));
  }

  StarError() : Base(Tag::name(), std::string(), true) {}
  explicit StarError(std::string message, bool genStackTrace = true)
    : Base(Tag::name(), std::move(message), genStackTrace) {}
  explicit StarError(ExceptionDiagnostic cause) : Base(Tag::name(), std::string(), std::move(cause)) {}
  StarError(std::string message, ExceptionDiagnostic cause) : Base(Tag::name(), std::move(message), std::move(cause)) {}

protected:
  StarError(char const* type, std::string message, bool genStackTrace = true)
    : Base(type, std::move(message), genStackTrace) {}
  StarError(char const* type, std::string message, ExceptionDiagnostic cause)
    : Base(type, std::move(message), std::move(cause)) {}
};

struct OutOfRangeExceptionTag {
  static constexpr char const* name() { return "OutOfRangeException"; }
};
using OutOfRangeException = StarError<OutOfRangeExceptionTag, StarException>;
struct IOExceptionTag {
  static constexpr char const* name() { return "IOException"; }
};
using IOException = StarError<IOExceptionTag, StarException>;
struct MemoryExceptionTag {
  static constexpr char const* name() { return "MemoryException"; }
};
using MemoryException = StarError<MemoryExceptionTag, StarException>;

template <typename... Args>
StarException StarException::format(std::format_string<Args...> fmt, Args const&... args) {
  return StarException(strfChecked(fmt, args...));
}

template <typename S, typename... Args>
  requires RuntimeFormatString<S>
StarException StarException::format(S const& fmt, Args const&... args) {
  return StarException(strf(fmt, args...));
}

}
namespace Star {

struct FormatExceptionTag {
  static constexpr char const* name() { return "FormatException"; }
};
using FormatException = StarError<FormatExceptionTag, StarException>;

namespace Detail {
  template <typename S>
  std::string_view formatStringView(S const& fmt) {
    if constexpr (HasUtf8Ptr<std::remove_reference_t<S>>::value)
      return fmt.utf8();
    else
      return fmt;
  }

  // Named arguments remain lvalues for make_format_args; the argument store
  // borrows them only for the synchronous vformat call.
  template <typename S, typename... T>
  std::string formatRuntime(S const& fmt, T&&... args) {
    try {
      return std::vformat(formatStringView(fmt), std::make_format_args(args...));
    } catch (std::exception const& e) {
      throw FormatException(std::format("Exception thrown during string format: {}", e.what()));
    }
  }

  template <typename... T>
  std::string vformatChecked(std::string_view fmt, T const&... args) {
    try {
      return std::vformat(fmt, std::make_format_args(args...));
    } catch (std::exception const& e) {
      throw FormatException(std::format("Exception thrown during string format: {}", e.what()));
    }
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

// Runtime strings exclude literals and already checked format strings.
template <typename S, typename... T>
  requires RuntimeFormatString<S>
std::string strf(S const& fmt, T&&... args) {
  return Detail::formatRuntime(fmt, std::forward<T>(args)...);
}

template <typename... T>
void format(std::ostream& out, std::format_string<T...> fmt, T&&... args) {
  out << Detail::vformatChecked(fmt.get(), args...);
}

template <typename S, typename... T>
  requires RuntimeFormatString<S>
void format(std::ostream& out, S const& fmt, T&&... args) {
  out << Detail::formatRuntime(fmt, args...);
}

// Automatically flushes; use format to avoid flushing.
template <typename... T>
void coutf(std::format_string<T...> fmt, T&&... args) {
  std::cout << Detail::vformatChecked(fmt.get(), args...);
  std::cout.flush();
}

template <typename S, typename... T>
  requires RuntimeFormatString<S>
void coutf(S const& fmt, T&&... args) {
  format(std::cout, fmt, args...);
  std::cout.flush();
}

// Automatically flushes; use format to avoid flushing.
template <typename... T>
void cerrf(std::format_string<T...> fmt, T&&... args) {
  std::cerr << Detail::vformatChecked(fmt.get(), args...);
  std::cerr.flush();
}

template <typename S, typename... T>
  requires RuntimeFormatString<S>
void cerrf(S const& fmt, T&&... args) {
  format(std::cerr, fmt, args...);
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
using ::Star::IsFormatStringType;
using ::Star::HasUtf8Ptr;
using ::Star::RuntimeFormatString;
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
using ::Star::fatalException;
using ::Star::starAssertFailed;
using ::Star::debugPrintStack;
using ::Star::strf;
using ::Star::strfChecked;
using ::Star::format;
using ::Star::coutf;
using ::Star::cerrf;
using ::Star::toString;
}
