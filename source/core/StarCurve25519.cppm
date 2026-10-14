module;
#include <stddef.h>
#include "curve25519/include/curve25519_dh.h"
#include "curve25519/include/ed25519_signature.h"
import std;
import star.random;
import star.logging;


// The native C declarations are parsed in the global module fragment.
import star.string;
import star.byte_array;
import star.encode;
import star.array;

export module star.curve25519;

export namespace Star {
namespace Curve25519 {

constexpr std::size_t PublicKeySize = 32;
constexpr std::size_t SecretKeySize = 32;
constexpr std::size_t PrivateKeySize = 64;
constexpr std::size_t SignatureSize = 64;

typedef Array<std::uint8_t, PublicKeySize> PublicKey;
typedef Array<std::uint8_t, SecretKeySize> SecretKey;
typedef Array<std::uint8_t, PrivateKeySize> PrivateKey;
typedef Array<std::uint8_t, SignatureSize> Signature;

PublicKey const& publicKey();
Signature sign(void* data, std::size_t len);
bool verify(std::uint8_t const* signature, std::uint8_t const* publicKey, void* data, std::size_t len);


}
}

// The definitions are not exported: staticKeys has internal linkage, which is
// not allowed in an exported namespace.
namespace Star {
namespace Curve25519 {
struct KeySet {
  PrivateKey privateKey;
  PublicKey publicKey;

  KeySet() {
    SecretKey secret;
    Random::randBytes(SecretKeySize).copyTo((char*)secret.data());

    secret[0]  &= 248;
    secret[31] &= 127;
    secret[31] |= 64;

    ed25519_CreateKeyPair(publicKey.data(), privateKey.data(), nullptr, secret.data());
  }
};

static KeySet const& staticKeys() {
  static KeySet keys;

  return keys;
}

PrivateKey const& privateKey() { return staticKeys().privateKey; }



Signature sign(void* data, std::size_t len) {
  Signature signature;
  ed25519_SignMessage(signature.data(), privateKey().data(), nullptr, (unsigned char*)data, len);
  return signature;
}

bool verify(std::uint8_t const* signature, std::uint8_t const* publicKey, void* data, std::size_t len) {
  return ed25519_VerifySignature(signature, publicKey, (unsigned char*)data, len);
}

PublicKey  const& publicKey()  { return staticKeys().publicKey;  }

}
}