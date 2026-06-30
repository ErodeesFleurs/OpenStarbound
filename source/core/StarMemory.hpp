#pragma once

#include <new>

#include "StarConfig.hpp"

namespace Star {

// Don't want to override global C allocation functions, as our API is
// different.

[[nodiscard]] void* malloc(size_t size);
[[nodiscard]] void* realloc(void* ptr, size_t size);
void free(void* ptr);
void free(void* ptr, size_t size);

}
