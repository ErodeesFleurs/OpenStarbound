#include "StarSignalHandler.hpp"
#include "StarException.hpp"

#ifdef STAR_SYSTEM_WINDOWS
#include "StarFormat.hpp"
#include "StarMiniDump.hpp"
#include "StarString.hpp"
#include <windows.h>
#else
#include <signal.h>
#endif

import std;

namespace Star {

#ifdef STAR_SYSTEM_WINDOWS
String g_sehMessage;
#endif

struct SignalHandlerImpl {
  bool handlingFatal = false;
  bool handlingInterrupt = false;
  bool interrupted = false;

#ifdef STAR_SYSTEM_WINDOWS
  PVOID handler = nullptr;
#endif

  SignalHandlerImpl() = default;

  ~SignalHandlerImpl() {
    setHandleFatal(false);
    setHandleInterrupt(false);
  }

  void setHandleFatal(bool b) {
    handlingFatal = b;
#ifdef STAR_SYSTEM_WINDOWS
    if (handler) {
      RemoveVectoredExceptionHandler(handler);
      handler = nullptr;
    }

    if (handlingFatal)
      handler = AddVectoredExceptionHandler(1, vectoredExceptionHandler);
#else
    if (handlingFatal) {
      signal(SIGSEGV, handleFatal);
      signal(SIGILL, handleFatal);
      signal(SIGFPE, handleFatal);
      signal(SIGBUS, handleFatal);
    } else {
      signal(SIGSEGV, SIG_DFL);
      signal(SIGILL, SIG_DFL);
      signal(SIGFPE, SIG_DFL);
      signal(SIGBUS, SIG_DFL);
    }
#endif
  }

  void setHandleInterrupt(bool b) {
    handlingInterrupt = b;
#ifdef STAR_SYSTEM_WINDOWS
    SetConsoleCtrlHandler(nullptr, false);

    if (handlingInterrupt)
      SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleCtrlHandler, true);
#else
    if (handlingInterrupt)
      signal(SIGINT, handleInterrupt);
    else
      signal(SIGINT, SIG_DFL);
#endif
  }

#ifdef STAR_SYSTEM_WINDOWS
  static void sehTrampoline() {
    fatalError(g_sehMessage.utf8Ptr(), true);
  }

  static void handleFatalError(String const& msg, PEXCEPTION_POINTERS ExceptionInfo) {
    static bool dumpWritten = false;
    if (!dumpWritten) {
      dumpWritten = true;
      writeMiniDump(ExceptionInfo);
    }
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
      String mode;
      DWORD modeFlag = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
      if (modeFlag == 0)
        mode = "Read";
      else if (modeFlag == 1)
        mode = "Write";
      else if (modeFlag == 8)
        mode = "Execute";
      else
        mode = strf("Mode({})", modeFlag);
      g_sehMessage = strf("Access violation detected at {} ({} of address {})",
                          ExceptionInfo->ExceptionRecord->ExceptionAddress,
                          mode,
                          (PVOID)ExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
    } else {
      g_sehMessage = msg;
      g_sehMessage = strf("{} ({} @ {})",
                          g_sehMessage,
                          (PVOID)ExceptionInfo->ExceptionRecord->ExceptionCode,
                          ExceptionInfo->ExceptionRecord->ExceptionAddress);
      for (DWORD i = 0; i < ExceptionInfo->ExceptionRecord->NumberParameters; i++)
        g_sehMessage = strf("{} [{}]", g_sehMessage, (PVOID)ExceptionInfo->ExceptionRecord->ExceptionInformation[i]);
    }

#ifdef STAR_ARCHITECTURE_X86_64
    DWORD64 rsp = ExceptionInfo->ContextRecord->Rsp - 8;
    DWORD64 rip = ExceptionInfo->ContextRecord->Rip;
    *((DWORD64*)rsp) = rip;
    ExceptionInfo->ContextRecord->Rsp = rsp;
    ExceptionInfo->ContextRecord->Rip = (DWORD64)&sehTrampoline;
#else
    DWORD esp = ExceptionInfo->ContextRecord->Esp - 4;
    DWORD eip = ExceptionInfo->ContextRecord->Eip;
    *((DWORD*)esp) = eip;
    ExceptionInfo->ContextRecord->Esp = esp;
    ExceptionInfo->ContextRecord->Eip = (DWORD)&sehTrampoline;
#endif
  }

