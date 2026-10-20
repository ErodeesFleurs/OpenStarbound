module;
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
#include <windows.h>
#include <dbghelp.h>
#else
#include <signal.h>
#include <unistd.h>
#endif
import std;
import star.format;

#ifdef STAR_SYSTEM_FAMILY_WINDOWS
import star.string;
#endif

export module star.signal_handler;

export namespace Star {

struct SignalHandlerImpl;

// Process-owned registration for fatal signals and non-fatal interrupts.
// Only the native callbacks' minimal bridge state is process-wide.
class SignalHandler {
public:
  SignalHandler();
  ~SignalHandler();

  SignalHandler(SignalHandler const&) = delete;
  SignalHandler& operator=(SignalHandler const&) = delete;

  // If enabled, will catch segfault, fpe, and illegal instructions and output
  // error information before dying.
  void setHandleFatal(bool handleFatal);
  bool handlingFatal() const;

  // If enabled, non-fatal interrupt signal will be caught and will not kill
  // the process and will instead set the interrupted flag.
  void setHandleInterrupt(bool handleInterrupt);
  bool handlingInterrupt() const;

  bool interruptCaught() const;

private:
  std::unique_ptr<SignalHandlerImpl> m_impl;
};

}

namespace Star {

#ifdef STAR_SYSTEM_FAMILY_WINDOWS

DWORD WINAPI writeMiniDump(void* ExceptionInfo) {
  auto hFile = CreateFileA("starbound.dmp", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if (hFile == INVALID_HANDLE_VALUE)
    return 0;
  MINIDUMP_EXCEPTION_INFORMATION dumpExceptionInfo{};
  dumpExceptionInfo.ThreadId = GetCurrentThreadId();
  dumpExceptionInfo.ExceptionPointers = (PEXCEPTION_POINTERS)ExceptionInfo;
  dumpExceptionInfo.ClientPointers = FALSE;
  MiniDumpWriteDump(
    GetCurrentProcess(),
    GetCurrentProcessId(),
    hFile,
    MiniDumpNormal,
    &dumpExceptionInfo,
    NULL,
    NULL);
  CloseHandle(hFile);
  if (dumpExceptionInfo.ExceptionPointers->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
    MessageBoxA(NULL, "Stack overflow encountered\nA minidump has been generated", NULL, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
  }
  return 0;
}

// Native callbacks and the resumed trampoline may outlive registration removal.
// Retain only the first fatal exception's native data for the process lifetime;
// no callback borrows the SignalHandler, its Impl, or application services.
struct FatalSignalBridge {
  std::atomic_flag claimed{};
  EXCEPTION_RECORD exceptionRecord{};
  char const* message = nullptr;
};
static_assert(std::is_trivially_destructible_v<FatalSignalBridge>);
static constinit FatalSignalBridge fatalSignalBridge;
static std::atomic<bool> interruptSignalPending{false};
static_assert(std::atomic<bool>::is_always_lock_free);

struct SignalHandlerImpl {
  bool handlingFatal = false;
  bool handlingInterrupt = false;
  bool interrupted = false;
  // All callback-accessed state has process lifetime, not this object's lifetime.

  PVOID handler = nullptr;

  ~SignalHandlerImpl() {
    setHandleFatal(false);
    setHandleInterrupt(false);
  }

  void setHandleFatal(bool b) {
    if (handlingFatal == b)
      return;

    if (b) {
      handler = AddVectoredExceptionHandler(1, vectoredExceptionHandler);
      if (!handler) {
        throw StarException("Could not register fatal exception handler");
      }
      handlingFatal = true;
    } else {
      RemoveVectoredExceptionHandler(handler);
      handler = nullptr;
      handlingFatal = false;
    }
  }

  void setHandleInterrupt(bool b) {
    if (handlingInterrupt == b)
      return;
    if (b) {
      interruptSignalPending.store(false);
      if (!SetConsoleCtrlHandler(consoleCtrlHandler, true))
        throw StarException("Could not register console interrupt handler");
      handlingInterrupt = true;
    } else {
      SetConsoleCtrlHandler(consoleCtrlHandler, false);
      interrupted = interrupted || interruptSignalPending.load();
      handlingInterrupt = false;
    }
  }

  static void sehTrampoline() {
    auto const& exception = fatalSignalBridge.exceptionRecord;
    String message;
    if (exception.ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
      String mode;
      auto modeFlag = exception.ExceptionInformation[0];
      if (modeFlag == 0)
        mode = "Read";
      else if (modeFlag == 1)
        mode = "Write";
      else if (modeFlag == 8)
        mode = "Execute";
      else
        mode = strf("Mode({})", modeFlag);
      message = strf("Access violation detected at {} ({} of address {})",
          exception.ExceptionAddress,
          mode,
          (PVOID)exception.ExceptionInformation[1]);
    } else {
      message = strf("{} (0x{:08X} @ {})",
          fatalSignalBridge.message,
          exception.ExceptionCode,
          exception.ExceptionAddress);
      for (DWORD i = 0; i < exception.NumberParameters; i++)
        message = strf("{} [{}]", message, (PVOID)exception.ExceptionInformation[i]);
    }
    fatalError(message.utf8Ptr(), true);
    TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
  }

  static LONG handleFatalError(char const* message, PEXCEPTION_POINTERS ExceptionInfo) {
    // Never replace a snapshot already consumed by a resumed trampoline. A
    // concurrent or recursive fault continues through native exception dispatch.
    if (fatalSignalBridge.claimed.test_and_set(std::memory_order_relaxed))
      return EXCEPTION_CONTINUE_SEARCH;
    fatalSignalBridge.exceptionRecord = *ExceptionInfo->ExceptionRecord;
    fatalSignalBridge.exceptionRecord.ExceptionRecord = nullptr;
    fatalSignalBridge.message = message;
    writeMiniDump(ExceptionInfo);

// setup a hijack into our own trampoline as if the failure actually was a
// function call
#ifdef STAR_ARCHITECTURE_X86_64
    DWORD64 rsp = ExceptionInfo->ContextRecord->Rsp - 8;
    DWORD64 rip = ExceptionInfo->ContextRecord->Rip; // an offset avoid the issue of gdb thinking
    // the error is one statement too early, but
    // the offset is instruction dependent, and we
    // don't know its size + 1;
    *((DWORD64*)rsp) = rip;
    ExceptionInfo->ContextRecord->Rsp = rsp;
    ExceptionInfo->ContextRecord->Rip = (DWORD64)&sehTrampoline;
#else
    DWORD esp = ExceptionInfo->ContextRecord->Esp - 4;
    DWORD eip = ExceptionInfo->ContextRecord->Eip; // an offset avoid the issue of gdb thinking the
    // error is one statement too early, but the
    // offset is instruction dependent, and we don't
    // know its size + 1;
    *((DWORD*)esp) = eip;
    ExceptionInfo->ContextRecord->Esp = esp;
    ExceptionInfo->ContextRecord->Eip = (DWORD)&sehTrampoline;
#endif
    return EXCEPTION_CONTINUE_EXECUTION;
  }

  static LONG CALLBACK vectoredExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo) {
    LONG result = EXCEPTION_CONTINUE_SEARCH;

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
      return handleFatalError("Stack overflow detected", ExceptionInfo);
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
      return handleFatalError("Access violation detected", ExceptionInfo);
    if ((ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION)) {
      return handleFatalError("Illegal instruction encountered", ExceptionInfo);
    }

    if ((ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_DENORMAL_OPERAND)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_DIVIDE_BY_ZERO)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_INEXACT_RESULT)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_INVALID_OPERATION)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_OVERFLOW)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_STACK_CHECK)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_UNDERFLOW)
            ) {
      return handleFatalError("Floating point exception", ExceptionInfo);
    }

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO) {
      return handleFatalError("Division by zero", ExceptionInfo);
    }

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INT_OVERFLOW) {
      return handleFatalError("Integer overflow", ExceptionInfo);
    }

    if ((ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_DATATYPE_MISALIGNMENT)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_IN_PAGE_ERROR)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_NONCONTINUABLE_EXCEPTION)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INVALID_DISPOSITION)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INVALID_HANDLE)) {
      return handleFatalError("Error occurred", ExceptionInfo);
    }

    return result;
  }

  static BOOL WINAPI consoleCtrlHandler(DWORD) {
    interruptSignalPending.store(true);
    return true;
  }
};

