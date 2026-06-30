#pragma once

#include "StarContainerEntity.hpp"

namespace Star {

class ContainerInteractor;
using ContainerInteractorPtr = SharedPtr<ContainerInteractor>;

using ContainerResult = List<ItemPtr>;

class ContainerInteractor {
public:
  void openContainer(ContainerEntityPtr containerEntity);
  void closeContainer();

  [[nodiscard]] bool containerOpen() const;

  // Returns NullEntityId if no container is open
  [[nodiscard]] EntityId openContainerId() const;

  // Throws exception if there is no currently open container.
  [[nodiscard]] ContainerEntityPtr const& openContainer() const;

  [[nodiscard]] List<ContainerResult> pullContainerResults();

  void swapInContainer(size_t slot, ItemPtr const& items);
  void addToContainer(ItemPtr const& items);
  void takeFromContainerSlot(size_t slot, size_t count);
  void applyAugmentInContainer(size_t slot, ItemPtr const& augment);

  void startCraftingInContainer();
  void stopCraftingInContainer();
  void burnContainer();
  void clearContainer();

private:
  [[nodiscard]] static ContainerResult resultFromItem(ItemPtr const& items);

  mutable ContainerEntityPtr m_openContainer;
  List<RpcPromise<ContainerResult>> m_pendingResults;
};

}
