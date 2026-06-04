#include "StarMiniDump.hpp"
#include <windows.h>
#include "minidumpapiset.h"

namespace Star {
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
      nullptr,
      nullptr);
    CloseHandle(hFile);
    if (dumpExceptionInfo.ExceptionPointers->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
      MessageBoxA(nullptr, "Stack overflow encountered\nA minidump has been generated", nullptr, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
    }
    return 0;
  };
}
