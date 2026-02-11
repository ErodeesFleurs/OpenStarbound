#pragma once

#include "StarIODevice.hpp"

import std;

namespace Star {

// All file methods are thread safe.
class File : public IODevice {
  public:
	// Converts the passed in path to use the platform specific directory
	// separators only.
	[[nodiscard]] static auto convertDirSeparators(std::string const& path) -> std::string;

	// All static file operations here throw IOException on error.
	[[nodiscard]] static auto currentDirectory() -> std::string;
	static void changeDirectory(std::string const& dirName);
	static void makeDirectory(std::string const& dirName);
	static void makeDirectoryRecursive(std::string const& dirName);

	// List all files or directories under given directory.
	[[nodiscard]] static auto dirList(std::string const& dirName, bool skipDots = true)
	  -> std::vector<std::pair<std::string, bool>>;

	[[nodiscard]] static auto baseName(std::string const& fileName) -> std::string;
	[[nodiscard]] static auto dirName(std::string const& fileName) -> std::string;

	// Resolve a path relative to another path.
	[[nodiscard]] static auto relativeTo(std::string const& relativeTo, std::string const& path) -> std::string;

	// Resolve the given possibly relative path into an absolute path.
	[[nodiscard]] static auto fullPath(std::string const& path) -> std::string;

	[[nodiscard]] static auto temporaryFileName() -> std::string;

	// Creates and opens a new ReadWrite temporary file.
	static auto temporaryFile() -> std::shared_ptr<File>;

	// Creates and opens new ReadWrite temporary file that will be removed on close.
	static auto ephemeralFile() -> std::shared_ptr<File>;

	[[nodiscard]] static auto temporaryDirectory() -> std::string;

	[[nodiscard]] static auto exists(std::string const& path) -> bool;

	[[nodiscard]] static auto isFile(std::string const& path) -> bool;
	[[nodiscard]] static auto isDirectory(std::string const& path) -> bool;

	static void remove(std::string const& filename);
	static void removeDirectoryRecursive(std::string const& filename);

	static void rename(std::string const& source, std::string const& target);

	static void copy(std::string const& source, std::string const& target);

	[[nodiscard]] static auto readFile(std::string const& filename) -> ByteArray;
	[[nodiscard]] static auto readFileString(std::string const& filename) -> std::string;
	[[nodiscard]] static auto fileSize(std::string const& filename) -> std::int64_t;

	static void writeFile(std::span<std::byte const> data, std::string const& filename);
	static void writeFile(ByteArray const& data, std::string const& filename);
	static void writeFile(std::string const& data, std::string const& filename);

	static void overwriteFileWithRename(std::span<std::byte const> data, std::string const& filename,
	                                    std::string const& newSuffix = ".new");
	static void overwriteFileWithRename(ByteArray const& data, std::string const& filename,
	                                    std::string const& newSuffix = ".new");
	static void overwriteFileWithRename(std::string const& data, std::string const& filename,
	                                    std::string const& newSuffix = ".new");

	static void backupFileInSequence(std::string const& initialFile, std::string const& targetFile,
	                                 unsigned maximumBackups, std::string const& backupExtensionPrefix = ".");
	static void backupFileInSequence(std::string const& targetFile, unsigned maximumBackups,
	                                 std::string const& backupExtensionPrefix = ".");

	static auto open(std::string const& filename, IOMode mode) -> std::shared_ptr<File>;

	File();
	File(const File&) = delete;
	File(File&&) = delete;
	explicit File(std::string filename);
	~File() override;

	auto operator=(const File&) -> File& = delete;
	auto operator=(File&&) -> File& = delete;

	[[nodiscard]] auto fileName() const -> std::string;
	void setFilename(std::string filename);

	void remove();

	[[nodiscard]] auto pos() -> std::int64_t override;
	void seek(std::int64_t pos, IOSeek seek = IOSeek::Absolute) override;
	void resize(std::int64_t size) override;
	[[nodiscard]] auto size() -> std::int64_t override;
	[[nodiscard]] auto atEnd() -> bool override;
	[[nodiscard]] auto read(std::span<std::byte> data) -> std::size_t override;
	[[nodiscard]] auto write(std::span<const std::byte> data) -> std::size_t override;

	[[nodiscard]] auto readAbsolute(std::int64_t readPosition, std::span<std::byte> data) -> std::size_t override;
	[[nodiscard]] auto writeAbsolute(std::int64_t writePosition, std::span<const std::byte> data)
	  -> std::size_t override;

	void open(IOMode mode) override;
	void close() override;

	void sync() override;

	[[nodiscard]] auto deviceName() const -> std::string override;

	[[nodiscard]] auto clone() -> std::shared_ptr<IODevice> override;

  private:
	void setupFstream(IOMode mode);

	std::string m_filename;
	std::fstream m_file;
	bool m_ephemeral = false;
	mutable std::recursive_mutex m_mutex;
};

}// namespace Star
