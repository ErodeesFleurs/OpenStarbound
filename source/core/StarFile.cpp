#include <stdio.h>

import star.file;
import star.io_device;
import star.string;
import star.format;
import std;
import star.byte_array;

namespace Star {

namespace {
  // Delete the link itself, never traverse its target.
  std::expected<bool, StarException> removeSymlink(String const& fileName) {
    auto link = File::isSymlink(fileName);
    if (!link)
      return std::unexpected(std::move(link.error()));
    if (!*link)
      return false;
    auto removed = File::remove(fileName);
    if (!removed)
      return std::unexpected(std::move(removed.error()));
    return true;
  }
}


std::expected<void, StarException> File::makeDirectoryRecursive(String const& fileName) {
  auto parent = dirName(fileName);
  auto parentExists = isDirectory(parent);
  if (!parentExists)
    return std::unexpected(std::move(parentExists.error()));
  if (!*parentExists) {
    if (parent == fileName)
      return std::unexpected(IOException::format("Cannot create directory ancestor '{}'", parent));
    auto result = makeDirectoryRecursive(parent);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  auto directoryExists = isDirectory(fileName);
  if (!directoryExists)
    return std::unexpected(std::move(directoryExists.error()));
  if (!*directoryExists)
    return makeDirectory(fileName);
  return {};
}

std::expected<void, StarException> File::removeDirectoryRecursive(String const& fileName) {
  auto link = removeSymlink(fileName);
  if (!link)
    return std::unexpected(std::move(link.error()));
  if (*link)
    return {};
  auto entries = dirList(fileName);
  if (!entries)
    return std::unexpected(std::move(entries.error()));
  for (auto const& [entry, isDir] : *entries) {
    auto path = relativeTo(fileName, entry);
    auto childLink = removeSymlink(path);
    if (!childLink)
      return std::unexpected(std::move(childLink.error()));
    if (*childLink)
      continue;
    auto result = isDir ? removeDirectoryRecursive(path) : remove(path);
    if (!result)
      return std::unexpected(std::move(result.error()));
  }
  return remove(fileName);
}

std::expected<void, StarException> File::copy(String const& source, String const& target) {
  auto sourceFile = File::open(source, IOMode::Read);
  if (!sourceFile)
    return std::unexpected(std::move(sourceFile.error()));
  auto targetFile = File::open(target, IOMode::ReadWrite);
  if (!targetFile)
    return std::unexpected(std::move(targetFile.error()));
  auto resized = (*targetFile)->resize(0);
  if (!resized)
    return std::unexpected(std::move(resized.error()));
  char buffer[1024];
  while (true) {
    auto end = (*sourceFile)->atEnd();
    if (!end)
      return std::unexpected(std::move(end.error()));
    if (*end)
      break;
    auto count = (*sourceFile)->read(buffer, sizeof(buffer));
    if (!count)
      return std::unexpected(std::move(count.error()));
    auto written = (*targetFile)->writeFull(buffer, *count);
    if (!written)
      return std::unexpected(std::move(written.error()));
  }
  auto closed = (*targetFile)->close();
  if (!closed)
    return std::unexpected(std::move(closed.error()));
  return (*sourceFile)->close();
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
    auto count = (*file)->read(buffer, sizeof(buffer));
    if (!count)
      return std::unexpected(std::move(count.error()));
    bytes.append(buffer, *count);
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
    auto count = (*file)->read(buffer, sizeof(buffer));
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

std::expected<void, StarException> File::writeFile(char const* data, std::size_t len, String const& filename) {
  auto file = File::open(filename, IOMode::Write | IOMode::Truncate);
  if (!file)
    return std::unexpected(std::move(file.error()));
  auto written = (*file)->writeFull(data, len);
  if (!written)
    return std::unexpected(std::move(written.error()));
  return (*file)->close();
}

std::expected<void, StarException> File::writeFile(ByteArray const& data, String const& filename) {
  return writeFile(data.ptr(), data.size(), filename);
}

std::expected<void, StarException> File::writeFile(String const& data, String const& filename) {
  return writeFile(data.utf8Ptr(), data.utf8Size(), filename);
}

std::expected<void, StarException> File::overwriteFileWithRename(ByteArray const& data, String const& filename, String const& newSuffix) {
  return overwriteFileWithRename(data.ptr(), data.size(), filename, newSuffix);
}

std::expected<void, StarException> File::overwriteFileWithRename(String const& data, String const& filename, String const& newSuffix) {
  return overwriteFileWithRename(data.utf8Ptr(), data.utf8Size(), filename, newSuffix);
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

std::expected<std::size_t, StarException> File::read(char* data, std::size_t len) {
  if (!m_file)
    return std::unexpected(IOException("read called on closed File"));
  if (!isReadable())
    return std::unexpected(IOException("read called on non-readable File"));
  return fread(m_file, data, len);
}

std::expected<std::size_t, StarException> File::write(char const* data, std::size_t len) {
  if (!m_file)
    return std::unexpected(IOException("write called on closed File"));
  if (!isWritable())
    return std::unexpected(IOException("write called on non-writable File"));
  return fwrite(m_file, data, len);
}

std::expected<std::size_t, StarException> File::readAbsolute(StreamOffset position, char* data, std::size_t len) {
  if (!m_file || !isReadable())
    return std::unexpected(IOException("readAbsolute called on closed or non-readable File"));
  return pread(m_file, data, len, position);
}

std::expected<std::size_t, StarException> File::writeAbsolute(StreamOffset position, char const* data, std::size_t len) {
  if (!m_file || !isWritable())
    return std::unexpected(IOException("writeAbsolute called on closed or non-writable File"));
  return pwrite(m_file, data, len, position);
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
