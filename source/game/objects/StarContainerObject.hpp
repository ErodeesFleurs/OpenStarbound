#pragma once

#include "StarItemBag.hpp"
#include "StarObject.hpp"
#include "StarWeightedPool.hpp"
#include "StarContainerEntity.hpp"
#include "StarItemRecipe.hpp"
#include "StarItemDatabase.hpp"

namespace Star {

class ContainerObject;
class ContainerObject : public Object, public virtual ContainerEntity {
public:
  ContainerObject(ObjectConfigConstPtr config, Json const& parameters, ItemDatabaseConstPtr itemDatabase);

  void init(World* world, EntityId entityId, EntityMode mode) override;

  void update(float dt, uint64_t currentStep) override;
  void render(RenderCallback* renderCallback) override;

  void destroy(RenderCallback* renderCallback) override;
  [[nodiscard]] InteractAction interact(InteractRequest const& request) override;

  [[nodiscard]] Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  [[nodiscard]] Json containerGuiConfig() const override;
  [[nodiscard]] String containerDescription() const override;
  [[nodiscard]] String containerSubTitle() const override;
  [[nodiscard]] ItemDescriptor iconItem() const override;

  [[nodiscard]] ItemBagConstPtr itemBag() const override;

  void containerOpen() override;
  void containerClose() override;

  void startCrafting() override;
  void stopCrafting() override;
  [[nodiscard]] bool isCrafting() const override;
  [[nodiscard]] float craftingProgress() const override;

  void burnContainerContents() override;

  [[nodiscard]] RpcPromise<ItemPtr> addItems(ItemPtr const& items) override;
  [[nodiscard]] RpcPromise<ItemPtr> putItems(size_t slot, ItemPtr const& items) override;
  [[nodiscard]] RpcPromise<ItemPtr> takeItems(size_t slot, size_t count = NPos) override;
  [[nodiscard]] RpcPromise<ItemPtr> swapItems(size_t slot, ItemPtr const& items, bool tryCombine = true) override;
  [[nodiscard]] RpcPromise<ItemPtr> applyAugment(size_t slot, ItemPtr const& augment) override;
  [[nodiscard]] RpcPromise<bool> consumeItems(ItemDescriptor const& descriptor) override;
  [[nodiscard]] RpcPromise<bool> consumeItems(size_t slot, size_t count) override;
  [[nodiscard]] RpcPromise<List<ItemPtr>> clearContainer() override;

protected:
  void getNetStates(bool initial) override;
  void setNetStates() override;

  void readStoredData(Json const& diskStore) override;
  [[nodiscard]] Json writeStoredData() const override;

private:
  using ContainerCallback = std::function<void(ContainerObject*)>;

  [[nodiscard]] ItemRecipe recipeForMaterials(List<ItemPtr> const& inputItems);
  void tickCrafting(float dt);

  [[nodiscard]] ItemPtr doAddItems(ItemPtr const& items);
  [[nodiscard]] ItemPtr doStackItems(ItemPtr const& items);
  [[nodiscard]] ItemPtr doPutItems(size_t slot, ItemPtr const& items);
  [[nodiscard]] ItemPtr doTakeItems(size_t slot, size_t count = NPos);
  [[nodiscard]] ItemPtr doSwapItems(size_t slot, ItemPtr const& items, bool tryCombine = true);
  [[nodiscard]] ItemPtr doApplyAugment(size_t slot, ItemPtr const& augment);
  [[nodiscard]] bool doConsumeItems(ItemDescriptor const& descriptor);
  [[nodiscard]] bool doConsumeItems(size_t slot, size_t count);
  [[nodiscard]] List<ItemPtr> doClearContainer();

  template<typename T>
  [[nodiscard]] RpcPromise<T> addSlavePromise(String const& message, JsonArray const& args, function<T(Json)> converter);

  void itemsUpdated();

  NetElementInt m_opened;

  NetElementBool m_crafting;
  NetElementFloat m_craftingProgress;

  ItemDatabaseConstPtr m_itemDatabase;
  ItemBagPtr m_items;
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

  List<ItemPtr> m_lostItems;
};

}
