#pragma once

#include "StarFlatHashMap.hpp"
#include "StarList.hpp"
#include "StarException.hpp"

#include <typeindex>
#include <typeinfo>
#include <cstdint>

namespace Star {
namespace ECS {

STAR_EXCEPTION(EcsException, StarException);

// Entity is just a unique identifier
using Entity = uint64_t;
constexpr Entity NullEntity = 0;

// Component type identifier
using ComponentTypeId = std::type_index;

// System priority type (lower = earlier execution)
using SystemPriority = int;

// Get a unique type ID for a component type
template<typename T>
ComponentTypeId getComponentTypeId() {
  return std::type_index(typeid(T));
}

// Entity generation counter for detecting stale entity references
struct EntityVersion {
  uint32_t index;
  uint32_t generation;
  
  bool operator==(EntityVersion const& other) const {
    return index == other.index && generation == other.generation;
  }
  
  bool operator!=(EntityVersion const& other) const {
    return !(*this == other);
  }
};

// Pack entity version into a single 64-bit value
inline Entity packEntity(EntityVersion version) {
  return (static_cast<uint64_t>(version.generation) << 32) | version.index;
}

inline EntityVersion unpackEntity(Entity entity) {
  return {
    static_cast<uint32_t>(entity & 0xFFFFFFFF),
    static_cast<uint32_t>(entity >> 32)
  };
}

} // namespace ECS
} // namespace Star
