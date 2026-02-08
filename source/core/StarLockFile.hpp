#pragma once

#include "StarString.hpp"

import std;

namespace Star {

class LockFile {
public:
  // Convenience function, tries to acquire a lock, and if successful returns an
  // already locked LockFile.
  static auto acquireLock(String const& filename, std::int64_t lockTimeout = 1000) -> std::optional<LockFile>;

  LockFile(String const& filename);
  LockFile(LockFile&& lockFile) noexcept;
  // Automatically unlocks.
  ~LockFile();

  LockFile(LockFile const&) = delete;
  auto operator=(LockFile const&) -> LockFile& = delete;

  auto operator=(LockFile&& lockFile) noexcept -> LockFile&;

  // Wait at most timeout time to acquire the file lock, and return true if the
  // lock was acquired. If timeout is negative, wait forever.
  auto lock(std::int64_t timeout = 0) -> bool;
  void unlock();

  [[nodiscard]] auto isLocked() const -> bool;

private:
  static constexpr std::int64_t MaximumSleepMillis = 25;

  String m_filename;
  std::shared_ptr<void> m_handle;
};

} // namespace Star