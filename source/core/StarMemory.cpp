#include "StarMemory.hpp"

import std;

namespace Star {
auto malloc(std::size_t size) -> void* {
  return std::malloc(size);
}

auto realloc(void* ptr, std::size_t size) -> void* {
  return std::realloc(ptr, size);
}

void free(void* ptr) {
  return std::free(ptr);
}

void free(void* ptr, std::size_t) {
  return std::free(ptr);
}

}// namespace Star