#else

// The POSIX callback may only communicate through async-signal-safe state.
static volatile sig_atomic_t interruptSignalPending = 0;

struct SignalHandlerImpl {
  bool handlingFatal = false;
  bool handlingInterrupt = false;
  bool interrupted = false;
  struct sigaction previousFatal[4]{};
  struct sigaction previousInterrupt{};

  static constexpr int FatalSignals[] = {SIGSEGV, SIGILL, SIGFPE, SIGBUS};

  ~SignalHandlerImpl() {
    setHandleFatal(false);
    setHandleInterrupt(false);
  }

  void setHandleFatal(bool b) {
    if (handlingFatal == b)
      return;
    if (b) {
      struct sigaction action{};
      action.sa_handler = handleFatal;
      sigemptyset(&action.sa_mask);
      unsigned installed = 0;
      for (int signal : FatalSignals) {
        if (sigaction(signal, &action, &previousFatal[installed]) != 0) {
          for (unsigned i = 0; i < installed; ++i)
            sigaction(FatalSignals[i], &previousFatal[i], nullptr);
          throw StarException("Could not register fatal signal handler");
        }
        ++installed;
      }
    } else {
      for (unsigned i = 0; i < std::size(FatalSignals); ++i)
        sigaction(FatalSignals[i], &previousFatal[i], nullptr);
    }
    handlingFatal = b;
  }

