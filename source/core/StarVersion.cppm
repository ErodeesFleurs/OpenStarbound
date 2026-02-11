export module star.version;

import std;

export namespace star {

extern constexpr std::string_view open_star_version_string = "0.1.14.1";
extern constexpr std::string_view star_version_string = "1.4.4";
extern constexpr std::string_view star_source_identifier_string = "";
extern constexpr std::string_view star_architecture_string = "linux";

using version_number = std::uint32_t;

}// namespace star
