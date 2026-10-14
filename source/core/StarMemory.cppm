module;

import std;

namespace Star {

inline constexpr std::size_t NPos = static_cast<std::size_t>(-1);

// Don't want to override global C allocation functions, as our API is
// different.

void* malloc(std::size_t size);
void* realloc(void* ptr, std::size_t size);
void free(void* ptr);
void free(void* ptr, std::size_t size);

}

export module star.memory;

export namespace Star {
  using ::Star::NPos;
  using ::Star::malloc;
  using ::Star::realloc;
  using ::Star::free;
}
