module;
#include "StarRandom.hpp"
#include "StarLogging.hpp"
#include "curve25519/include/curve25519_dh.h"
#include "curve25519/include/ed25519_signature.h"


// GCC has no header units, so the headers this module needs arrive through the
// global module fragment.
#include "StarEncode.hpp"
#include "StarByteArray.hpp"
#include "StarArray.hpp"

export module star.curve25519;

export namespace Star {
namespace Curve25519 {

constexpr size_t PublicKeySize = 32;
constexpr size_t SecretKeySize = 32;
constexpr size_t PrivateKeySize = 64;
constexpr size_t SignatureSize = 64;

typedef Array<uint8_t, PublicKeySize> PublicKey;
typedef Array<uint8_t, SecretKeySize> SecretKey;
typedef Array<uint8_t, PrivateKeySize> PrivateKey;
typedef Array<uint8_t, SignatureSize> Signature;

PublicKey const& publicKey();
Signature sign(void* data, size_t len);
bool verify(uint8_t const* signature, uint8_t const* publicKey, void* data, size_t len);


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



Signature sign(void* data, size_t len) {
  Signature signature;
  ed25519_SignMessage(signature.data(), privateKey().data(), nullptr, (unsigned char*)data, len);
  return signature;
}

bool verify(uint8_t const* signature, uint8_t const* publicKey, void* data, size_t len) {
  return ed25519_VerifySignature(signature, publicKey, (unsigned char*)data, len);
}

PublicKey  const& publicKey()  { return staticKeys().publicKey;  }

}
}