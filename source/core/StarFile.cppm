export module star.file;

export import star.io_device;
import std;
import star.byte_array;

export namespace star {

// All file methods are thread safe.
class file : public star::io_device {
  public:
	// Converts the passed in path to use the platform specific directory
	// separators only.
	[[nodiscard]] static auto convert_dir_separators(std::u8string const& path) -> std::u8string;

	// All static file operations here throw io_exception on error.
	[[nodiscard]] static auto current_directory() -> std::u8string;
	static void change_directory(std::u8string const& dir_name);
	static void make_directory(std::u8string const& dir_name);
	static void make_directory_recursive(std::u8string const& dir_name);

	// List all files or directories under given directory.
	[[nodiscard]] static auto dir_list(std::u8string const& dir_name, bool skip_dots = true)
	  -> std::vector<std::pair<std::u8string, bool>>;

	[[nodiscard]] static auto base_name(std::u8string const& file_name) -> std::u8string;
	[[nodiscard]] static auto dir_name(std::u8string const& file_name) -> std::u8string;

	// Resolve a path relative to another path.
	[[nodiscard]] static auto relative_to(std::u8string_view relative_to, std::u8string_view path) -> std::u8string;

	// Resolve the given possibly relative path into an absolute path.
	[[nodiscard]] static auto full_path(std::u8string const& path) -> std::u8string;

	[[nodiscard]] static auto temporary_file_name() -> std::u8string;

	// Creates and opens a new read_write temporary file.
	static auto temporary_file() -> std::shared_ptr<file>;

	// Creates and opens new read_write temporary file that will be removed on close.
	static auto ephemeral_file() -> std::shared_ptr<file>;

	[[nodiscard]] static auto temporary_directory() -> std::u8string;

	[[nodiscard]] static auto exists(std::u8string const& path) -> bool;

	[[nodiscard]] static auto is_file(std::u8string const& path) -> bool;
	[[nodiscard]] static auto is_directory(std::u8string const& path) -> bool;

	static void remove(std::u8string const& file_name);
	static void remove_directory_recursive(std::u8string const& file_name);

	static void rename(std::u8string const& source, std::u8string const& target);

	static void copy(std::u8string const& source, std::u8string const& target);

	[[nodiscard]] static auto read_file(std::u8string const& file_name) -> byte_array;
	[[nodiscard]] static auto read_file_string(std::u8string const& file_name) -> std::u8string;
	[[nodiscard]] static auto file_size(std::u8string const& file_name) -> std::int64_t;

	static void write_file(std::span<std::byte const> data, std::u8string const& file_name);
	static void write_file(byte_array const& data, std::u8string const& file_name);
	static void write_file(std::u8string const& data, std::u8string const& file_name);

	static void overwrite_file_with_rename(std::span<std::byte const> data, std::u8string const& file_name,
	                                       std::u8string const& new_suffix = u8".new");
	static void overwrite_file_with_rename(byte_array const& data, std::u8string const& file_name,
	                                       std::u8string const& new_suffix = u8".new");
	static void overwrite_file_with_rename(std::u8string const& data, std::u8string const& file_name,
	                                       std::u8string const& new_suffix = u8".new");

	static void backup_file_in_sequence(std::u8string const& initial_file, std::u8string const& target_file,
	                                    unsigned maximum_backups, std::u8string const& backup_extension_prefix = u8".");
	static void backup_file_in_sequence(std::u8string const& target_file, unsigned maximum_backups,
	                                    std::u8string const& backup_extension_prefix = u8".");

	static auto open(std::u8string const& file_name, io_mode mode) -> std::shared_ptr<file>;

	file();
	file(const file&) = delete;
	file(file&&) = delete;
	explicit file(std::u8string file_name);
	~file() override;

	auto operator=(const file&) -> file& = delete;
	auto operator=(file&&) -> file& = delete;

	[[nodiscard]] auto file_name() const -> std::u8string;
	void set_file_name(std::u8string file_name);

	void remove();

	[[nodiscard]] auto pos() -> std::int64_t override;
	void seek(std::int64_t offset, io_seek seek_mode = io_seek::absolute) override;
	void resize(std::int64_t size) override;
	[[nodiscard]] auto size() -> std::int64_t override;
	[[nodiscard]] auto at_end() -> bool override;
	[[nodiscard]] auto read(std::span<std::byte> data) -> std::size_t override;
	[[nodiscard]] auto write(std::span<const std::byte> data) -> std::size_t override;

	[[nodiscard]] auto read_absolute(std::int64_t read_position, std::span<std::byte> data) -> std::size_t override;
	[[nodiscard]] auto write_absolute(std::int64_t write_position, std::span<const std::byte> data)
	  -> std::size_t override;

	void open(io_mode mode) override;
	void close() override;

	void sync() override;

	[[nodiscard]] auto device_name() const -> std::string override;

	[[nodiscard]] auto clone() -> std::shared_ptr<star::io_device> override;

  private:
	void setup_fstream(io_mode mode);

	std::u8string m_file_name;
	std::fstream m_file;
	bool m_ephemeral = false;
	mutable std::recursive_mutex m_mutex;
};

}// namespace star
