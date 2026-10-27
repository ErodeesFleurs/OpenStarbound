module;

import std;

namespace Star {

inline constexpr std::size_t NPos = static_cast<std::size_t>(-1);

// Engine-owned storage uses mimalloc, independently of third-party C heaps.
// Sized free takes the allocation's requested capacity in bytes, not its used
// length. Realloc failure leaves the original allocation owned by the caller.

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
