#pragma once

#include "StarItemDescriptor.hpp"
#include "StarItem.hpp"
#include "StarMaybe.hpp"

namespace Star {

class IItemDatabase {
public:
  virtual ~IItemDatabase() = default;

  virtual ItemPtr item(ItemDescriptor descriptor, Maybe<float> level = {}, Maybe<uint64_t> seed = {}, bool ignoreInvalid = false) const = 0;
  virtual ItemPtr itemShared(ItemDescriptor descriptor, Maybe<float> level = {}, Maybe<uint64_t> seed = {}) const = 0;
  virtual bool loadItem(ItemDescriptor const& descriptor, ItemPtr& itemPtr) const = 0;

  virtual void cleanup() = 0;

  virtual bool hasItem(String const& itemName) const = 0;
  virtual String itemFriendlyName(String const& itemName) const = 0;

  virtual bool ageItem(ItemPtr& item, double aging) const = 0;
  virtual ItemPtr fromJson(Json const& spec) const = 0;
  virtual ItemPtr diskLoad(Json const& diskStore) const = 0;
  virtual Json diskStore(ItemConstPtr const& itemPtr) const = 0;
  virtual Json toJson(ItemConstPtr const& itemPtr) const = 0;
};

using IItemDatabasePtr = SharedPtr<IItemDatabase>;
using IItemDatabaseConstPtr = SharedPtr<IItemDatabase const>;

}
