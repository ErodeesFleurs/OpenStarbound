module;

#include "StarConfig.hpp"

namespace Star {

extern char const* const OpenStarVersionString;
extern char const* const StarVersionString;
extern char const* const StarSourceIdentifierString;
extern char const* const StarArchitectureString;

typedef uint32_t VersionNumber;

}

export module star.version;

export namespace Star {
  using ::Star::OpenStarVersionString;
  using ::Star::StarVersionString;
  using ::Star::StarSourceIdentifierString;
  using ::Star::StarArchitectureString;
  using ::Star::VersionNumber;
}
