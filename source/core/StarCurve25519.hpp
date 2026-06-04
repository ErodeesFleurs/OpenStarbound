#pragma once

#include "StarEncode.hpp"
#include "StarByteArray.hpp"
#include "StarArray.hpp"

namespace Star::Curve25519 {

constexpr size_t PublicKeySize = 32;
constexpr size_t SecretKeySize = 32;
constexpr size_t PrivateKeySize = 64;
constexpr size_t SignatureSize = 64;

using PublicKey = Array<uint8_t, PublicKeySize>;
using SecretKey = Array<uint8_t, SecretKeySize>;
using PrivateKey = Array<uint8_t, PrivateKeySize>;
using Signature = Array<uint8_t, SignatureSize>;

PublicKey const& publicKey();
Signature sign(void const* data, size_t len);
bool verify(uint8_t const* signature, uint8_t const* publicKey, void const* data, size_t len);

}
