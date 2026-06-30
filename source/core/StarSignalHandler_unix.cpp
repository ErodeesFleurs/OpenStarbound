#include "StarSignalHandler.hpp"

#include <signal.h>

namespace Star {

struct SignalHandlerImpl {
  bool handlingFatal{};
  bool handlingInterrupt{};
  bool interrupted{};

  SignalHandlerImpl() = default;

  ~SignalHandlerImpl() {
    setHandleFatal(false);
    setHandleInterrupt(false);
  }

  void setHandleFatal(bool b) {
    handlingFatal = b;
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
  }

  void setHandleInterrupt(bool b) {
    handlingInterrupt = b;
    if (handlingInterrupt)
      signal(SIGINT, handleInterrupt);
    else
      signal(SIGINT, SIG_DFL);
  }

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
    if (SignalHandler::s_activeHandler)
      SignalHandler::s_activeHandler->interrupted = true;
  }
};

UniquePtr<SignalHandlerImpl> SignalHandler::s_activeHandler;

SignalHandler::SignalHandler() {
  if (s_activeHandler)
    throw StarException("SignalHandler has been constructed twice!");

  s_activeHandler = make_unique<SignalHandlerImpl>();
}

SignalHandler::~SignalHandler() {
  s_activeHandler.reset();
}

void SignalHandler::setHandleFatal(bool handleFatal) {
  s_activeHandler->setHandleFatal(handleFatal);
}

[[nodiscard]] bool SignalHandler::handlingFatal() const {
  return s_activeHandler->handlingFatal;
}

void SignalHandler::setHandleInterrupt(bool handleInterrupt) {
  s_activeHandler->setHandleInterrupt(handleInterrupt);
}

[[nodiscard]] bool SignalHandler::handlingInterrupt() const {
  return s_activeHandler->handlingInterrupt;
}

[[nodiscard]] bool SignalHandler::interruptCaught() const {
  return s_activeHandler->interrupted;
}

}
