#pragma once

#ifdef STAR_SYSTEM_WINDOWS
#include <windows.h>
#endif

namespace Star {
#ifdef STAR_SYSTEM_WINDOWS
  [[nodiscard]] DWORD WINAPI writeMiniDump(void* ExceptionInfo);
#endif
}