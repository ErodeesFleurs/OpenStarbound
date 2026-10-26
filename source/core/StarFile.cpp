#include <stdio.h>

import star.file;
import star.io_device;
import star.string;
import star.format;
import std;
import star.byte_array;
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
import star.string_windows;
#endif

namespace Star {

namespace {
  std::expected<std::filesystem::path, StarException> filesystemPath(String const& path) {
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
    auto native = stringToUtf16(path);
    if (!native)
      return std::unexpected(std::move(native.error()));
    return std::filesystem::path(native->get());
#else
    return std::filesystem::path(path.utf8Ptr());
#endif
  }
}

std::expected<String, StarException> File::currentDirectory() {
  std::error_code error;
  auto path = std::filesystem::current_path(error);
  if (error)
    return std::unexpected(IOException::format("could not get current directory: {}", error.message()));
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
  return utf16ToString(path.c_str());
#else
  return String(path.native());
#endif
}

std::expected<void, StarException> File::changeDirectory(String const& dirName) {
  auto path = filesystemPath(dirName);
  if (!path)
    return std::unexpected(std::move(path.error()));
  std::error_code error;
  std::filesystem::current_path(*path, error);
  if (error)
    return std::unexpected(IOException::format("could not change directory to '{}': {}", dirName, error.message()));
  return {};
}

std::expected<void, StarException> File::makeDirectory(String const& dirName) {
  auto path = filesystemPath(dirName);
  if (!path)
    return std::unexpected(std::move(path.error()));
  std::error_code error;
  if (!std::filesystem::create_directory(*path, error)) {
    // Unlike create_directory, a single mkdir must reject an existing directory.
    if (!error)
      error = std::make_error_code(std::errc::file_exists);
    return std::unexpected(IOException::format("could not create directory '{}': {}", dirName, error.message()));
  }
  return {};
}

std::expected<void, StarException> File::makeDirectoryRecursive(String const& fileName) {
  auto path = filesystemPath(fileName);
  if (!path)
    return std::unexpected(std::move(path.error()));
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
  // The native helper rejects bare, drive-relative roots after trimming separators.
  if (path->has_root_directory() && !path->has_root_name() && !path->has_relative_path())
    return std::unexpected(IOException::format("Cannot create directory ancestor '{}'", fileName));
#endif
  std::error_code error;
  std::filesystem::create_directories(*path, error);
  if (error)
    return std::unexpected(IOException::format("could not create directory '{}': {}", fileName, error.message()));
  return {};
}

std::expected<void, StarException> File::remove(String const& fileName) {
  auto path = filesystemPath(fileName);
  if (!path)
    return std::unexpected(std::move(path.error()));
  std::error_code error;
  if (!std::filesystem::remove(*path, error)) {
    // Missing paths are an error in the File interface, not a successful no-op.
    if (!error)
      error = std::make_error_code(std::errc::no_such_file_or_directory);
    return std::unexpected(IOException::format("remove error on '{}': {}", fileName, error.message()));
  }
  return {};
}

std::expected<void, StarException> File::removeDirectoryRecursive(String const& fileName) {
  auto path = filesystemPath(fileName);
  if (!path)
    return std::unexpected(std::move(path.error()));
  std::error_code error;
  auto status = std::filesystem::symlink_status(*path, error);
  if (error)
    return std::unexpected(IOException::format("could not remove directory '{}': {}", fileName, error.message()));
  // A trailing separator must still require a directory, but must not turn a
  // directory symlink into a traversal of its target.
  if (path->has_relative_path() && path->filename().empty()) {
    while (path->has_relative_path() && path->filename().empty())
      *path = path->parent_path();
    status = std::filesystem::symlink_status(*path, error);
    if (error)
      return std::unexpected(IOException::format("could not remove directory '{}': {}", fileName, error.message()));
  }
  // remove_all also accepts regular files and missing paths; this API does not.
  if (!std::filesystem::is_directory(status) && !std::filesystem::is_symlink(status))
    return std::unexpected(IOException::format("cannot recursively remove non-directory '{}'", fileName));
  if (std::filesystem::remove_all(*path, error) == 0 && !error)
    error = std::make_error_code(std::errc::no_such_file_or_directory);
  if (error)
    return std::unexpected(IOException::format("could not remove directory '{}': {}", fileName, error.message()));
  return {};
}

std::expected<void, StarException> File::copy(String const& source, String const& target) {
  auto sourcePath = filesystemPath(source);
  if (!sourcePath)
    return std::unexpected(std::move(sourcePath.error()));
  auto targetPath = filesystemPath(target);
  if (!targetPath)
    return std::unexpected(std::move(targetPath.error()));
  std::error_code error;
  std::filesystem::copy_file(*sourcePath, *targetPath, std::filesystem::copy_options::overwrite_existing, error);
  if (error)
    return std::unexpected(IOException::format("could not copy '{}' to '{}': {}", source, target, error.message()));
  return {};
}

std::expected<std::shared_ptr<File>, StarException> File::open(String const& filename, IOMode mode) {
  auto file = std::make_shared<File>(filename);
  auto opened = file->open(mode);
  if (!opened)
    return std::unexpected(std::move(opened.error()));
  return file;
}

std::expected<ByteArray, StarException> File::readFile(String const& filename) {
  auto file = File::open(filename, IOMode::Read);
  if (!file)
    return std::unexpected(std::move(file.error()));
  ByteArray bytes;
  while (true) {
    auto end = (*file)->atEnd();
    if (!end)
      return std::unexpected(std::move(end.error()));
    if (*end)
      break;
    char buffer[1024];
    auto count = (*file)->read(buffer);
    if (!count)
      return std::unexpected(std::move(count.error()));
    bytes.append(std::span{buffer, *count});
  }
  auto closed = (*file)->close();
  if (!closed)
    return std::unexpected(std::move(closed.error()));
  return bytes;
}

std::expected<String, StarException> File::readFileString(String const& filename) {
  auto file = File::open(filename, IOMode::Read);
  if (!file)
    return std::unexpected(std::move(file.error()));
  std::string text;
  while (true) {
    auto end = (*file)->atEnd();
    if (!end)
      return std::unexpected(std::move(end.error()));
    if (*end)
      break;
    char buffer[1024];
    auto count = (*file)->read(buffer);
    if (!count)
      return std::unexpected(std::move(count.error()));
    text.append(buffer, *count);
  }
  auto closed = (*file)->close();
  if (!closed)
    return std::unexpected(std::move(closed.error()));
  return String(std::move(text));
}

std::expected<StreamOffset, StarException> File::fileSize(String const& filename) {
  auto file = File::open(filename, IOMode::Read);
  if (!file)
    return std::unexpected(std::move(file.error()));
  auto result = (*file)->size();
  if (!result)
    return std::unexpected(std::move(result.error()));
  auto closed = (*file)->close();
  if (!closed)
    return std::unexpected(std::move(closed.error()));
  return *result;
}

std::expected<void, StarException> File::writeFile(std::span<char const> data, String const& filename) {
  auto file = File::open(filename, IOMode::Write | IOMode::Truncate);
  if (!file)
    return std::unexpected(std::move(file.error()));
  auto written = (*file)->writeFull(data);
  if (!written)
    return std::unexpected(std::move(written.error()));
  return (*file)->close();
}

std::expected<void, StarException> File::writeFile(ByteArray const& data, String const& filename) {
  return writeFile(data.span(), filename);
}

std::expected<void, StarException> File::writeFile(String const& data, String const& filename) {
  return writeFile(std::span<char const>(data.utf8()), filename);
}

std::expected<void, StarException> File::overwriteFileWithRename(ByteArray const& data, String const& filename, String const& newSuffix) {
  return overwriteFileWithRename(data.span(), filename, newSuffix);
}

std::expected<void, StarException> File::overwriteFileWithRename(String const& data, String const& filename, String const& newSuffix) {
  return overwriteFileWithRename(std::span<char const>(data.utf8()), filename, newSuffix);
}

std::expected<void, StarException> File::backupFileInSequence(String const& initialFile, String const& targetFile, unsigned maximumBackups, String const& backupExtensionPrefix) {
  for (unsigned i = maximumBackups; i > 0; --i) {
    bool initial = i == 1;
    String const& sourceFile = initial ? initialFile : targetFile;
    String curExtension = initial ? "" : strf("{}{}", backupExtensionPrefix, i - 1);
    String nextExtension = strf("{}{}", backupExtensionPrefix, i);
    auto sourceExists = File::isFile(sourceFile + curExtension);
    if (!sourceExists)
      return std::unexpected(std::move(sourceExists.error()));
    if (*sourceExists) {
      auto copied = File::copy(sourceFile + curExtension, targetFile + nextExtension);
      if (!copied)
        return std::unexpected(std::move(copied.error()));
    }
  }
  return {};
}

std::expected<void, StarException> File::backupFileInSequence(String const& targetFile, unsigned maximumBackups, String const& backupExtensionPrefix) {
  return backupFileInSequence(targetFile, targetFile, maximumBackups, backupExtensionPrefix);
}

File::File() : IODevice(IOMode::Closed), m_file(nullptr) {}

File::File(String filename)
  : IODevice(IOMode::Closed), m_filename(std::move(filename)), m_file(nullptr) {}

File::~File() noexcept {
  auto result = close();
  if (!result)
    std::fprintf(stderr, "Error closing file '%s' during destruction: %s\n", m_filename.utf8Ptr(), result.error().what());
}

std::expected<StreamOffset, StarException> File::pos() {
  if (!m_file)
    return std::unexpected(IOException("pos called on closed File"));
  return ftell(m_file);
}

std::expected<void, StarException> File::seek(StreamOffset offset, IOSeek seekMode) {
  if (!m_file)
    return std::unexpected(IOException("seek called on closed File"));
  return fseek(m_file, offset, seekMode);
}

std::expected<StreamOffset, StarException> File::size() {
  if (!m_file)
    return std::unexpected(IOException("size called on closed File"));
  return fsize(m_file);
}

std::expected<bool, StarException> File::atEnd() {
  auto position = pos();
  if (!position)
    return std::unexpected(std::move(position.error()));
  auto length = size();
  if (!length)
    return std::unexpected(std::move(length.error()));
  return *position >= *length;
}

std::expected<std::size_t, StarException> File::read(std::span<char> data) {
  if (!m_file)
    return std::unexpected(IOException("read called on closed File"));
  if (!isReadable())
    return std::unexpected(IOException("read called on non-readable File"));
  return fread(m_file, data.data(), data.size());
}

std::expected<std::size_t, StarException> File::write(std::span<char const> data) {
  if (!m_file)
    return std::unexpected(IOException("write called on closed File"));
  if (!isWritable())
    return std::unexpected(IOException("write called on non-writable File"));
  return fwrite(m_file, data.data(), data.size());
}

std::expected<std::size_t, StarException> File::readAbsolute(StreamOffset position, std::span<char> data) {
  if (!m_file || !isReadable())
    return std::unexpected(IOException("readAbsolute called on closed or non-readable File"));
  return pread(m_file, data.data(), data.size(), position);
}

std::expected<std::size_t, StarException> File::writeAbsolute(StreamOffset position, std::span<char const> data) {
  if (!m_file || !isWritable())
    return std::unexpected(IOException("writeAbsolute called on closed or non-writable File"));
  return pwrite(m_file, data.data(), data.size(), position);
}

String File::fileName() const {
  return m_filename;
}

std::expected<void, StarException> File::setFilename(String filename) {
  if (isOpen())
    return std::unexpected(IOException("Cannot call setFilename while File is open"));
  m_filename = std::move(filename);
  return {};
}

std::expected<void, StarException> File::remove() {
  auto closed = close();
  if (!closed)
    return std::unexpected(std::move(closed.error()));
  if (m_filename.empty())
    return std::unexpected(IOException("Cannot remove file, no filename set"));
  return remove(m_filename);
}

std::expected<void, StarException> File::resize(StreamOffset length) {
  bool tempOpen = !isOpen();
  if (tempOpen) {
    auto opened = open(mode());
    if (!opened)
      return std::unexpected(std::move(opened.error()));
  }
  auto result = File::resize(m_file, length);
  if (tempOpen) {
    auto closed = close();
    if (!closed) {
      if (result)
        return std::unexpected(std::move(closed.error()));
      std::fprintf(stderr, "Error closing file '%s' after failed resize: %s\n", m_filename.utf8Ptr(), closed.error().what());
    }
  }
  return result;
}

std::expected<void, StarException> File::sync() {
  if (!m_file)
    return std::unexpected(IOException("sync called on closed File"));
  return fsync(m_file);
}

std::expected<void, StarException> File::open(IOMode mode) {
  auto closed = close();
  if (!closed)
    return std::unexpected(std::move(closed.error()));
  if (m_filename.empty())
    return std::unexpected(IOException("Cannot open file, no filename set"));
  auto opened = fopen(m_filename.utf8Ptr(), mode);
  if (!opened)
    return std::unexpected(std::move(opened.error()));
  m_file = *opened;
  setMode(mode);
  return {};
}

std::expected<void, StarException> File::close() {
  // Consume first: a failed close must never later close a reused descriptor.
  auto file = std::exchange(m_file, nullptr);
  setMode(IOMode::Closed);
  if (file)
    return fclose(file);
  return {};
}

std::expected<String, StarException> File::deviceName() const {
  return m_filename.empty() ? String("<unnamed temp file>") : m_filename;
}

std::expected<std::shared_ptr<IODevice>, StarException> File::clone() {
  auto cloned = std::make_shared<File>(m_filename);
  if (isOpen()) {
    auto opened = cloned->open(mode());
    if (!opened)
      return std::unexpected(std::move(opened.error()));
    auto position = pos();
    if (!position)
      return std::unexpected(std::move(position.error()));
    auto moved = cloned->seek(*position, IOSeek::Absolute);
    if (!moved)
      return std::unexpected(std::move(moved.error()));
  }
  return std::shared_ptr<IODevice>(std::move(cloned));
}

}
