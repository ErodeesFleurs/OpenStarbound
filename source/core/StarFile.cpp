#include "StarFile.hpp"

#include "StarEncode.hpp"
#include "StarRandom.hpp"

import std;

namespace fs = std::filesystem;

namespace Star {

namespace detail {
	auto from_path(fs::path const& p) -> std::string {
		auto u8 = p.u8string();
		return {(char const*)u8.data(), u8.size()};
	}

	auto io_mode_to_std(IOMode mode) -> std::ios_base::openmode {
		std::ios_base::openmode stdMode = std::ios::binary;
		if (mode & IOMode::Read) {
			stdMode |= std::ios::in;
		}
		if (mode & IOMode::Write) {
			stdMode |= std::ios::out;
		}
		if (mode & IOMode::Append) {
			stdMode |= std::ios::app;
		}
		if (mode & IOMode::Truncate) {
			stdMode |= std::ios::trunc;
		}
		return stdMode;
	}
}// namespace detail

auto File::convertDirSeparators(std::string const& path) -> std::string {
	return detail::from_path(fs::path(path).make_preferred());
}

auto File::currentDirectory() -> std::string {
	try {
		return detail::from_path(fs::current_path());
	} catch (fs::filesystem_error const& e) { throw IOException::format("currentDirectory failed: {}", e.what()); }
}

void File::changeDirectory(std::string const& dirName) {
	try {
		fs::current_path(fs::path(dirName));
	} catch (fs::filesystem_error const& e) {
		throw IOException::format("changeDirectory failed for '{}': {}", dirName, e.what());
	}
}

void File::makeDirectory(std::string const& dirName) {
	try {
		fs::create_directory(fs::path(dirName));
	} catch (fs::filesystem_error const& e) {
		throw IOException::format("makeDirectory failed for '{}': {}", dirName, e.what());
	}
}

void File::makeDirectoryRecursive(std::string const& dirName) {
	try {
		fs::create_directories(fs::path(dirName));
	} catch (fs::filesystem_error const& e) {
		throw IOException::format("makeDirectoryRecursive failed for '{}': {}", dirName, e.what());
	}
}

auto File::dirList(std::string const& dirName, bool skipDots) -> std::vector<std::pair<std::string, bool>> {
	try {
		std::vector<std::pair<std::string, bool>> result;
		for (auto const& entry : fs::directory_iterator(fs::path(dirName))) {
			std::string name = detail::from_path(entry.path().filename());
			if (!skipDots || (name != "." && name != "..")) {
				result.emplace_back(name, entry.is_directory());
			}
		}
		return result;
	} catch (fs::filesystem_error const& e) {
		throw IOException::format("dirList failed for '{}': {}", dirName, e.what());
	}
}

auto File::baseName(std::string const& fileName) -> std::string {
	return detail::from_path(fs::path(fileName).filename());
}

auto File::dirName(std::string const& fileName) -> std::string {
	auto p = fs::path(fileName).parent_path();
	return p.empty() ? "." : detail::from_path(p);
}

auto File::relativeTo(std::string const& relativeTo,// NOLINT(bugprone-easily-swappable-parameters)
                      std::string const& path) -> std::string {
	fs::path p(path);
	if (p.is_absolute()) {
		return path;
	}
	return detail::from_path(fs::path(relativeTo) / p);
}

auto File::fullPath(std::string const& path) -> std::string {
	try {
		return detail::from_path(fs::absolute(fs::path(path)));
	} catch (fs::filesystem_error const& e) {
		throw IOException::format("fullPath failed for '{}': {}", path, e.what());
	}
}

auto File::temporaryFileName() -> std::string {
	auto tempDir = fs::temp_directory_path();
	auto bytes = Random::randBytes(16);
	auto randomSuffix = hex_encode(std::as_bytes(bytes.span()));
	auto name = std::format("starbound.tmpfile.{}", randomSuffix);
	return detail::from_path(tempDir / name);
}

auto File::temporaryFile() -> std::shared_ptr<File> { return open(temporaryFileName(), IOMode::ReadWrite); }

auto File::ephemeralFile() -> std::shared_ptr<File> {
	auto file = std::make_shared<File>(temporaryFileName());
	file->m_ephemeral = true;
	file->open(IOMode::ReadWrite);
	return file;
}

auto File::temporaryDirectory() -> std::string {
	auto bytes = Random::randBytes(16);
	auto randomSuffix = hex_encode(std::as_bytes(bytes.span()));
	std::string dirname =
	  relativeTo(detail::from_path(fs::temp_directory_path()), std::format("starbound.tmpdir.{}", randomSuffix));
	makeDirectory(dirname);
	return dirname;
}

auto File::exists(std::string const& path) -> bool { return fs::exists(fs::path(path)); }

auto File::isFile(std::string const& path) -> bool { return fs::is_regular_file(fs::path(path)); }

auto File::isDirectory(std::string const& path) -> bool { return fs::is_directory(fs::path(path)); }

void File::remove(std::string const& filename) {
	try {
		fs::remove(fs::path(filename));
	} catch (fs::filesystem_error const& e) {
		throw IOException::format("remove failed for '{}': {}", filename, e.what());
	}
}

void File::removeDirectoryRecursive(std::string const& filename) {
	try {
		fs::remove_all(fs::path(filename));
	} catch (fs::filesystem_error const& e) {
		throw IOException::format("removeDirectoryRecursive failed for '{}': {}", filename, e.what());
	}
}

void File::rename(std::string const& source, std::string const& target) {
	try {
		fs::rename(fs::path(source), fs::path(target));
	} catch (fs::filesystem_error const& e) {
		throw IOException::format("rename failed from '{}' to '{}': {}", source, target, e.what());
	}
}

void File::copy(std::string const& source, std::string const& target) {
	try {
		fs::copy(fs::path(source), fs::path(target), fs::copy_options::overwrite_existing);
	} catch (fs::filesystem_error const& e) {
		throw IOException::format("copy failed from '{}' to '{}': {}", source, target, e.what());
	}
}

auto File::fileSize(std::string const& filename) -> std::int64_t {
	try {
		return static_cast<std::int64_t>(fs::file_size(fs::path(filename)));
	} catch (fs::filesystem_error const& e) {
		throw IOException::format("fileSize failed for '{}': {}", filename, e.what());
	}
}

void File::writeFile(std::span<const std::byte> data, std::string const& filename) {
	std::ofstream file(fs::path(filename), std::ios::binary | std::ios::trunc);
	if (!file) {
		throw IOException::format("writeFile: could not open '{}'", filename);
	}
	file.write(reinterpret_cast<char const*>(data.data()), static_cast<std::uint32_t>(data.size()));
}

void File::writeFile(ByteArray const& data, std::string const& filename) {
	writeFile(std::as_bytes(data.span()), filename);
}

void File::writeFile(std::string const& data, std::string const& filename) {
	writeFile(std::span<std::byte const>{reinterpret_cast<std::byte const*>(data.c_str()), data.size()}, filename);
}

auto File::readFile(std::string const& filename) -> ByteArray {
	std::ifstream file(fs::path(filename), std::ios::binary | std::ios::ate);
	if (!file) {
		throw IOException::format("readFile: could not open '{}'", filename);
	}
	auto size = file.tellg();
	file.seekg(0, std::ios::beg);
	ByteArray bytes(std::string_view(nullptr, size));
	file.read(bytes.data(), size);
	return bytes;
}

auto File::readFileString(std::string const& filename) -> std::string {
	std::ifstream file(fs::path(filename), std::ios::binary | std::ios::ate);
	if (!file) {
		throw IOException::format("readFilestd::string: could not open '{}'", filename);
	}
	auto size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::string str(size, '\0');
	file.read(str.data(), size);
	return {str};
}

void File::overwriteFileWithRename(std::span<std::byte const> data, std::string const& filename,
                                   std::string const& newSuffix) {
	std::string newFile = filename + newSuffix;
	writeFile(data, newFile);
	File::rename(newFile, filename);
}

void File::overwriteFileWithRename(ByteArray const& data, std::string const& filename, std::string const& newSuffix) {
	overwriteFileWithRename(std::as_bytes(data.span()), filename, newSuffix);
}

void File::overwriteFileWithRename(std::string const& data, std::string const& filename, std::string const& newSuffix) {
	overwriteFileWithRename(std::span<std::byte const>{reinterpret_cast<std::byte const*>(data.c_str()), data.size()},
	                        filename, newSuffix);
}

void File::backupFileInSequence(std::string const& initialFile, std::string const& targetFile, unsigned maximumBackups,
                                std::string const& backupExtensionPrefix) {
	for (unsigned i = maximumBackups; i > 0; --i) {
		bool initial = (i == 1);
		std::string sourceFile = initial ? initialFile : targetFile;
		std::string curExt = initial ? "" : std::format("{}{}", backupExtensionPrefix, i - 1);
		std::string nextExt = std::format("{}{}", backupExtensionPrefix, i);
		if (File::isFile(sourceFile + curExt)) {
			File::copy(sourceFile + curExt, targetFile + nextExt);
		}
	}
}

void File::backupFileInSequence(std::string const& targetFile, unsigned maximumBackups,
                                std::string const& backupExtensionPrefix) {
	backupFileInSequence(targetFile, targetFile, maximumBackups, backupExtensionPrefix);
}

auto File::open(std::string const& filename, IOMode mode) -> std::shared_ptr<File> {
	auto file = std::make_shared<File>(filename);
	file->open(mode);
	return file;
}

File::File() : IODevice(IOMode::Closed) {}

File::File(std::string filename) : IODevice(IOMode::Closed), m_filename(std::move(filename)) {}

File::~File() { close(); }

auto File::fileName() const -> std::string { return m_filename; }

void File::setFilename(std::string filename) {
	std::lock_guard lock(m_mutex);
	if (isOpen()) {
		throw IOException("setFilename called on open File");
	}
	m_filename = std::move(filename);
}

void File::remove() {
	close();
	if (m_filename.empty()) {
		throw IOException("remove called on unnamed File");
	}
	remove(m_filename);
}

auto File::pos() -> std::int64_t {
	std::lock_guard lock(m_mutex);
	return m_file.tellg();
}

void File::seek(std::int64_t offset, IOSeek seekMode) {
	std::lock_guard lock(m_mutex);
	auto dir = std::ios::beg;
	if (seekMode == IOSeek::Relative) {
		dir = std::ios::cur;
	} else if (seekMode == IOSeek::End) {
		dir = std::ios::end;
	}
	m_file.clear();
	m_file.seekg(offset, dir);
	m_file.seekp(offset, dir);
}

void File::resize(std::int64_t size) {
	std::lock_guard lock(m_mutex);
	close();
	fs::resize_file(fs::path(m_filename), size);
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

auto File::read(std::span<std::byte> data) -> std::size_t {
	std::lock_guard lock(m_mutex);
	m_file.read(reinterpret_cast<char*>(data.data()), static_cast<std::uint32_t>(data.size()));
	return m_file.gcount();
}

auto File::write(std::span<const std::byte> data) -> std::size_t {
	std::lock_guard lock(m_mutex);
	m_file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::uint32_t>(data.size()));
	return m_file.good() ? data.size() : 0;
}

auto File::readAbsolute(std::int64_t readPosition, std::span<std::byte> data) -> std::size_t {
	std::lock_guard lock(m_mutex);
	auto oldPos = m_file.tellg();
	m_file.clear();
	m_file.seekg(oldPos, std::ios::beg);
	m_file.seekg(readPosition, std::ios::beg);
	return read(data);
}

auto File::writeAbsolute(std::int64_t writePosition, std::span<const std::byte> data) -> std::size_t {
	std::lock_guard lock(m_mutex);
	auto oldPos = m_file.tellp();
	m_file.clear();
	m_file.seekp(oldPos, std::ios::beg);
	m_file.seekp(writePosition, std::ios::beg);
	return write(data);
}

void File::open(IOMode mode) {
	std::lock_guard lock(m_mutex);
	close();
	m_file.open(fs::path(m_filename), detail::io_mode_to_std(mode));
	if (!m_file) {
		throw IOException::format("could not open file '{}'", m_filename);
	}
	setMode(mode);
}

void File::close() {
	std::lock_guard lock(m_mutex);
	if (m_file.is_open()) {
		m_file.close();
	}
	if (m_ephemeral && !m_filename.empty()) {
		std::error_code ec;
		fs::remove(fs::path(m_filename), ec);
	}
	setMode(IOMode::Closed);
}

void File::sync() {
	std::lock_guard lock(m_mutex);
	m_file.flush();
}

auto File::deviceName() const -> std::string { return m_filename.empty() ? "<unnamed file>" : m_filename; }

auto File::clone() -> std::shared_ptr<IODevice> {
	std::lock_guard lock(m_mutex);
	auto cloned = std::make_shared<File>(m_filename);
	if (isOpen()) {
		cloned->open(mode());
		cloned->seek(pos());
	}
	return cloned;
}

}// namespace Star
