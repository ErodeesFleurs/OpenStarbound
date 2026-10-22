module;
import std;
import star.string;
import star.format;

namespace Star {

class LockFile {
public:
  // Convenience function, tries to acquire a lock, and if succesfull returns an
  // already locked
  // LockFile.
  [[nodiscard]] static std::expected<std::optional<LockFile>, StarException> acquireLock(String const& filename, std::int64_t lockTimeout = 1000);

  LockFile(String const& filename);
  LockFile(LockFile&& lockFile);
  // Automatically unlocks.
  ~LockFile();

  LockFile(LockFile const&) = delete;
  LockFile& operator=(LockFile const&) = delete;

  LockFile& operator=(LockFile&& lockFile);

  // Wait at most timeout time to acquire the file lock, and return true if the
  // lock was acquired.  If timeout is negative, wait forever.
  [[nodiscard]] std::expected<bool, StarException> lock(std::int64_t timeout = 0);
  // Consumes the handle, including when releasing it fails.
  [[nodiscard]] std::expected<void, StarException> unlock();

  bool isLocked() const;

private:
  static std::int64_t const MaximumSleepMillis = 25;

  String m_filename;
  std::shared_ptr<void> m_handle;
};

}

export module star.lock_file;

export namespace Star {
  using ::Star::LockFile;
}
