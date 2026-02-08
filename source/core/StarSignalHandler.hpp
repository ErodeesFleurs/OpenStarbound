#pragma once

#include "StarConfig.hpp"

namespace Star {

struct SignalHandlerImpl;

// Singleton signal handler that registers handlers for segfault, fpe,
// illegal instructions etc as well as non-fatal interrupts.
class SignalHandler {
public:
  SignalHandler();
  ~SignalHandler();

  // If enabled, will catch segfault, fpe, and illegal instructions and output
  // error information before dying.
  void setHandleFatal(bool handleFatal);
  [[nodiscard]] auto handlingFatal() const -> bool;

  // If enabled, non-fatal interrupt signal will be caught and will not kill
  // the process and will instead set the interrupted flag.
  void setHandleInterrupt(bool handleInterrupt);
  [[nodiscard]] auto handlingInterrupt() const -> bool;

  [[nodiscard]] auto interruptCaught() const -> bool;

private:
  friend SignalHandlerImpl;

  static UPtr<SignalHandlerImpl> s_singleton;
};

}
