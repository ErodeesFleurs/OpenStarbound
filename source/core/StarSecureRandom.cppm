module;

#include "StarByteArray.hpp"

#ifdef STAR_SYSTEM_FAMILY_WINDOWS
#define NOMINMAX
#include <windows.h>
#include <wincrypt.h>
#else
#include "StarFile.hpp"
#endif

export module star.secure_random;

export namespace Star {

// Generate cryptographically secure random numbers for usage in password salts
// and such using OS facilities
ByteArray secureRandomBytes(size_t size);

}

namespace Star {

ByteArray secureRandomBytes(size_t size) {
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
  HCRYPTPROV context = 0;
  auto res = ByteArray(size, '\0');

  CryptAcquireContext(&context, 0, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
  auto success = CryptGenRandom(context, size, (PBYTE)res.ptr());
  CryptReleaseContext(context, 0);

  if (!success)
    throw StarException("Could not read random bytes from source.");

  return res;
#else
  return File::open("/dev/urandom", IOMode::Read)->readBytes(size);
#endif
}

}
