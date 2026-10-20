module;
import std;
import star.type_identity;
import star.memory;
import star.string;
import star.json;
import star.id_map;
import star.game_types;
import star.casting;
import star.data_stream;
import star.strong_typedef;
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
class ItemBag;

// All container methods may be called on both master and slave entities.
class ContainerEntity : public virtual TileEntity {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<ContainerEntity, TileEntity>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<ContainerEntity>();
  }

  std::size_t containerSize() const;
  List<std::shared_ptr<Item>> containerItems() const;

  virtual Json containerGuiConfig() const = 0;
  virtual String containerDescription() const = 0;
  virtual String containerSubTitle() const = 0;
  virtual ItemDescriptor iconItem() const = 0;

  virtual std::shared_ptr<ItemBag const> itemBag() const = 0;

  virtual void containerOpen() = 0;
  virtual void containerClose() = 0;

  virtual void startCrafting() = 0;
  virtual void stopCrafting() = 0;
  virtual bool isCrafting() const = 0;
  virtual float craftingProgress() const = 0;

  virtual void burnContainerContents() = 0;

  virtual RpcPromise<std::shared_ptr<Item>> addItems(std::shared_ptr<Item> const& items) = 0;
  virtual RpcPromise<std::shared_ptr<Item>> putItems(std::size_t slot, std::shared_ptr<Item> const& items) = 0;
  virtual RpcPromise<std::shared_ptr<Item>> takeItems(std::size_t slot, std::size_t count = NPos) = 0;
  virtual RpcPromise<std::shared_ptr<Item>> swapItems(std::size_t slot, std::shared_ptr<Item> const& items, bool tryCombine = true) = 0;
  virtual RpcPromise<std::shared_ptr<Item>> applyAugment(std::size_t slot, std::shared_ptr<Item> const& augment) = 0;
  virtual RpcPromise<bool> consumeItems(ItemDescriptor const& descriptor) = 0;
  virtual RpcPromise<bool> consumeItems(std::size_t slot, std::size_t count) = 0;
  virtual RpcPromise<List<std::shared_ptr<Item>>> clearContainer() = 0;
};

}

export module star.container_entity;

export namespace Star {
  using ::Star::Item;
  using ::Star::ItemBag;
  using ::Star::ContainerEntity;
}