  static LONG CALLBACK vectoredExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo) {
    LONG result = EXCEPTION_CONTINUE_SEARCH;

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
      handleFatalError("Stack overflow detected", ExceptionInfo);
      result = EXCEPTION_CONTINUE_EXECUTION;
    }
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
      handleFatalError("Access violation detected", ExceptionInfo);
      result = EXCEPTION_CONTINUE_EXECUTION;
    }
    if ((ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION)) {
      handleFatalError("Illegal instruction encountered", ExceptionInfo);
      result = EXCEPTION_CONTINUE_EXECUTION;
    }

    if ((ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_DENORMAL_OPERAND)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_DIVIDE_BY_ZERO)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_INEXACT_RESULT)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_INVALID_OPERATION)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_OVERFLOW)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_STACK_CHECK)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_FLT_UNDERFLOW)) {
      handleFatalError("Floating point exception", ExceptionInfo);
      result = EXCEPTION_CONTINUE_EXECUTION;
    }

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO) {
      handleFatalError("Division by zero", ExceptionInfo);
      result = EXCEPTION_CONTINUE_EXECUTION;
    }

    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INT_OVERFLOW) {
      handleFatalError("Integer overflow", ExceptionInfo);
      result = EXCEPTION_CONTINUE_EXECUTION;
    }

    if ((ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_DATATYPE_MISALIGNMENT)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_IN_PAGE_ERROR)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_NONCONTINUABLE_EXCEPTION)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INVALID_DISPOSITION)
        || (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INVALID_HANDLE)) {
      handleFatalError("Error occurred", ExceptionInfo);
      result = EXCEPTION_CONTINUE_EXECUTION;
    }

    return result;
  }

  static BOOL WINAPI consoleCtrlHandler(DWORD) {
    if (SignalHandler::s_singleton)
      SignalHandler::s_singleton->interrupted = true;
    return TRUE;
  }
#else
  static void handleFatal(int signum) {
    if (signum == SIGSEGV)
      fatalError("Segfault Encountered!", true);
    else if (signum == SIGILL)
      fatalError("Illegal Instruction Encountered!", true);
    else if (signum == SIGFPE)
      fatalError("Floating Point Exception Encountered!", true);
    else if (signum == SIGBUS)
      fatalError("Bus Error Encountered!", true);
  }

  static void handleInterrupt(int) {
    if (SignalHandler::s_singleton)
      SignalHandler::s_singleton->interrupted = true;
  }
#endif
};

UPtr<SignalHandlerImpl> SignalHandler::s_singleton;

SignalHandler::SignalHandler() {
  if (s_singleton)
    throw StarException("Singleton SignalHandler has been constructed twice!");

  s_singleton = std::make_unique<SignalHandlerImpl>();
}

SignalHandler::~SignalHandler() {
  s_singleton.reset();
}

void SignalHandler::setHandleFatal(bool handleFatal) {
  s_singleton->setHandleFatal(handleFatal);
}

auto SignalHandler::handlingFatal() const -> bool {
  return s_singleton->handlingFatal;
}

void SignalHandler::setHandleInterrupt(bool handleInterrupt) {
  s_singleton->setHandleInterrupt(handleInterrupt);
}

auto SignalHandler::handlingInterrupt() const -> bool {
  return s_singleton->handlingInterrupt;
}

auto SignalHandler::interruptCaught() const -> bool {
  return s_singleton->interrupted;
}

}// namespace Star
