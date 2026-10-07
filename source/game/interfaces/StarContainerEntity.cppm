module;
#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"


import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.tile_damage;

import star.interaction_types;

import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;

import star.interactive_entity;
import star.collision_block;

import star.tile_entity;

namespace Star {

STAR_CLASS(Item);
STAR_CLASS(ItemBag);
STAR_CLASS(ContainerEntity);

// All container methods may be called on both master and slave entities.
class ContainerEntity : public virtual TileEntity {
public:
  size_t containerSize() const;
  List<ItemPtr> containerItems() const;

  virtual Json containerGuiConfig() const = 0;
  virtual String containerDescription() const = 0;
  virtual String containerSubTitle() const = 0;
  virtual ItemDescriptor iconItem() const = 0;

  virtual ItemBagConstPtr itemBag() const = 0;

  virtual void containerOpen() = 0;
  virtual void containerClose() = 0;

  virtual void startCrafting() = 0;
  virtual void stopCrafting() = 0;
  virtual bool isCrafting() const = 0;
  virtual float craftingProgress() const = 0;

  virtual void burnContainerContents() = 0;

  virtual RpcPromise<ItemPtr> addItems(ItemPtr const& items) = 0;
  virtual RpcPromise<ItemPtr> putItems(size_t slot, ItemPtr const& items) = 0;
  virtual RpcPromise<ItemPtr> takeItems(size_t slot, size_t count = NPos) = 0;
  virtual RpcPromise<ItemPtr> swapItems(size_t slot, ItemPtr const& items, bool tryCombine = true) = 0;
  virtual RpcPromise<ItemPtr> applyAugment(size_t slot, ItemPtr const& augment) = 0;
  virtual RpcPromise<bool> consumeItems(ItemDescriptor const& descriptor) = 0;
  virtual RpcPromise<bool> consumeItems(size_t slot, size_t count) = 0;
  virtual RpcPromise<List<ItemPtr>> clearContainer() = 0;
};

}

export module star.container_entity;

export namespace Star {
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::ItemBag;
  using ::Star::ItemBagPtr;
  using ::Star::ItemBagConstPtr;
  using ::Star::ItemBagWeakPtr;
  using ::Star::ItemBagConstWeakPtr;
  using ::Star::ItemBagUPtr;
  using ::Star::ItemBagConstUPtr;
  using ::Star::ContainerEntity;
  using ::Star::ContainerEntityPtr;
  using ::Star::ContainerEntityConstPtr;
  using ::Star::ContainerEntityWeakPtr;
  using ::Star::ContainerEntityConstWeakPtr;
  using ::Star::ContainerEntityUPtr;
  using ::Star::ContainerEntityConstUPtr;
}
