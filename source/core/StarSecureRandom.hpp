#pragma once

#include "StarByteArray.hpp"

import std;

namespace Star {

// Generate cryptographically secure random numbers for usage in password salts
// and such using OS facilities
auto secureRandomBytes(std::size_t size) -> ByteArray;

}// namespace Star
