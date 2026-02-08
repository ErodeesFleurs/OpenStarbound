#pragma once

#include "StarArray.hpp"

import std;

namespace Star::Curve25519 {

constexpr std::size_t PublicKeySize = 32;
constexpr std::size_t SecretKeySize = 32;
constexpr std::size_t PrivateKeySize = 64;
constexpr std::size_t SignatureSize = 64;

using PublicKey = Array<std::uint8_t, PublicKeySize>;
using SecretKey = Array<std::uint8_t, SecretKeySize>;
using PrivateKey = Array<std::uint8_t, PrivateKeySize>;
using Signature = Array<std::uint8_t, SignatureSize>;

auto publicKey() -> PublicKey const&;
auto sign(void* data, std::size_t len) -> Signature;
auto verify(std::uint8_t const* signature, std::uint8_t const* publicKey, void* data, std::size_t len) -> bool;

}// namespace Star::Curve25519
