#pragma once

#include "StarMaybe.hpp"
#include "StarString.hpp"

namespace Star {

class LockFile {
public:
  // Convenience function, tries to acquire a lock, and if successful returns an
  // already locked
  // LockFile.
  static Maybe<LockFile> acquireLock(String filename, int64_t lockTimeout = 1000);

  LockFile(String filename);
  LockFile(LockFile&& lockFile);
  // Automatically unlocks.
  ~LockFile();

  LockFile(LockFile const&) = delete;
  LockFile& operator=(LockFile const&) = delete;

  LockFile& operator=(LockFile&& lockFile);

  // Wait at most timeout time to acquire the file lock, and return true if the
  // lock was acquired.  If timeout is negative, wait forever.
  bool lock(int64_t timeout = 0);
  void unlock();

  bool isLocked() const;

private:
  static constexpr int64_t MaximumSleepMillis = 25;

  String m_filename;
  shared_ptr<void> m_handle;
};

}// namespace Star
