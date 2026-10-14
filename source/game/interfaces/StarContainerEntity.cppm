module;
import std;
import star.memory;
import star.string;
import star.json;
import star.id_map;
import star.game_types;
#include "StarCasting.hpp"
import star.data_stream;
#include "StarStrongTypedef.hpp"
import star.bi_map;
import star.list;
import star.rpc_promise;


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

class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<const Item>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<const Item>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<const Item>;
class ItemBag;
using ItemBagPtr = std::shared_ptr<ItemBag>;
using ItemBagConstPtr = std::shared_ptr<const ItemBag>;
using ItemBagWeakPtr = std::weak_ptr<ItemBag>;
using ItemBagConstWeakPtr = std::weak_ptr<const ItemBag>;
using ItemBagUPtr = std::unique_ptr<ItemBag>;
using ItemBagConstUPtr = std::unique_ptr<const ItemBag>;
class ContainerEntity;
using ContainerEntityPtr = std::shared_ptr<ContainerEntity>;
using ContainerEntityConstPtr = std::shared_ptr<const ContainerEntity>;
using ContainerEntityWeakPtr = std::weak_ptr<ContainerEntity>;
using ContainerEntityConstWeakPtr = std::weak_ptr<const ContainerEntity>;
using ContainerEntityUPtr = std::unique_ptr<ContainerEntity>;
using ContainerEntityConstUPtr = std::unique_ptr<const ContainerEntity>;

// All container methods may be called on both master and slave entities.
class ContainerEntity : public virtual TileEntity {
public:
  std::size_t containerSize() const;
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
  virtual RpcPromise<ItemPtr> putItems(std::size_t slot, ItemPtr const& items) = 0;
  virtual RpcPromise<ItemPtr> takeItems(std::size_t slot, std::size_t count = NPos) = 0;
  virtual RpcPromise<ItemPtr> swapItems(std::size_t slot, ItemPtr const& items, bool tryCombine = true) = 0;
  virtual RpcPromise<ItemPtr> applyAugment(std::size_t slot, ItemPtr const& augment) = 0;
  virtual RpcPromise<bool> consumeItems(ItemDescriptor const& descriptor) = 0;
  virtual RpcPromise<bool> consumeItems(std::size_t slot, std::size_t count) = 0;
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
