module star.file;

import std;
import star.encode;
import star.exception;
import star.random;
import star.string;

namespace fs = std::filesystem;

namespace star {

namespace detail {
    auto from_path(fs::path const& p) -> std::u8string { return p.u8string(); }

    auto io_mode_to_std(io_mode mode) -> std::ios_base::openmode {
        std::ios_base::openmode std_mode = std::ios::binary;
        if (static_cast<bool>(mode & io_mode::read)) {
            std_mode |= std::ios::in;
        }
        if (static_cast<bool>(mode & io_mode::write)) {
            std_mode |= std::ios::out;
        }
        if (static_cast<bool>(mode & io_mode::append)) {
            std_mode |= std::ios::app;
        }
        if (static_cast<bool>(mode & io_mode::truncate)) {
            std_mode |= std::ios::trunc;
        }
        return std_mode;
    }
}// namespace detail

auto file::convert_dir_separators(std::u8string const& path) -> std::u8string {
    return detail::from_path(fs::path(path).make_preferred());
}

auto file::current_directory() -> std::u8string {
    try {
        return detail::from_path(fs::current_path());
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("current_directory failed: {}", e.what());
    }
}

void file::change_directory(std::u8string const& dir_name) {
    try {
        fs::current_path(fs::path(dir_name));
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("change_directory failed for '{}': {}", to_sv(dir_name),
                                   e.what());
    }
}

void file::make_directory(std::u8string const& dir_name) {
    try {
        fs::create_directory(fs::path(dir_name));
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("make_directory failed for '{}': {}", to_sv(dir_name), e.what());
    }
}

void file::make_directory_recursive(std::u8string const& dir_name) {
    try {
        fs::create_directories(fs::path(dir_name));
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("make_directory_recursive failed for '{}': {}", to_sv(dir_name),
                                   e.what());
    }
}

auto file::dir_list(std::u8string const& dir_name, bool skip_dots)
  -> std::vector<std::pair<std::u8string, bool>> {
    try {
        std::vector<std::pair<std::u8string, bool>> result;
        for (auto const& entry : fs::directory_iterator(fs::path(dir_name))) {
            std::u8string name = detail::from_path(entry.path().filename());
            if (!skip_dots || (name != u8"." && name != u8"..")) {
                result.emplace_back(name, entry.is_directory());
            }
        }
        return result;
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("dir_list failed for '{}': {}", to_sv(dir_name), e.what());
    }
}

auto file::base_name(std::u8string const& file_name) -> std::u8string {
    return detail::from_path(fs::path(file_name).filename());
}

auto file::dir_name(std::u8string const& file_name) -> std::u8string {
    auto p = fs::path(file_name).parent_path();
    return p.empty() ? u8"." : detail::from_path(p);
}

auto file::relative_to(std::u8string_view relative_to, std::u8string_view path) -> std::u8string {
    fs::path p(path);
    if (p.is_absolute()) {
        return std::u8string(path);
    }
    return detail::from_path(fs::path(relative_to) / p);
}

auto file::full_path(std::u8string const& path) -> std::u8string {
    try {
        return detail::from_path(fs::absolute(fs::path(path)));
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("full_path failed for '{}': {}", to_sv(path), e.what());
    }
}

auto file::temporary_file_name() -> std::u8string {
    auto temp_dir = fs::temp_directory_path();
    auto bytes = random::rand_bytes(16);
    auto random_suffix = hex_encode(std::as_bytes(bytes.span()));
    auto name = std::format("starbound.tmpfile.{}", to_sv(random_suffix));
    return detail::from_path(temp_dir / name);
}

auto file::temporary_file() -> std::shared_ptr<file> {
    return open(temporary_file_name(), io_mode::read_write);
}

auto file::ephemeral_file() -> std::shared_ptr<file> {
    auto f = std::make_shared<file>(temporary_file_name());
    f->m_ephemeral = true;
    f->open(io_mode::read_write);
    return f;
}

auto file::temporary_directory() -> std::u8string {
    auto bytes = random::rand_bytes(16);
    auto random_suffix = hex_encode(std::as_bytes(bytes.span()));
    auto file_name = std::format("starbound.tmpdir.{}", to_sv(random_suffix));
    std::u8string dirname =
      relative_to(detail::from_path(fs::temp_directory_path()), to_u8sv(file_name));
    make_directory(dirname);
    return dirname;
}

auto file::exists(std::u8string const& path) -> bool { return fs::exists(fs::path(path)); }

auto file::is_file(std::u8string const& path) -> bool {
    return fs::is_regular_file(fs::path(path));
}

auto file::is_directory(std::u8string const& path) -> bool {
    return fs::is_directory(fs::path(path));
}

void file::remove(std::u8string const& file_name) {
    try {
        fs::remove(fs::path(file_name));
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("remove failed for '{}': {}", to_sv(file_name), e.what());
    }
}

void file::remove_directory_recursive(std::u8string const& file_name) {
    try {
        fs::remove_all(fs::path(file_name));
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("remove_directory_recursive failed for '{}': {}",
                                   to_sv(file_name), e.what());
    }
}

void file::rename(std::u8string const& source, std::u8string const& target) {
    try {
        fs::rename(fs::path(source), fs::path(target));
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("rename failed from '{}' to '{}': {}", to_sv(source),
                                   to_sv(target), e.what());
    }
}

void file::copy(std::u8string const& source, std::u8string const& target) {
    try {
        fs::copy(fs::path(source), fs::path(target), fs::copy_options::overwrite_existing);
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("copy failed from '{}' to '{}': {}", to_sv(source),
                                   to_sv(target), e.what());
    }
}

auto file::file_size(std::u8string const& file_name) -> std::int64_t {
    try {
        return static_cast<std::int64_t>(fs::file_size(fs::path(file_name)));
    } catch (fs::filesystem_error const& e) {
        throw io_exception::format("file_size failed for '{}': {}", to_sv(file_name), e.what());
    }
}

void file::write_file(std::span<const std::byte> data, std::u8string const& file_name) {
    std::ofstream f(fs::path(file_name), std::ios::binary | std::ios::trunc);
    if (!f) {
        throw io_exception::format("write_file: could not open '{}'", to_sv(file_name));
    }
    f.write(reinterpret_cast<char const*>(data.data()), static_cast<std::uint32_t>(data.size()));
}

void file::write_file(byte_array const& data, std::u8string const& file_name) {
    write_file(std::as_bytes(data.span()), file_name);
}

void file::write_file(std::u8string const& data, std::u8string const& file_name) {
    write_file(
      std::span<std::byte const>{reinterpret_cast<std::byte const*>(data.c_str()), data.size()},
      file_name);
}

auto file::read_file(std::u8string const& file_name) -> byte_array {
    std::ifstream f(fs::path(file_name), std::ios::binary | std::ios::ate);
    if (!f) {
        throw io_exception::format("read_file: could not open '{}'", to_sv(file_name));
    }
    auto size = f.tellg();
    f.seekg(0, std::ios::beg);
    byte_array bytes(std::string_view(nullptr, size));
    f.read(bytes.data(), size);
    return bytes;
}

auto file::read_file_string(std::u8string const& file_name) -> std::u8string {
    std::ifstream f(fs::path(file_name), std::ios::binary | std::ios::ate);
    if (!f) {
        throw io_exception::format("read_file_string: could not open '{}'", file_name);
    }
    auto size = f.tellg();
    f.seekg(0, std::ios::beg);
    std::string str(size, '\0');
    f.read(str.data(), size);
    return string_cast<std::u8string>(str);
}

void file::overwrite_file_with_rename(std::span<std::byte const> data,
                                      std::u8string const& file_name,
                                      std::u8string const& new_suffix) {
    std::u8string new_file = file_name + new_suffix;
    write_file(data, new_file);
    file::rename(new_file, file_name);
}

void file::overwrite_file_with_rename(byte_array const& data, std::u8string const& file_name,
                                      std::u8string const& new_suffix) {
    overwrite_file_with_rename(std::as_bytes(data.span()), file_name, new_suffix);
}

void file::overwrite_file_with_rename(std::u8string const& data, std::u8string const& file_name,
                                      std::u8string const& new_suffix) {
    overwrite_file_with_rename(
      std::span<std::byte const>{reinterpret_cast<std::byte const*>(data.c_str()), data.size()},
      file_name, new_suffix);
}

void file::backup_file_in_sequence(std::u8string const& initial_file, std::u8string const& target_file,
                                   unsigned maximum_backups,
                                   std::u8string const& backup_extension_prefix) {
    for (unsigned i = maximum_backups; i > 0; --i) {
        bool initial = (i == 1);
        std::u8string source_file = initial ? initial_file : target_file;
        std::u8string cur_ext = string_cast<std::u8string>(initial ? "" : std::format("{}{}", backup_extension_prefix, i - 1));
        std::u8string next_ext = string_cast<std::u8string>(std::format("{}{}", backup_extension_prefix, i));
        if (file::is_file(source_file + cur_ext)) {
            file::copy(source_file + cur_ext, target_file + next_ext);
        }
    }
}

void file::backup_file_in_sequence(std::u8string const& target_file, unsigned maximum_backups,
                                   std::u8string const& backup_extension_prefix) {
    backup_file_in_sequence(target_file, target_file, maximum_backups, backup_extension_prefix);
}

auto file::open(std::u8string const& file_name, io_mode mode) -> std::shared_ptr<file> {
    auto f = std::make_shared<file>(file_name);
    f->open(mode);
    return f;
}

file::file() : io_device(io_mode::closed) {}

file::file(std::u8string file_name) : io_device(io_mode::closed), m_file_name(std::move(file_name)) {}

file::~file() { close(); }

auto file::file_name() const -> std::u8string { return m_file_name; }

void file::set_file_name(std::u8string file_name) {
    std::lock_guard lock(m_mutex);
    if (is_open()) {
        throw io_exception("set_file_name called on open file");
    }
    m_file_name = std::move(file_name);
}

void file::remove() {
    close();
    if (m_file_name.empty()) {
        throw io_exception("remove called on unnamed file");
    }
    remove(m_file_name);
}

auto file::pos() -> std::int64_t {
    std::lock_guard lock(m_mutex);
    return m_file.tellg();
}

void file::seek(std::int64_t offset, io_seek seek_mode) {
    std::lock_guard lock(m_mutex);
    auto dir = std::ios::beg;
    if (seek_mode == io_seek::relative) {
        dir = std::ios::cur;
    } else if (seek_mode == io_seek::end) {
        dir = std::ios::end;
    }
    m_file.clear();
    m_file.seekg(offset, dir);
    m_file.seekp(offset, dir);
}

void file::resize(std::int64_t size) {
    std::lock_guard lock(m_mutex);
    close();
    fs::resize_file(fs::path(m_file_name), size);
    open(mode());
}

auto file::size() -> std::int64_t {
    std::lock_guard lock(m_mutex);
    auto current = m_file.tellg();
    m_file.seekg(0, std::ios::end);
    auto s = m_file.tellg();
    m_file.seekg(current, std::ios::beg);
    return s;
}

auto file::at_end() -> bool {
    std::lock_guard lock(m_mutex);
    return m_file.eof() || (m_file.tellg() >= size());
}

auto file::read(std::span<std::byte> data) -> std::size_t {
    std::lock_guard lock(m_mutex);
    m_file.read(reinterpret_cast<char*>(data.data()), static_cast<std::uint32_t>(data.size()));
    return m_file.gcount();
}

auto file::write(std::span<const std::byte> data) -> std::size_t {
    std::lock_guard lock(m_mutex);
    m_file.write(reinterpret_cast<const char*>(data.data()),
                 static_cast<std::uint32_t>(data.size()));
    return m_file.good() ? data.size() : 0;
}

auto file::read_absolute(std::int64_t read_position, std::span<std::byte> data) -> std::size_t {
    std::lock_guard lock(m_mutex);
    auto old_pos = m_file.tellg();
    m_file.clear();
    m_file.seekg(old_pos, std::ios::beg);
    m_file.seekg(read_position, std::ios::beg);
    return read(data);
}

auto file::write_absolute(std::int64_t write_position, std::span<const std::byte> data)
  -> std::size_t {
    std::lock_guard lock(m_mutex);
    auto old_pos = m_file.tellp();
    m_file.clear();
    m_file.seekp(old_pos, std::ios::beg);
    m_file.seekp(write_position, std::ios::beg);
    return write(data);
}

void file::open(io_mode mode) {
    std::lock_guard lock(m_mutex);
    close();
    m_file.open(fs::path(m_file_name), detail::io_mode_to_std(mode));
    if (!m_file) {
        throw io_exception::format("could not open file '{}'", m_file_name);
    }
    set_mode(mode);
}

void file::close() {
    std::lock_guard lock(m_mutex);
    if (m_file.is_open()) {
        m_file.close();
    }
    if (m_ephemeral && !m_file_name.empty()) {
        std::error_code ec;
        fs::remove(fs::path(m_file_name), ec);
    }
    set_mode(io_mode::closed);
}

void file::sync() {
    std::lock_guard lock(m_mutex);
    m_file.flush();
}

auto file::device_name() const -> std::string {
    return m_file_name.empty() ? "<unnamed file>" : string_cast<std::string>(m_file_name);
}

auto file::clone() -> std::shared_ptr<io_device> {
    std::lock_guard lock(m_mutex);
    auto cloned = std::make_shared<file>(m_file_name);
    if (is_open()) {
        cloned->open(mode());
        cloned->seek(pos());
    }
    return cloned;
}

}// namespace star
