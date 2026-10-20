#include <stdlib.h>
#ifdef STAR_USE_CPPTRACE
#include "cpptrace/cpptrace.hpp"
#include "cpptrace/formatting.hpp"
#else
#include <execinfo.h>
#endif
import star.format;
import star.maybe;
import star.logging;

import std;

import star.output_proxy;
import star.array;
namespace Star {

#ifdef STAR_USE_CPPTRACE

static inline std::string captureBacktrace() {
  auto formatter = cpptrace::formatter{}
    .paths(cpptrace::formatter::path_mode::basename);
  std::ostringstream out;
  formatter.print(out, cpptrace::generate_trace());
  return out.str();
}

#else

static std::size_t const StackLimit = 256;

typedef std::pair<Array<void*, StackLimit>, std::size_t> StackCapture;

inline StackCapture captureStack() {
  StackCapture stackCapture;
  stackCapture.second = backtrace(stackCapture.first.ptr(), StackLimit);
  return stackCapture;
}

OutputProxy outputStack(StackCapture stack) {
  return OutputProxy([stack = std::move(stack)](std::ostream & os) {
      char** symbols = backtrace_symbols(stack.first.ptr(), stack.second);
      for (std::size_t i = 0; i < stack.second; ++i) {
        os << symbols[i];
        if (i + 1 < stack.second)
          os << std::endl;
      }

      if (stack.second == StackLimit)
        os << std::endl << "[Stack Output Limit Reached]";

      std::free(symbols);
    });
}
  
#endif

StarException::StarException() noexcept
  : StarException(std::string("StarException")) {}

StarException::~StarException() noexcept {}

StarException::StarException(std::string message, bool genStackTrace) noexcept
  : StarException("StarException", std::move(message), genStackTrace) {}

StarException::StarException(ExceptionDiagnostic cause) noexcept
  : StarException("StarException", std::string(), std::move(cause)) {}

StarException::StarException(std::string message, ExceptionDiagnostic cause) noexcept
  : StarException("StarException", std::move(message), std::move(cause)) {}

const char* StarException::what() const throw() {
  if (m_whatBuffer.empty()) {
    std::ostringstream os;
    m_printException(os, false);
    m_whatBuffer = os.str();
  }
  return m_whatBuffer.c_str();
}

StarException::StarException(char const* type, std::string message, bool genStackTrace) noexcept {
#ifdef STAR_USE_CPPTRACE
  auto printException = [](std::ostream& os, bool fullStacktrace, char const* type, std::string message, std::string stack) {
#else
  auto printException = [](std::ostream& os, bool fullStacktrace, char const* type, std::string message, Maybe<StackCapture> stack) {
#endif
    os << "(" << type << ")";
    if (!message.empty())
      os << " " << message;

#ifdef STAR_USE_CPPTRACE
    if (fullStacktrace && !stack.empty()) {
      os << std::endl;
      os << stack;
    }
#else
    if (fullStacktrace && stack) {
      os << std::endl;
      os << outputStack(*stack);
    }
#endif
  };
#ifdef STAR_USE_CPPTRACE
  m_printException = std::bind(printException, std::placeholders::_1, std::placeholders::_2, type, std::move(message), genStackTrace ? captureBacktrace() : std::string());
#else
  m_printException = std::bind(printException, std::placeholders::_1, std::placeholders::_2, type, std::move(message), genStackTrace ? captureStack() : Maybe<StackCapture>());
#endif
}

StarException::StarException(char const* type, std::string message, ExceptionDiagnostic cause) noexcept
  : StarException(type, std::move(message), true) {
  auto printException = [](std::ostream& os, bool fullStacktrace, std::function<void(std::ostream&, bool)> self, std::function<void(std::ostream&, bool)> cause) {
    self(os, fullStacktrace);
    os << std::endl << "Caused by: ";
    cause(os, fullStacktrace);
  };

  m_printException = std::bind(printException, std::placeholders::_1, std::placeholders::_2, m_printException, std::move(cause.m_printException));
}

ExceptionDiagnostic::ExceptionDiagnostic(StarException const& exception)
  : m_printException(exception.m_printException) {}

ExceptionDiagnostic::ExceptionDiagnostic(std::exception_ptr exception) {
  if (!exception)
    throw std::invalid_argument("ExceptionDiagnostic requires an exception");
  try {
    std::rethrow_exception(exception);
  } catch (StarException const& e) {
    m_printException = e.m_printException;
  } catch (std::exception const& e) {
    m_printException = [what = std::string(e.what())](std::ostream& os, bool) {
      os << "std::exception: " << what;
    };
  } catch (...) {
    m_printException = [](std::ostream& os, bool) { os << "Unknown exception"; };
  }
}

std::string printException(ExceptionDiagnostic const& e, bool fullStacktrace) {
  std::ostringstream os;
  printException(os, e, fullStacktrace);
  return os.str();
}

void printException(std::ostream& os, ExceptionDiagnostic const& e, bool fullStacktrace) {
  e.m_printException(os, fullStacktrace);
}

OutputProxy outputException(ExceptionDiagnostic const& e, bool fullStacktrace) {
  return OutputProxy(std::bind(e.m_printException, std::placeholders::_1, fullStacktrace));
}

void printStack(char const* message) {
#ifdef STAR_USE_CPPTRACE
  EmergencyLog::info("Stack Trace ({})...\n{}", message, captureBacktrace());
#else
  EmergencyLog::info("Stack Trace ({})...\n{}", message, outputStack(captureStack()));
#endif
}

void fatalError(char const* message, bool showStackTrace) {
  if (showStackTrace)
#ifdef STAR_USE_CPPTRACE
    EmergencyLog::error("Fatal Error: {}\n{}", message, captureBacktrace());
#else
    EmergencyLog::error("Fatal Error: {}\n{}", message, outputStack(captureStack()));
#endif
  else
    EmergencyLog::error("Fatal Error: {}", message);

  std::abort();
}

void fatalException(ExceptionDiagnostic const& e, bool showStackTrace) {
  if (showStackTrace)
#ifdef STAR_USE_CPPTRACE
    EmergencyLog::error("Fatal Exception caught: {}\nCaught at:\n{}", outputException(e, true), captureBacktrace());
#else
    EmergencyLog::error("Fatal Exception caught: {}\nCaught at:\n{}", outputException(e, true), outputStack(captureStack()));
#endif
  else
    EmergencyLog::error("Fatal Exception caught: {}", outputException(e, showStackTrace));

  std::abort();
}

}