  void setHandleInterrupt(bool b) {
    if (handlingInterrupt == b)
      return;
    if (b) {
      struct sigaction action{};
      action.sa_handler = handleInterrupt;
      sigemptyset(&action.sa_mask);
      interruptSignalPending = 0;
      if (sigaction(SIGINT, &action, &previousInterrupt) != 0)
        throw StarException("Could not register interrupt signal handler");
    } else {
      sigaction(SIGINT, &previousInterrupt, nullptr);
      interrupted = interrupted || interruptSignalPending != 0;
    }
    handlingInterrupt = b;
  }

  static void handleFatal(int signum) {
    // Logging, formatting and stack unwinding are not async-signal-safe.
    char const* message;
    std::size_t length;
    if (signum == SIGSEGV) {
      message = "Segfault Encountered!\n";
      length = sizeof("Segfault Encountered!\n") - 1;
    } else if (signum == SIGILL) {
      message = "Illegal Instruction Encountered!\n";
      length = sizeof("Illegal Instruction Encountered!\n") - 1;
    } else if (signum == SIGFPE) {
      message = "Floating Point Exception Encountered!\n";
      length = sizeof("Floating Point Exception Encountered!\n") - 1;
    } else {
      message = "Bus Error Encountered!\n";
      length = sizeof("Bus Error Encountered!\n") - 1;
    }
    (void)::write(STDERR_FILENO, message, length);
    std::_Exit(128 + signum);
  }

  static void handleInterrupt(int) {
    interruptSignalPending = 1;
  }
};

#endif

SignalHandler::SignalHandler() : m_impl(std::make_unique<SignalHandlerImpl>()) {}

SignalHandler::~SignalHandler() = default;

void SignalHandler::setHandleFatal(bool handleFatal) {
  m_impl->setHandleFatal(handleFatal);
}

bool SignalHandler::handlingFatal() const {
  return m_impl->handlingFatal;
}

void SignalHandler::setHandleInterrupt(bool handleInterrupt) {
  m_impl->setHandleInterrupt(handleInterrupt);
}

bool SignalHandler::handlingInterrupt() const {
  return m_impl->handlingInterrupt;
}

bool SignalHandler::interruptCaught() const {
  return m_impl->interrupted || (m_impl->handlingInterrupt && interruptSignalPending);
}

}
