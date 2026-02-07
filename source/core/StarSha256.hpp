#pragma once

#include "StarByteArray.hpp"
#include "StarString.hpp"

import std;

namespace Star {

using sha_state = struct sha_state_struct {
  std::array<std::uint32_t, 8> state;
  std::uint32_t length, curlen;
  std::array<std::uint8_t, 64> buf;
};

class Sha256Hasher {
public:
  Sha256Hasher();

  void push(char const* data, size_t length);
  void push(String const& data);
  void push(ByteArray const& data);

  // Produces 32 bytes
  void compute(char* hashDestination);
  auto compute() -> ByteArray;

private:
  bool m_finished;
  sha_state m_state;
};

// Sha256 must, obviously, have 32 bytes available in the destination.
void sha256(char const* source, size_t length, char* hashDestination);

auto sha256(char const* source, size_t length) -> ByteArray;

void sha256(ByteArray const& in, ByteArray& out);
void sha256(String const& in, ByteArray& out);

auto sha256(ByteArray const& in) -> ByteArray;
auto sha256(String const& in) -> ByteArray;

}// namespace Star
