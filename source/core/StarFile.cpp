#include "StarFile.hpp"

#include "StarEncode.hpp"
#include "StarFormat.hpp"
#include "StarRandom.hpp"

import std;

namespace fs = std::filesystem;

namespace Star {

namespace {
auto fromU8Path(fs::path const& p) -> String {
  auto u8 = p.u8string();
  return {(char const*)u8.data(), u8.size()};
}

auto ioModeToStd(IOMode mode) -> std::ios_base::openmode {
  std::ios_base::openmode stdMode = std::ios::binary;
  if (mode & IOMode::Read)
    stdMode |= std::ios::in;
  if (mode & IOMode::Write)
    stdMode |= std::ios::out;
  if (mode & IOMode::Append)
    stdMode |= std::ios::app;
  if (mode & IOMode::Truncate)
    stdMode |= std::ios::trunc;
  return stdMode;
}
}// namespace

auto File::convertDirSeparators(String const& path) -> String {
  return fromU8Path(fs::path(path.utf8()).make_preferred());
}

auto File::currentDirectory() -> String {
  try {
    return fromU8Path(fs::current_path());
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("currentDirectory failed: {}", e.what());
  }
}

void File::changeDirectory(String const& dirName) {
  try {
    fs::current_path(fs::path(dirName.utf8()));
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("changeDirectory failed for '{}': {}", dirName, e.what());
  }
}

void File::makeDirectory(String const& dirName) {
  try {
    fs::create_directory(fs::path(dirName.utf8()));
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("makeDirectory failed for '{}': {}", dirName, e.what());
  }
}

void File::makeDirectoryRecursive(String const& dirName) {
  try {
    fs::create_directories(fs::path(dirName.utf8()));
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("makeDirectoryRecursive failed for '{}': {}", dirName, e.what());
  }
}

auto File::dirList(String const& dirName, bool skipDots) -> List<std::pair<String, bool>> {
  try {
    List<std::pair<String, bool>> result;
    for (auto const& entry : fs::directory_iterator(fs::path(dirName.utf8()))) {
      String name = fromU8Path(entry.path().filename());
      if (!skipDots || (name != "." && name != ".."))
        result.append({name, entry.is_directory()});
    }
    return result;
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("dirList failed for '{}': {}", dirName, e.what());
  }
}

auto File::baseName(String const& fileName) -> String {
  return fromU8Path(fs::path(fileName.utf8()).filename());
}

auto File::dirName(String const& fileName) -> String {
  auto p = fs::path(fileName.utf8()).parent_path();
  return p.empty() ? "." : fromU8Path(p);
}

auto File::relativeTo(String const& relativeTo, String const& path) -> String {
  fs::path p(path.utf8());
  if (p.is_absolute())
    return path;
  return fromU8Path(fs::path(relativeTo.utf8()) / p);
}

auto File::fullPath(String const& path) -> String {
  try {
    return fromU8Path(fs::absolute(fs::path(path.utf8())));
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("fullPath failed for '{}': {}", path, e.what());
  }
}

auto File::temporaryFileName() -> String {
  return relativeTo(fromU8Path(fs::temp_directory_path()), strf("starbound.tmpfile.{}", hexEncode(Random::randBytes(16))));
}

auto File::temporaryFile() -> Ptr<File> {
  return open(temporaryFileName(), IOMode::ReadWrite);
}

auto File::ephemeralFile() -> Ptr<File> {
  auto file = std::make_shared<File>(temporaryFileName());
  file->m_ephemeral = true;
  file->open(IOMode::ReadWrite);
  return file;
}

auto File::temporaryDirectory() -> String {
  String dirname = relativeTo(fromU8Path(fs::temp_directory_path()), strf("starbound.tmpdir.{}", hexEncode(Random::randBytes(16))));
  makeDirectory(dirname);
  return dirname;
}

auto File::exists(String const& path) -> bool {
  return fs::exists(fs::path(path.utf8()));
}

auto File::isFile(String const& path) -> bool {
  return fs::is_regular_file(fs::path(path.utf8()));
}

auto File::isDirectory(String const& path) -> bool {
  return fs::is_directory(fs::path(path.utf8()));
}

void File::remove(String const& filename) {
  try {
    fs::remove(fs::path(filename.utf8()));
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("remove failed for '{}': {}", filename, e.what());
  }
}

void File::removeDirectoryRecursive(String const& filename) {
  try {
    fs::remove_all(fs::path(filename.utf8()));
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("removeDirectoryRecursive failed for '{}': {}", filename, e.what());
  }
}

void File::rename(String const& source, String const& target) {
  try {
    fs::rename(fs::path(source.utf8()), fs::path(target.utf8()));
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("rename failed from '{}' to '{}': {}", source, target, e.what());
  }
}

void File::copy(String const& source, String const& target) {
  try {
    fs::copy(fs::path(source.utf8()), fs::path(target.utf8()), fs::copy_options::overwrite_existing);
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("copy failed from '{}' to '{}': {}", source, target, e.what());
  }
}

auto File::fileSize(String const& filename) -> std::int64_t {
  try {
    return fs::file_size(fs::path(filename.utf8()));
  } catch (fs::filesystem_error const& e) {
    throw IOException::format("fileSize failed for '{}': {}", filename, e.what());
  }
}

void File::writeFile(char const* data, std::size_t len, String const& filename) {
  std::ofstream file(fs::path(filename.utf8()), std::ios::binary | std::ios::trunc);
  if (!file)
    throw IOException::format("writeFile: could not open '{}'", filename);
  file.write(data, len);
}

void File::writeFile(ByteArray const& data, String const& filename) {
  writeFile(data.ptr(), data.size(), filename);
}

void File::writeFile(String const& data, String const& filename) {
  writeFile(data.utf8Ptr(), data.utf8Size(), filename);
}

auto File::readFile(String const& filename) -> ByteArray {
  std::ifstream file(fs::path(filename.utf8()), std::ios::binary | std::ios::ate);
  if (!file)
    throw IOException::format("readFile: could not open '{}'", filename);
  auto size = file.tellg();
  file.seekg(0, std::ios::beg);
  ByteArray bytes(size, 0);
  file.read(bytes.ptr(), size);
  return bytes;
}

auto File::readFileString(String const& filename) -> String {
  std::ifstream file(fs::path(filename.utf8()), std::ios::binary | std::ios::ate);
  if (!file)
    throw IOException::format("readFileString: could not open '{}'", filename);
  auto size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::string str(size, '\0');
  file.read(str.data(), size);
  return {std::move(str)};
}

void File::overwriteFileWithRename(char const* data, std::size_t len, String const& filename, String const& newSuffix) {
  String newFile = filename + newSuffix;
  writeFile(data, len, newFile);
  File::rename(newFile, filename);
}

void File::overwriteFileWithRename(ByteArray const& data, String const& filename, String const& newSuffix) {
  overwriteFileWithRename(data.ptr(), data.size(), filename, newSuffix);
}

void File::overwriteFileWithRename(String const& data, String const& filename, String const& newSuffix) {
  overwriteFileWithRename(data.utf8Ptr(), data.utf8Size(), filename, newSuffix);
}

void File::backupFileInSequence(String const& initialFile, String const& targetFile, unsigned maximumBackups, String const& backupExtensionPrefix) {
  for (unsigned i = maximumBackups; i > 0; --i) {
    bool initial = (i == 1);
    String sourceFile = initial ? initialFile : targetFile;
    String curExt = initial ? "" : strf("{}{}", backupExtensionPrefix, i - 1);
    String nextExt = strf("{}{}", backupExtensionPrefix, i);
    if (File::isFile(sourceFile + curExt))
      File::copy(sourceFile + curExt, targetFile + nextExt);
  }
}

void File::backupFileInSequence(String const& targetFile, unsigned maximumBackups, String const& backupExtensionPrefix) {
  backupFileInSequence(targetFile, targetFile, maximumBackups, backupExtensionPrefix);
}

auto File::open(String const& filename, IOMode mode) -> Ptr<File> {
  auto file = std::make_shared<File>(filename);
  file->open(mode);
  return file;
}

File::File() : IODevice(IOMode::Closed) {}

File::File(String filename) : IODevice(IOMode::Closed), m_filename(std::move(filename)) {}

File::~File() {
  close();
}

auto File::fileName() const -> String {
  return m_filename;
}

void File::setFilename(String filename) {
  std::lock_guard lock(m_mutex);
  if (isOpen())
    throw IOException("setFilename called on open File");
  m_filename = std::move(filename);
}

void File::remove() {
  close();
  if (m_filename.empty())
    throw IOException("remove called on unnamed File");
  remove(m_filename);
}

auto File::pos() -> std::int64_t {
  std::lock_guard lock(m_mutex);
  return m_file.tellg();
}

void File::seek(std::int64_t offset, IOSeek seekMode) {
  std::lock_guard lock(m_mutex);
  auto dir = std::ios::beg;
  if (seekMode == IOSeek::Relative)
    dir = std::ios::cur;
  else if (seekMode == IOSeek::End)
    dir = std::ios::end;
  m_file.seekg(offset, dir);
  m_file.seekp(offset, dir);
}

void File::resize(std::int64_t size) {
  std::lock_guard lock(m_mutex);
  close();
  fs::resize_file(fs::path(m_filename.utf8()), size);
  open(mode());
}

auto File::size() -> std::int64_t {
  std::lock_guard lock(m_mutex);
  auto current = m_file.tellg();
  m_file.seekg(0, std::ios::end);
  auto s = m_file.tellg();
  m_file.seekg(current, std::ios::beg);
  return s;
}

auto File::atEnd() -> bool {
  std::lock_guard lock(m_mutex);
  return m_file.eof() || (m_file.tellg() >= size());
}

auto File::read(char* data, std::size_t len) -> std::size_t {
  std::lock_guard lock(m_mutex);
  m_file.read(data, len);
  return m_file.gcount();
}

auto File::write(char const* data, std::size_t len) -> std::size_t {
  std::lock_guard lock(m_mutex);
  m_file.write(data, len);
  return m_file.good() ? len : 0;
}

auto File::readAbsolute(std::int64_t readPosition, char* data, std::size_t len) -> std::size_t {
  std::lock_guard lock(m_mutex);
  auto oldPos = m_file.tellg();
  m_file.seekg(readPosition, std::ios::beg);
  m_file.read(data, len);
  auto read = m_file.gcount();
  m_file.seekg(oldPos, std::ios::beg);
  return read;
}

auto File::writeAbsolute(std::int64_t writePosition, char const* data, std::size_t len) -> std::size_t {
  std::lock_guard lock(m_mutex);
  auto oldPos = m_file.tellp();
  m_file.seekp(writePosition, std::ios::beg);
  m_file.write(data, len);
  m_file.seekp(oldPos, std::ios::beg);
  return m_file.good() ? len : 0;
}

void File::open(IOMode mode) {
  std::lock_guard lock(m_mutex);
  close();
  m_file.open(fs::path(m_filename.utf8()), ioModeToStd(mode));
  if (!m_file)
    throw IOException::format("could not open file '{}'", m_filename);
  setMode(mode);
}

void File::close() {
  std::lock_guard lock(m_mutex);
  if (m_file.is_open())
    m_file.close();
  if (m_ephemeral && !m_filename.empty()) {
    std::error_code ec;
    fs::remove(fs::path(m_filename.utf8()), ec);
  }
  setMode(IOMode::Closed);
}

void File::sync() {
  std::lock_guard lock(m_mutex);
  m_file.flush();
}

auto File::deviceName() const -> String {
  return m_filename.empty() ? "<unnamed file>" : m_filename;
}

auto File::clone() -> Ptr<IODevice> {
  std::lock_guard lock(m_mutex);
  auto cloned = std::make_shared<File>(m_filename);
  if (isOpen()) {
    cloned->open(mode());
    cloned->seek(pos());
  }
  return cloned;
}

}// namespace Star
