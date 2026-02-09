#include "StarCurve25519.hpp"
#include "StarRandom.hpp"

#include "curve25519/include/ed25519_signature.h"

import std;

namespace Star::Curve25519 {

struct KeySet {
  PrivateKey privateKey;
  PublicKey publicKey;

  KeySet() {
    SecretKey secret;
    Random::randBytes(SecretKeySize).copyTo((char*)secret.data());

    secret[0] &= 248;
    secret[31] &= 127;
    secret[31] |= 64;

    ed25519_CreateKeyPair(publicKey.data(), privateKey.data(), nullptr, secret.data());
  }
};

static auto staticKeys() -> KeySet const& {
  static KeySet keys;

  return keys;
}

auto privateKey() -> PrivateKey const& { return staticKeys().privateKey; }

auto sign(void* data, std::size_t len) -> Signature {
  Signature signature;
  ed25519_SignMessage(signature.data(), privateKey().data(), nullptr, (unsigned char*)data, len);
  return signature;
}

auto verify(std::uint8_t const* signature, std::uint8_t const* publicKey, void* data, std::size_t len) -> bool {
  return ed25519_VerifySignature(signature, publicKey, (unsigned char*)data, len);
}

auto publicKey() -> PublicKey const& { return staticKeys().publicKey; }

}// namespace Star::Curve25519
