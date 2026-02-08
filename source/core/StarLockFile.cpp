#include "StarLockFile.hpp"
#include "StarFormat.hpp"
#include "StarThread.hpp"
#include "StarTime.hpp"

#ifdef STAR_SYSTEM_WINDOWS
#include "StarString_windows.hpp"
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

import std;

namespace Star {

auto LockFile::acquireLock(String const& filename, std::int64_t lockTimeout) -> std::optional<LockFile> {
  LockFile lock(filename);
  if (lock.lock(lockTimeout))
    return std::move(lock);
  return {};
}

LockFile::LockFile(String const& filename) : m_filename(std::move(filename)) {}

LockFile::LockFile(LockFile&& lockFile) noexcept {
  operator=(std::move(lockFile));
}

LockFile::~LockFile() {
  unlock();
}

auto LockFile::operator=(LockFile&& lockFile) noexcept -> LockFile& {
  if (this != &lockFile) {
    unlock();
    m_filename = std::move(lockFile.m_filename);
    m_handle = std::move(lockFile.m_handle);
  }
  return *this;
}

auto LockFile::lock(std::int64_t timeout) -> bool {
  auto attemptLock = [this]() -> std::shared_ptr<void> {
#ifdef STAR_SYSTEM_WINDOWS
    HANDLE handle = CreateFileW(
      stringToUtf16(m_filename).get(),
      GENERIC_READ,
      0,// No sharing
      nullptr,
      OPEN_ALWAYS,
      FILE_FLAG_DELETE_ON_CLOSE,
      nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
      if (GetLastError() == ERROR_SHARING_VIOLATION)
        return {};
      throw StarException(strf("Could not open lock file {}, error code {}\n", m_filename, GetLastError()));
    }
    return std::shared_ptr<void>(handle, [](void* h) { CloseHandle((HANDLE)h); });
#else
    int fd = ::open(m_filename.utf8Ptr(), O_RDONLY | O_CREAT, 0644);
    if (fd < 0)
      throw StarException(strf("Could not open lock file {}, {}\n", m_filename, std::strerror(errno)));

    if (::flock(fd, LOCK_EX | LOCK_NB) != 0) {
      ::close(fd);
      if (errno != EWOULDBLOCK)
        throw StarException(strf("Could not lock file {}, {}\n", m_filename, std::strerror(errno)));
      return {};
    }
    return {(void*)(intptr_t)fd, [fn = m_filename](void* h) {
              int f = (int)(intptr_t)h;
              ::unlink(fn.utf8Ptr());
              ::close(f);
            }};
#endif
  };

  if (timeout == 0) {
    m_handle = attemptLock();
    return (bool)m_handle;
  }

  auto startTime = Time::monotonicMilliseconds();
  while (true) {
    m_handle = attemptLock();
    if (m_handle)
      return true;

    if (timeout > 0 && (Time::monotonicMilliseconds() - startTime) > timeout)
      return false;

    Thread::sleep(std::min<std::int64_t>(timeout > 0 ? timeout / 4 : MaximumSleepMillis, MaximumSleepMillis));
  }
}

void LockFile::unlock() {
  m_handle.reset();
}

auto LockFile::isLocked() const -> bool {
  return (bool)m_handle;
}

}// namespace Star
