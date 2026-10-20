#include <winsock2.h>
#include <windows.h>
#include <dbghelp.h>

import std;
import star.format;
import star.maybe;
import star.output_proxy;
import star.logging;
import star.string_windows;
import star.version;
import star.native_runtime;


import star.array;
import star.math_common;
namespace Star {

// DbgHelp is process-native and not thread-safe. Its lock must remain usable
// through entry-point teardown without a dynamically initialized global owner.
struct DbgHelpLock {
  void lock() {
    AcquireSRWLockExclusive(&nativeLock);
  }

  void unlock() {
    ReleaseSRWLockExclusive(&nativeLock);
  }

  SRWLOCK nativeLock = SRWLOCK_INIT;
};
static_assert(std::is_trivially_destructible_v<DbgHelpLock>);
static constinit DbgHelpLock g_dbgHelpLock;
// Protected by g_dbgHelpLock; overlapping explicit owners share one session.
static constinit std::size_t g_diagnosticsOwners = 0;

NativeRuntime::NativeRuntime() {
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    fatalError("WSAStartup failed", false);

  std::lock_guard lock(g_dbgHelpLock);
  if (g_diagnosticsOwners == 0) {
    DWORD options = SymGetOptions();
    SymSetOptions(options | SYMOPT_LOAD_LINES);
    if (!SymInitialize(GetCurrentProcess(), NULL, TRUE))
      fatalError("SymInitialize failed", false);
  }
  ++g_diagnosticsOwners;
}

NativeRuntime::~NativeRuntime() {
  {
    std::lock_guard lock(g_dbgHelpLock);
    if (--g_diagnosticsOwners == 0)
      SymCleanup(GetCurrentProcess());
  }
  WSACleanup();
}

static std::size_t const StackLimit = 256;

typedef std::pair<Array<DWORD64, StackLimit>, std::size_t> StackCapture;

inline StackCapture captureStack() {
  HANDLE process = GetCurrentProcess();
  HANDLE thread = GetCurrentThread();

  CONTEXT context;
  DWORD image;
  STACKFRAME64 stackFrame;

  std::memset(&context, 0, sizeof(CONTEXT));
  context.ContextFlags = CONTEXT_FULL;

  ZeroMemory(&stackFrame, sizeof(STACKFRAME64));
  stackFrame.AddrPC.Mode = AddrModeFlat;
  stackFrame.AddrReturn.Mode = AddrModeFlat;
  stackFrame.AddrFrame.Mode = AddrModeFlat;
  stackFrame.AddrStack.Mode = AddrModeFlat;

#ifdef STAR_ARCHITECTURE_I386

  DWORD eip_val = 0;
  DWORD esp_val = 0;
  DWORD ebp_val = 0;

  __asm__ __volatile__("call 1f\n1: pop %0" : "=g"(eip_val));
  __asm__ __volatile__("movl %%esp, %0" : "=g"(esp_val));
  __asm__ __volatile__("movl %%ebp, %0" : "=g"(ebp_val));

  context.Eip = eip_val;
  context.Esp = esp_val;
  context.Ebp = ebp_val;

  image = IMAGE_FILE_MACHINE_I386;

  stackFrame.AddrPC.Offset = context.Eip;
  stackFrame.AddrReturn.Offset = context.Eip;
  stackFrame.AddrFrame.Offset = context.Ebp;
  stackFrame.AddrStack.Offset = context.Esp;

#elif defined STAR_ARCHITECTURE_X86_64

  RtlCaptureContext(&context);

  image = IMAGE_FILE_MACHINE_AMD64;

  stackFrame.AddrPC.Offset = context.Rip;
  stackFrame.AddrReturn.Offset = context.Rip;
  stackFrame.AddrFrame.Offset = context.Rbp;
  stackFrame.AddrStack.Offset = context.Rsp;

#endif

  std::lock_guard lock(g_dbgHelpLock);

  Array<DWORD64, StackLimit> addresses;
  std::size_t count = 0;
  for (std::size_t i = 0; i < StackLimit; i++) {
    if (!StackWalk64(image, process, thread, &stackFrame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
      break;
    if (stackFrame.AddrPC.Offset == 0)
      break;
    addresses[i] = stackFrame.AddrPC.Offset;
    ++count;
  }

  return {addresses, count};
}

OutputProxy outputStack(StackCapture stack) {
  return OutputProxy([stack = std::move(stack)](std::ostream & os) {
    HANDLE process = GetCurrentProcess();
    std::lock_guard lock(g_dbgHelpLock);
    for (std::size_t i = 0; i < stack.second; ++i) {
      char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
      PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
      symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
      symbol->MaxNameLen = MAX_SYM_NAME;

      format(os, "[{:0{}}] {}", i, (int)std::log10(stack.second) + 1, (void*)stack.first[i]);
      IMAGEHLP_LINE64 line{};
      DWORD displacement = 0;
      line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
      if (SymGetLineFromAddr64(process, stack.first[i], &displacement, &line) && *line.FileName) {
        char* file = line.FileName;
        for (char* i = file; *i; ++i) {
          if (*i == '\\' || *i == '/')
            file = i;
        }
        format(os, " ({}:{})", ++file, line.LineNumber);
      }
      if (SymFromAddr(process, stack.first[i], NULL, symbol))
        format(os, " {}", std::string_view(symbol->Name, symbol->NameLen));

      if (i + 1 < stack.second)
        os << std::endl;
    }

    if (stack.second == StackLimit)
      os << std::endl << "[Stack Output Limit Reached]";

  });
}

StarException::StarException() noexcept : StarException(std::string("StarException")) {}

StarException::~StarException() noexcept {}

StarException::StarException(std::string message, bool genStackTrace) noexcept : StarException("StarException", std::move(message), genStackTrace) {}

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
  auto printException = [](
      std::ostream& os, bool fullStacktrace, char const* type, std::string message, Maybe<StackCapture> stack) {
    os << "(" << type << ")";
    if (!message.empty())
      os << " " << message;

    if (fullStacktrace && stack) {
      os << std::endl;
      os << outputStack(*stack);
    }
  };

  m_printException = std::bind(printException, std::placeholders::_1, std::placeholders::_2, type, std::move(message), genStackTrace ? captureStack() : Maybe<StackCapture>());
}

StarException::StarException(char const* type, std::string message, ExceptionDiagnostic cause) noexcept
    : StarException(type, std::move(message), true) {
  auto printException = [](std::ostream& os,
      bool fullStacktrace,
      std::function<void(std::ostream&, bool)> self,
      std::function<void(std::ostream&, bool)> cause) {
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
  EmergencyLog::info("Stack Trace ({})...\n{}", message, outputStack(captureStack()));
}

void fatalError(char const* message, bool showStackTrace) {
  std::ostringstream ss;
  ss << "v" << OpenStarVersionString << " (" << StarSourceIdentifierString << ")\n"
     << "Fatal Error: " << message << std::endl;
  if (showStackTrace)
    ss << outputStack(captureStack());

  EmergencyLog::log(LogLevel::Error, ss.str().c_str());
  MessageBoxW(NULL, stringToUtf16(ss.str()).get(), stringToUtf16("Error").get(), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

  std::abort();
}

void fatalException(ExceptionDiagnostic const& e, bool showStackTrace) {
  std::ostringstream ss;
  ss << "v" << OpenStarVersionString << " (" << StarSourceIdentifierString << ")\n"
     << "Fatal Exception: " << outputException(e, showStackTrace) << std::endl;
  if (showStackTrace)
    ss << "Caught at:" << std::endl << outputStack(captureStack());

  EmergencyLog::log(LogLevel::Error, ss.str().c_str());
  MessageBoxW(NULL, stringToUtf16(ss.str()).get(), stringToUtf16("Error").get(), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

  std::abort();
}

}
