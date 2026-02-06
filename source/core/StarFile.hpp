#pragma once

#include "StarIODevice.hpp"
#include "StarString.hpp"

import std;

namespace Star {

// All file methods are thread safe.
class File : public IODevice {
public:
  // Converts the passed in path to use the platform specific directory
  // separators only (Windows supports '/' just fine, this is mostly for
  // uniform appearance).  Does *nothing else* (no validity checks, etc).
  static auto convertDirSeparators(String const& path) -> String;

  // All static file operations here throw IOException on error.
  // get the current working directory
  static auto currentDirectory() -> String;
  // set the current working directory.
  static void changeDirectory(String const& dirName);
  static void makeDirectory(String const& dirName);
  static void makeDirectoryRecursive(String const& dirName);

  // List all files or directories under given directory.  skipDots skips the
  // special '.' and '..' entries.  Bool value is true for directories.
  static auto dirList(String const& dirName, bool skipDots = true) -> List<std::pair<String, bool>>;

  // Returns the final component of the given path with no directory separators
  static auto baseName(String const& fileName) -> String;
  // All components of the given path minus the final component, separated by
  // the directory separator
  static auto dirName(String const& fileName) -> String;

  // Resolve a path relative to another path.  If the given path is absolute,
  // then the given path is returned unmodified.
  static auto relativeTo(String const& relativeTo, String const& path) -> String;

  // Resolve the given possibly relative path into an absolute path.
  static auto fullPath(String const& path) -> String;

  static auto temporaryFileName() -> String;

  // Creates and opens a new ReadWrite temporary file with a real path that can
  // be closed and re-opened.  Will not be removed automatically.
  static auto temporaryFile() -> Ptr<File>;

  // Creates and opens new ReadWrite temporary file and opens it.  This file
  // has no filename and will be removed on close.
  static auto ephemeralFile() -> Ptr<File>;

  // Creates a new temporary directory and reutrns the path.  Will not be
  // removed automatically.
  static auto temporaryDirectory() -> String;

  static auto exists(String const& path) -> bool;

  // Does the file exist and is it a regular file (not a directory or special
  // file)?
  static auto isFile(String const& path) -> bool;
  // Is the file a directory?
  static auto isDirectory(String const& path) -> bool;

  static void remove(String const& filename);
  static void removeDirectoryRecursive(String const& filename);

  // Moves the source file to the target path, overwriting the target path if
  // it already exists.
  static void rename(String const& source, String const& target);

  // Copies the source file to the target, overwriting the target path if it
  // already exists.
  static void copy(String const& source, String const& target);

  static auto readFile(String const& filename) -> ByteArray;
  static auto readFileString(String const& filename) -> String;
  static auto fileSize(String const& filename) -> std::int64_t;

  static void writeFile(char const* data, size_t len, String const& filename);
  static void writeFile(ByteArray const& data, String const& filename);
  static void writeFile(String const& data, String const& filename);

  // Write a new file, potentially overwriting an existing file, in the safest
  // way possible while preserving the old file in the same directory until the
  // operation completes.  Writes to the same path as the existing file to
  // avoid different partition copying.  This may clobber anything in the given
  // path that matches filename + newSuffix.
  static void overwriteFileWithRename(char const* data, size_t len, String const& filename, String const& newSuffix = ".new");
  static void overwriteFileWithRename(ByteArray const& data, String const& filename, String const& newSuffix = ".new");
  static void overwriteFileWithRename(String const& data, String const& filename, String const& newSuffix = ".new");

  static void backupFileInSequence(String const& initialFile, String const& targetFile, unsigned maximumBackups, String const& backupExtensionPrefix = ".");
  static void backupFileInSequence(String const& targetFile, unsigned maximumBackups, String const& backupExtensionPrefix = ".");

  static auto open(String const& filename, IOMode mode) -> Ptr<File>;

  File();
  File(String filename);
  ~File() override;

  auto fileName() const -> String;
  void setFilename(String filename);

  // File is closed before removal.
  void remove();

  auto pos() -> std::int64_t override;
  void seek(std::int64_t pos, IOSeek seek = IOSeek::Absolute) override;
  void resize(std::int64_t size) override;
  auto size() -> std::int64_t override;
  auto atEnd() -> bool override;
  auto read(char* data, size_t len) -> size_t override;
  auto write(char const* data, size_t len) -> size_t override;

  // Do an immediate read / write of an absolute location in the file, without
  // modifying the current file cursor.  Safe to call in a threaded context
  // with other reads and writes, but not safe vs changing the File state like
  // open and close.
  auto readAbsolute(std::int64_t readPosition, char* data, size_t len) -> size_t override;
  auto writeAbsolute(std::int64_t writePosition, char const* data, size_t len) -> size_t override;

  void open(IOMode mode) override;
  void close() override;

  void sync() override;

  auto deviceName() const -> String override;

  auto clone() -> Ptr<IODevice> override;

private:
  static auto fopen(char const* filename, IOMode mode) -> void*;
  static void fseek(void* file, std::int64_t offset, IOSeek seek);
  static auto ftell(void* file) -> std::int64_t;
  static auto fread(void* file, char* data, size_t len) -> size_t;
  static auto fwrite(void* file, char const* data, size_t len) -> size_t;
  static void fsync(void* file);
  static void fclose(void* file);
  static auto fsize(void* file) -> std::int64_t;
  static auto pread(void* file, char* data, size_t len, std::int64_t absPosition) -> size_t;
  static auto pwrite(void* file, char const* data, size_t len, std::int64_t absPosition) -> size_t;
  static void resize(void* file, std::int64_t size);

  String m_filename;
  void* m_file;
};

}
