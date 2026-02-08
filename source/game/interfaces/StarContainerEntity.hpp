#pragma once

#include "StarConfig.hpp"
#include "StarItemDescriptor.hpp"
#include "StarRpcPromise.hpp"
#include "StarTileEntity.hpp"

import std;

namespace Star {

class ItemBag;

// All container methods may be called on both master and slave entities.
class ContainerEntity : public virtual TileEntity {
public:
  [[nodiscard]] auto containerSize() const -> std::size_t;
  [[nodiscard]] auto containerItems() const -> List<Ptr<Item>>;

  [[nodiscard]] virtual auto containerGuiConfig() const -> Json = 0;
  [[nodiscard]] virtual auto containerDescription() const -> String = 0;
  [[nodiscard]] virtual auto containerSubTitle() const -> String = 0;
  [[nodiscard]] virtual auto iconItem() const -> ItemDescriptor = 0;

  [[nodiscard]] virtual auto itemBag() const -> ConstPtr<ItemBag> = 0;

  virtual void containerOpen() = 0;
  virtual void containerClose() = 0;

  virtual void startCrafting() = 0;
  virtual void stopCrafting() = 0;
  [[nodiscard]] virtual auto isCrafting() const -> bool = 0;
  [[nodiscard]] virtual auto craftingProgress() const -> float = 0;

  virtual void burnContainerContents() = 0;

  virtual auto addItems(Ptr<Item> const& items) -> RpcPromise<Ptr<Item>> = 0;
  virtual auto putItems(std::size_t slot, Ptr<Item> const& items) -> RpcPromise<Ptr<Item>> = 0;
  virtual auto takeItems(std::size_t slot, std::size_t count = std::numeric_limits<std::size_t>::max()) -> RpcPromise<Ptr<Item>> = 0;
  virtual auto swapItems(std::size_t slot, Ptr<Item> const& items, bool tryCombine = true) -> RpcPromise<Ptr<Item>> = 0;
  virtual auto applyAugment(std::size_t slot, Ptr<Item> const& augment) -> RpcPromise<Ptr<Item>> = 0;
  virtual auto consumeItems(ItemDescriptor const& descriptor) -> RpcPromise<bool> = 0;
  virtual auto consumeItems(std::size_t slot, std::size_t count) -> RpcPromise<bool> = 0;
  virtual auto clearContainer() -> RpcPromise<List<Ptr<Item>>> = 0;
};

}// namespace Star
