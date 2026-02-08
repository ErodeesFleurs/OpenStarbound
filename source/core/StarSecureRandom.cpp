#include "StarSecureRandom.hpp"
#include "StarException.hpp"
#include "StarFormat.hpp"

import std;

namespace Star {

auto secureRandomBytes(std::size_t size) -> ByteArray {
  if (size == 0)
    return {};

  ByteArray bytes(size, 0);
  try {
    std::random_device rd;

    char* data = bytes.ptr();
    std::size_t wordSize = sizeof(std::random_device::result_type);
    std::size_t words = size / wordSize;
    std::size_t rem = size % wordSize;

    // Fill full words
    for (std::size_t i = 0; i < words; ++i) {
      std::random_device::result_type val = rd();
      std::memcpy(data + i * wordSize, &val, wordSize);
    }

    // Fill remaining bytes
    if (rem > 0) {
      std::random_device::result_type val = rd();
      std::memcpy(data + words * wordSize, &val, rem);
    }
  } catch (std::exception const& e) {
    // std::random_device can throw if no entropy source is available
    throw StarException(strf("Could not generate secure random bytes: {}", e.what()));
  }

  return bytes;
}

}// namespace Star
