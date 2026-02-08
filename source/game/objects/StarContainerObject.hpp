#pragma once

#include "StarConfig.hpp"
#include "StarContainerEntity.hpp"
#include "StarItemBag.hpp"
#include "StarItemRecipe.hpp"
#include "StarObject.hpp"

import std;

namespace Star {

class ContainerObject : public Object, public virtual ContainerEntity {
public:
  ContainerObject(ConstPtr<ObjectConfig> config, Json const& parameters);

  void init(World* world, EntityId entityId, EntityMode mode) override;

  void update(float dt, uint64_t currentStep) override;
  void render(RenderCallback* renderCallback) override;

  void destroy(RenderCallback* renderCallback) override;
  auto interact(InteractRequest const& request) -> InteractAction override;

  auto receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) -> std::optional<Json> override;

  auto containerGuiConfig() const -> Json override;
  auto containerDescription() const -> String override;
  auto containerSubTitle() const -> String override;
  auto iconItem() const -> ItemDescriptor override;

  auto itemBag() const -> ConstPtr<ItemBag> override;

  void containerOpen() override;
  void containerClose() override;

  void startCrafting() override;
  void stopCrafting() override;
  auto isCrafting() const -> bool override;
  auto craftingProgress() const -> float override;

  void burnContainerContents() override;

  auto addItems(Ptr<Item> const& items) -> RpcPromise<Ptr<Item>> override;
  auto putItems(size_t slot, Ptr<Item> const& items) -> RpcPromise<Ptr<Item>> override;
  auto takeItems(size_t slot, size_t count = std::numeric_limits<std::size_t>::max()) -> RpcPromise<Ptr<Item>> override;
  auto swapItems(size_t slot, Ptr<Item> const& items, bool tryCombine = true) -> RpcPromise<Ptr<Item>> override;
  auto applyAugment(size_t slot, Ptr<Item> const& augment) -> RpcPromise<Ptr<Item>> override;
  auto consumeItems(ItemDescriptor const& descriptor) -> RpcPromise<bool> override;
  auto consumeItems(size_t slot, size_t count) -> RpcPromise<bool> override;
  auto clearContainer() -> RpcPromise<List<Ptr<Item>>> override;

protected:
  void getNetStates(bool initial) override;
  void setNetStates() override;

  void readStoredData(Json const& diskStore) override;
  auto writeStoredData() const -> Json override;

private:
  using ContainerCallback = std::function<void(ContainerObject*)>;

  auto recipeForMaterials(List<Ptr<Item>> const& inputItems) -> ItemRecipe;
  void tickCrafting(float dt);

  auto doAddItems(Ptr<Item> const& items) -> Ptr<Item>;
  auto doStackItems(Ptr<Item> const& items) -> Ptr<Item>;
  auto doPutItems(size_t slot, Ptr<Item> const& items) -> Ptr<Item>;
  auto doTakeItems(size_t slot, size_t count = std::numeric_limits<std::size_t>::max()) -> Ptr<Item>;
  auto doSwapItems(size_t slot, Ptr<Item> const& items, bool tryCombine = true) -> Ptr<Item>;
  auto doApplyAugment(size_t slot, Ptr<Item> const& augment) -> Ptr<Item>;
  auto doConsumeItems(ItemDescriptor const& descriptor) -> bool;
  auto doConsumeItems(size_t slot, size_t count) -> bool;
  auto doClearContainer() -> List<Ptr<Item>>;

  template <typename T>
  auto addSlavePromise(String const& message, JsonArray const& args, std::function<T(Json)> converter) -> RpcPromise<T>;

  void itemsUpdated();

  NetElementInt m_opened;

  NetElementBool m_crafting;
  NetElementFloat m_craftingProgress;

  Ptr<ItemBag> m_items;
  NetElementBytes m_itemsNetState;

  // master only

  bool m_initialized;
  int m_count;
  int m_currentState;
  int64_t m_animationFrameCooldown;
  int64_t m_autoCloseCooldown;

  ItemRecipe m_goalRecipe;

  bool m_itemsUpdated;
  bool m_runUpdatedCallback;

  ContainerCallback m_containerCallback;

  EpochTimer m_ageItemsTimer;

  List<Ptr<Item>> m_lostItems;
};

}// namespace Star
