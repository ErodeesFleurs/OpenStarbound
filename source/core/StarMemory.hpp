#pragma once

import std;

namespace Star {

// Don't want to override global C allocation functions, as our API is
// different.

auto malloc(std::size_t size) -> void*;
auto realloc(void* ptr, std::size_t size) -> void*;
void free(void* ptr);
void free(void* ptr, std::size_t size);

}
