#pragma once

#include "StarIODevice.hpp"
#include "StarString.hpp"

import std;

namespace Star {

// All file methods are thread safe.
class File : public IODevice {
public:
  // Converts the passed in path to use the platform specific directory
  // separators only.
  static auto convertDirSeparators(String const& path) -> String;

  // All static file operations here throw IOException on error.
  static auto currentDirectory() -> String;
  static void changeDirectory(String const& dirName);
  static void makeDirectory(String const& dirName);
  static void makeDirectoryRecursive(String const& dirName);

  // List all files or directories under given directory.
  static auto dirList(String const& dirName, bool skipDots = true) -> List<std::pair<String, bool>>;

  static auto baseName(String const& fileName) -> String;
  static auto dirName(String const& fileName) -> String;

  // Resolve a path relative to another path.
  static auto relativeTo(String const& relativeTo, String const& path) -> String;

  // Resolve the given possibly relative path into an absolute path.
  static auto fullPath(String const& path) -> String;

  static auto temporaryFileName() -> String;

  // Creates and opens a new ReadWrite temporary file.
  static auto temporaryFile() -> Ptr<File>;

  // Creates and opens new ReadWrite temporary file that will be removed on close.
  static auto ephemeralFile() -> Ptr<File>;

  static auto temporaryDirectory() -> String;

  static auto exists(String const& path) -> bool;

  static auto isFile(String const& path) -> bool;
  static auto isDirectory(String const& path) -> bool;

  static void remove(String const& filename);
  static void removeDirectoryRecursive(String const& filename);

  static void rename(String const& source, String const& target);

  static void copy(String const& source, String const& target);

  static auto readFile(String const& filename) -> ByteArray;
  static auto readFileString(String const& filename) -> String;
  static auto fileSize(String const& filename) -> std::int64_t;

  static void writeFile(char const* data, std::size_t len, String const& filename);
  static void writeFile(ByteArray const& data, String const& filename);
  static void writeFile(String const& data, String const& filename);

  static void overwriteFileWithRename(char const* data, std::size_t len, String const& filename, String const& newSuffix = ".new");
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

  void remove();

  auto pos() -> std::int64_t override;
  void seek(std::int64_t pos, IOSeek seek = IOSeek::Absolute) override;
  void resize(std::int64_t size) override;
  auto size() -> std::int64_t override;
  auto atEnd() -> bool override;
  auto read(char* data, std::size_t len) -> std::size_t override;
  auto write(char const* data, std::size_t len) -> std::size_t override;

  auto readAbsolute(std::int64_t readPosition, char* data, std::size_t len) -> std::size_t override;
  auto writeAbsolute(std::int64_t writePosition, char const* data, std::size_t len) -> std::size_t override;

  void open(IOMode mode) override;
  void close() override;

  void sync() override;

  auto deviceName() const -> String override;

  auto clone() -> Ptr<IODevice> override;

private:
  void setupFstream(IOMode mode);

  String m_filename;
  std::fstream m_file;
  bool m_ephemeral = false;
  mutable std::recursive_mutex m_mutex;
};

}// namespace Star
