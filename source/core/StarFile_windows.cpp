#include <io.h>
#include <stdio.h>
#include <windows.h>
import std;
import star.file;
import star.io_device;
import star.list;
import star.format;
import star.random;
import star.string;
import star.byte_array;
import star.encode;
import star.string_windows;

namespace Star {
namespace {
  OVERLAPPED makeOverlapped(StreamOffset offset) {
    OVERLAPPED overlapped = {};
    overlapped.Offset = static_cast<DWORD>(offset);
    overlapped.OffsetHigh = static_cast<DWORD>(static_cast<std::uint64_t>(offset) >> 32);
    return overlapped;
  }

  void closeAfterFailure(HANDLE file) {
    if (!CloseHandle(file))
      std::fprintf(stderr, "Error closing file after failed acquisition: %lu\n", GetLastError());
  }

  std::expected<String, StarException> temporaryRoot() {
    WCHAR path[MAX_PATH];
    auto length = GetTempPathW(MAX_PATH, path);
    if (!length)
      return std::unexpected(IOException::format("Could not call GetTempPath: {}", GetLastError()));
    if (length >= MAX_PATH)
      return std::unexpected(IOException("GetTempPath exceeds path buffer"));
    return utf16ToString(path);
  }
}

String File::convertDirSeparators(String const& path) {
  auto bytes = path.utf8();
  std::replace(bytes.begin(), bytes.end(), '/', '\\');
  return String(std::move(bytes));
}

// These queries intentionally retain wildcard matching and Windows reparse-point
// attributes, which are not equivalent to filesystem::status/symlink_status.
std::expected<bool, StarException> File::exists(String const& path) {
  auto native = stringToUtf16(path);
  if (!native)
    return std::unexpected(std::move(native.error()));
  WIN32_FIND_DATAW data;
  auto handle = FindFirstFileW(native->get(), &data);
  if (handle == INVALID_HANDLE_VALUE)
    return false;
  if (!FindClose(handle))
    return std::unexpected(IOException::format("FindClose failed: {}", GetLastError()));
  return true;
}

std::expected<bool, StarException> File::isFile(String const& path) {
  auto native = stringToUtf16(path);
  if (!native)
    return std::unexpected(std::move(native.error()));
  WIN32_FIND_DATAW data;
  auto handle = FindFirstFileW(native->get(), &data);
  if (handle == INVALID_HANDLE_VALUE)
    return false;
  if (!FindClose(handle))
    return std::unexpected(IOException::format("FindClose failed: {}", GetLastError()));
  return !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

std::expected<bool, StarException> File::isDirectory(String const& path) {
  auto bytes = path.utf8();
  while (!bytes.empty() && (bytes.back() == '\\' || bytes.back() == '/'))
    bytes.pop_back();
  auto native = stringToUtf16(String(std::move(bytes)));
  if (!native)
    return std::unexpected(std::move(native.error()));
  auto attributes = GetFileAttributesW(native->get());
  return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

std::expected<bool, StarException> File::isSymlink(String const& path) {
  auto native = stringToUtf16(path);
  if (!native)
    return std::unexpected(std::move(native.error()));
  WIN32_FIND_DATAW data;
  auto handle = FindFirstFileW(native->get(), &data);
  if (handle == INVALID_HANDLE_VALUE)
    return false;
  if (!FindClose(handle))
    return std::unexpected(IOException::format("FindClose failed: {}", GetLastError()));
  return (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && data.dwReserved0 == IO_REPARSE_TAG_SYMLINK;
}

// GetFullPathName is lexical on Windows and accepts paths that do not exist.
std::expected<String, StarException> File::fullPath(String const& path) {
  auto native = stringToUtf16(path);
  if (!native)
    return std::unexpected(std::move(native.error()));
  WCHAR buffer[MAX_PATH];
  auto length = GetFullPathNameW(native->get(), MAX_PATH, buffer, nullptr);
  if (!length)
    return std::unexpected(IOException::format("GetFullPathName failed on path '{}': {}", path, GetLastError()));
  if (length >= MAX_PATH)
    return std::unexpected(IOException::format("GetFullPathName exceeds path buffer on path '{}'", path));
  return utf16ToString(buffer);
}

std::expected<List<std::pair<String, bool>>, StarException> File::dirList(String const& dirName, bool skipDots) {
  auto native = stringToUtf16(relativeTo(dirName, "*"));
  if (!native)
    return std::unexpected(std::move(native.error()));
  std::error_code error;
  std::filesystem::path path(native->get());
  path.remove_filename();
  std::filesystem::directory_iterator entry(path, error);
  if (error)
    return std::unexpected(IOException::format("dirList failed on dir '{}': {}", dirName, error.message()));
  List<std::pair<String, bool>> files;
  // Windows root directories do not expose the special dot entries.
  if (!skipDots && path != path.root_path()) {
    files.append({".", true});
    files.append({"..", true});
  }
  for (std::filesystem::directory_iterator end; entry != end; entry.increment(error)) {
    if (error)
      return std::unexpected(IOException::format("dirList failed on dir '{}': {}", dirName, error.message()));
    auto name = utf16ToString(entry->path().filename().c_str());
    if (!name)
      return std::unexpected(std::move(name.error()));
    // Keep Windows' directory-reparse-point classification, even for a broken link.
    auto attributes = GetFileAttributesW(entry->path().c_str());
    files.append({std::move(*name), attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)});
  }
  if (error)
    return std::unexpected(IOException::format("dirList failed on dir '{}': {}", dirName, error.message()));
  return files;
}

String File::baseName(String const& fileName) {
  auto const& bytes = fileName.utf8();
  auto end = bytes.find_last_not_of("\\/");
  if (end == std::string::npos)
    return {};
  auto separator = bytes.find_last_of("\\/", end);
  auto start = separator == std::string::npos ? 0 : separator + 1;
  return String(bytes.substr(start, end + 1 - start));
}

String File::dirName(String const& fileName) {
  if (fileName == "\\" || fileName == "/")
    return "\\";
  auto const& bytes = fileName.utf8();
  auto end = bytes.find_last_not_of("\\/");
  if (end == std::string::npos)
    return ".";
  auto separator = bytes.find_last_of("\\/", end);
  if (separator == std::string::npos)
    return ".";
  auto parentEnd = bytes.find_last_not_of("\\/", separator);
  return parentEnd == std::string::npos ? String(".") : String(bytes.substr(0, parentEnd + 1));
}

String File::relativeTo(String const& relativeTo, String const& path) {
  auto const& suffix = path.utf8();
  if ((!suffix.empty() && (suffix[0] == '/' || suffix[0] == '\\'))
      || (suffix.size() >= 2 && ((suffix[0] >= 'a' && suffix[0] <= 'z') || (suffix[0] >= 'A' && suffix[0] <= 'Z')) && suffix[1] == ':'))
    return path;
  auto bytes = relativeTo.utf8();
  if (!bytes.empty() && (bytes.back() == '/' || bytes.back() == '\\'))
    bytes.pop_back();
  else if (bytes.size() >= 2 && bytes.back() == '.' && (bytes[bytes.size() - 2] == '/' || bytes[bytes.size() - 2] == '\\'))
    bytes.resize(bytes.size() - 2);
  bytes.push_back('\\');
  bytes.append(suffix, suffix.size() >= 2 && suffix[0] == '.' && (suffix[1] == '/' || suffix[1] == '\\') ? 2 : 0, std::string::npos);
  return String(std::move(bytes));
}

std::expected<String, StarException> File::temporaryFileName(RandomService& random) {
  auto root = temporaryRoot();
  if (!root)
    return std::unexpected(std::move(root.error()));
  return relativeTo(*root, strf("starbound.tmpfile.{}", hexEncode(random.randBytes(16))));
}

std::expected<std::shared_ptr<File>, StarException> File::temporaryFile(RandomService& random) {
  auto name = temporaryFileName(random);
  if (!name)
    return std::unexpected(std::move(name.error()));
  return open(*name, IOMode::ReadWrite);
}

std::expected<std::shared_ptr<File>, StarException> File::ephemeralFile(RandomService& random) {
  auto file = temporaryFile(random);
  if (!file)
    return std::unexpected(std::move(file.error()));
  auto path = stringToUtf16((*file)->fileName());
  if (!path)
    return std::unexpected(std::move(path.error()));
  if (!DeleteFileW(path->get()))
    return std::unexpected(IOException::format("Could not delete ephemeral file '{}': {}", (*file)->fileName(), GetLastError()));
  (*file)->m_filename = "";
  return std::move(*file);
}

std::expected<String, StarException> File::temporaryDirectory(RandomService& random) {
  auto root = temporaryRoot();
  if (!root)
    return std::unexpected(std::move(root.error()));
  auto directory = relativeTo(*root, strf("starbound.tmpdir.{}", hexEncode(random.randBytes(16))));
  auto made = makeDirectory(directory);
  if (!made)
    return std::unexpected(std::move(made.error()));
  return directory;
}

std::expected<void, StarException> File::rename(String const& source, String const& target) {
  auto sourcePath = stringToUtf16(source);
  if (!sourcePath)
    return std::unexpected(std::move(sourcePath.error()));
  auto targetPath = stringToUtf16(target);
  if (!targetPath)
    return std::unexpected(std::move(targetPath.error()));
  auto temp = target + ".tmp";
  auto tempPath = stringToUtf16(temp);
  if (!tempPath)
    return std::unexpected(std::move(tempPath.error()));
  auto replace = exists(target);
  if (!replace)
    return std::unexpected(std::move(replace.error()));
  if (*replace) {
    if (!DeleteFileW(tempPath->get())) {
      auto error = GetLastError();
      if (error != ERROR_FILE_NOT_FOUND)
        return std::unexpected(IOException::format("error deleting existing temp file: {}", error));
    }
    if (!MoveFileExW(targetPath->get(), tempPath->get(), MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH))
      return std::unexpected(IOException::format("error temporary file '{}': {}", temp, GetLastError()));
  }
  if (!MoveFileExW(sourcePath->get(), targetPath->get(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH))
    return std::unexpected(IOException::format("Rename error: {}", GetLastError()));
  if (*replace && !DeleteFileW(tempPath->get()))
    return std::unexpected(IOException::format("error deleting temp file '{}': {}", temp, GetLastError()));
  return {};
}

std::expected<void, StarException> File::overwriteFileWithRename(char const* data, std::size_t len, String const& filename, String const& newSuffix) {
  auto newFile = filename + newSuffix;
  auto atomicWrite = [&]() -> std::expected<void, StarException> {
    auto file = open(newFile, IOMode::Write | IOMode::Truncate);
    if (!file)
      return std::unexpected(std::move(file.error()));
    auto written = (*file)->writeFull({data, len});
    if (!written)
      return std::unexpected(std::move(written.error()));
    auto synced = (*file)->sync();
    if (!synced)
      return std::unexpected(std::move(synced.error()));
    auto closed = (*file)->close();
    if (!closed)
      return std::unexpected(std::move(closed.error()));
    return rename(newFile, filename);
  };
  auto result = atomicWrite();
  if (result || !result.error().is<IOException>())
    return result;
  // Preserve the existing Windows recovery boundary: IO failures in the
  // write/flush/move sequence retry a non-atomic write. Removal is best effort.
  auto path = stringToUtf16(newFile);
  if (!path)
    return std::unexpected(std::move(path.error()));
  ::_wremove(path->get());
  return writeFile(data, len, filename);
}

std::expected<void*, StarException> File::fopen(char const* filename, IOMode mode) {
  auto path = stringToUtf16(String(filename));
  if (!path)
    return std::unexpected(std::move(path.error()));
  DWORD access = 0;
  if (mode & IOMode::Read)
    access |= GENERIC_READ;
  if (mode & IOMode::Write)
    access |= GENERIC_WRITE;
  if (!access)
    return std::unexpected(IOException("Cannot open file with closed IO mode"));
  auto file = CreateFileW(path->get(), access, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      nullptr, mode & IOMode::Write ? OPEN_ALWAYS : OPEN_EXISTING, 0, nullptr);
  if (file == INVALID_HANDLE_VALUE)
    return std::unexpected(IOException::format("could not open file '{}' {}", filename, GetLastError()));
  LARGE_INTEGER zero = {};
  if (!SetFilePointerEx(file, zero, nullptr, FILE_BEGIN)) {
    auto error = GetLastError();
    closeAfterFailure(file);
    return std::unexpected(IOException::format("could not set file pointer in fopen '{}' {}", filename, error));
  }
  if (mode & IOMode::Truncate && !SetEndOfFile(file)) {
    auto error = GetLastError();
    closeAfterFailure(file);
    return std::unexpected(IOException::format("could not set end of file in fopen '{}' {}", filename, error));
  }
  if (mode & IOMode::Append) {
    if (!SetFilePointerEx(file, zero, nullptr, FILE_END)) {
      auto error = GetLastError();
      closeAfterFailure(file);
      return std::unexpected(IOException::format("could not set file pointer in fopen '{}' {}", filename, error));
    }
  }
  return static_cast<void*>(file);
}

std::expected<void, StarException> File::fseek(void* file, StreamOffset offset, IOSeek seek) {
  LARGE_INTEGER distance;
  distance.QuadPart = offset;
  DWORD origin = seek == IOSeek::Relative ? FILE_CURRENT : seek == IOSeek::Absolute ? FILE_BEGIN : FILE_END;
  if (!SetFilePointerEx(file, distance, nullptr, origin))
    return std::unexpected(IOException::format("Seek error {}", GetLastError()));
  return {};
}

std::expected<StreamOffset, StarException> File::ftell(void* file) {
  LARGE_INTEGER zero = {};
  LARGE_INTEGER position;
  if (!SetFilePointerEx(file, zero, &position, FILE_CURRENT))
    return std::unexpected(IOException::format("Tell error {}", GetLastError()));
  return position.QuadPart;
}

std::expected<std::size_t, StarException> File::fread(void* file, char* data, std::size_t len) {
  if (!len)
    return 0;
  DWORD count = 0;
  if (!ReadFile(file, data, static_cast<DWORD>(std::min<std::size_t>(len, MAXDWORD)), &count, nullptr)) {
    auto error = GetLastError();
    if (error != ERROR_IO_PENDING)
      return std::unexpected(IOException::format("read error {}", error));
  }
  return count;
}

std::expected<std::size_t, StarException> File::fwrite(void* file, char const* data, std::size_t len) {
  if (!len)
    return 0;
  DWORD count = 0;
  if (!WriteFile(file, data, static_cast<DWORD>(std::min<std::size_t>(len, MAXDWORD)), &count, nullptr)) {
    auto error = GetLastError();
    if (error != ERROR_IO_PENDING)
      return std::unexpected(IOException::format("write error {}", error));
  }
  return count;
}

std::expected<void, StarException> File::fsync(void* file) {
  if (!FlushFileBuffers(file))
    return std::unexpected(IOException::format("fsync error {}", GetLastError()));
  return {};
}

std::expected<void, StarException> File::fclose(void* file) {
  if (!CloseHandle(file))
    return std::unexpected(IOException::format("Close error {}", GetLastError()));
  return {};
}

std::expected<StreamOffset, StarException> File::fsize(void* file) {
  LARGE_INTEGER size;
  if (!GetFileSizeEx(file, &size))
    return std::unexpected(IOException::format("could not get file size in fsize {}", GetLastError()));
  return size.QuadPart;
}

std::expected<std::size_t, StarException> File::pread(void* file, char* data, std::size_t len, StreamOffset position) {
  auto cursor = ftell(file);
  if (!cursor)
    return std::unexpected(std::move(cursor.error()));
  DWORD count = 0;
  auto overlapped = makeOverlapped(position);
  bool read = ReadFile(file, data, static_cast<DWORD>(std::min<std::size_t>(len, MAXDWORD)), &count, &overlapped);
  auto error = read ? ERROR_SUCCESS : GetLastError();
  auto restored = fseek(file, *cursor, IOSeek::Absolute);
  if (!read && error != ERROR_IO_PENDING) {
    if (!restored)
      std::fprintf(stderr, "Error restoring file cursor after failed pread: %s\n", restored.error().what());
    return std::unexpected(IOException::format("pread error {}", error));
  }
  if (!restored)
    return std::unexpected(std::move(restored.error()));
  return count;
}

std::expected<std::size_t, StarException> File::pwrite(void* file, char const* data, std::size_t len, StreamOffset position) {
  auto cursor = ftell(file);
  if (!cursor)
    return std::unexpected(std::move(cursor.error()));
  DWORD count = 0;
  auto overlapped = makeOverlapped(position);
  bool written = WriteFile(file, data, static_cast<DWORD>(std::min<std::size_t>(len, MAXDWORD)), &count, &overlapped);
  auto error = written ? ERROR_SUCCESS : GetLastError();
  auto restored = fseek(file, *cursor, IOSeek::Absolute);
  if (!written && error != ERROR_IO_PENDING) {
    if (!restored)
      std::fprintf(stderr, "Error restoring file cursor after failed pwrite: %s\n", restored.error().what());
    return std::unexpected(IOException::format("pwrite error {}", error));
  }
  if (!restored)
    return std::unexpected(std::move(restored.error()));
  return count;
}

std::expected<void, StarException> File::resize(void* file, StreamOffset size) {
  auto moved = fseek(file, size, IOSeek::Absolute);
  if (!moved)
    return std::unexpected(std::move(moved.error()));
  if (!SetEndOfFile(file))
    return std::unexpected(IOException::format("resize error {}", GetLastError()));
  return {};
}

}
