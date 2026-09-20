module;

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
