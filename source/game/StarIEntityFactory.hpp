#pragma once

#include "StarEntity.hpp"
#include "StarMaybe.hpp"

namespace Star {

class IEntityFactory {
public:
  virtual ~IEntityFactory() = default;

  virtual EntityPtr create(String const& entityName, Json const& extraParams = {}) const = 0;
  virtual ByteArray netStoreEntity(EntityPtr const& entity, NetCompatibilityRules rules = {}) const = 0;
  virtual EntityPtr netLoadEntity(EntityType type, ByteArray const& netStore, NetCompatibilityRules rules = {}) const = 0;
};

using IEntityFactoryPtr = SharedPtr<IEntityFactory>;
using IEntityFactoryConstPtr = SharedPtr<IEntityFactory const>;

}
