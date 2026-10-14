module;
import std;
import star.memory;
import star.json;
import star.id_map;
import star.random;
import star.weighted_pool;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
import star.list;
import star.rpc_promise;
import star.data_stream;
import star.bi_map;
import star.game_types;
import star.periodic;
import star.periodic_function;
import star.set;
import star.color;
import star.lua;
import star.audio;
import star.thread;
import star.map;
import star.maybe;
import star.string;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ordered_map;

#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.damage_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.object;
import star.container_entity;
import star.item_recipe;

import star.net_element_basic_fields;
import star.net_element_float_fields;

namespace Star {

class ContainerObject;
using ContainerObjectPtr = std::shared_ptr<ContainerObject>;
using ContainerObjectConstPtr = std::shared_ptr<const ContainerObject>;
using ContainerObjectWeakPtr = std::weak_ptr<ContainerObject>;
using ContainerObjectConstWeakPtr = std::weak_ptr<const ContainerObject>;
using ContainerObjectUPtr = std::unique_ptr<ContainerObject>;
using ContainerObjectConstUPtr = std::unique_ptr<const ContainerObject>;

class ContainerObject : public Object, public virtual ContainerEntity {
public:
  ContainerObject(ObjectConfigConstPtr config, Json const& parameters);

  void init(World* world, EntityId entityId, EntityMode mode) override;

  void update(float dt, std::uint64_t currentStep) override;
  void render(RenderCallback* renderCallback) override;

  void destroy(RenderCallback* renderCallback) override;
  InteractAction interact(InteractRequest const& request) override;

  Maybe<ChainableJsonMessageResponse> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;

  Json containerGuiConfig() const override;
  String containerDescription() const override;
  String containerSubTitle() const override;
  ItemDescriptor iconItem() const override;

  ItemBagConstPtr itemBag() const override;

  void containerOpen() override;
  void containerClose() override;

  void startCrafting() override;
  void stopCrafting() override;
  bool isCrafting() const override;
  float craftingProgress() const override;

  void burnContainerContents() override;

  RpcPromise<ItemPtr> addItems(ItemPtr const& items) override;
  RpcPromise<ItemPtr> putItems(std::size_t slot, ItemPtr const& items) override;
  RpcPromise<ItemPtr> takeItems(std::size_t slot, std::size_t count = NPos) override;
  RpcPromise<ItemPtr> swapItems(std::size_t slot, ItemPtr const& items, bool tryCombine = true) override;
  RpcPromise<ItemPtr> applyAugment(std::size_t slot, ItemPtr const& augment) override;
  RpcPromise<bool> consumeItems(ItemDescriptor const& descriptor) override;
  RpcPromise<bool> consumeItems(std::size_t slot, std::size_t count) override;
  RpcPromise<List<ItemPtr>> clearContainer() override;

protected:
  void getNetStates(bool initial) override;
  void setNetStates() override;

  void readStoredData(Json const& diskStore) override;
  Json writeStoredData() const override;

private:
  typedef std::function<void(ContainerObject*)> ContainerCallback;

  ItemRecipe recipeForMaterials(List<ItemPtr> const& inputItems);
  void tickCrafting(float dt);

  ItemPtr doAddItems(ItemPtr const& items);
  ItemPtr doStackItems(ItemPtr const& items);
  ItemPtr doPutItems(std::size_t slot, ItemPtr const& items);
  ItemPtr doTakeItems(std::size_t slot, std::size_t count = NPos);
  ItemPtr doSwapItems(std::size_t slot, ItemPtr const& items, bool tryCombine = true);
  ItemPtr doApplyAugment(std::size_t slot, ItemPtr const& augment);
  bool doConsumeItems(ItemDescriptor const& descriptor);
  bool doConsumeItems(std::size_t slot, std::size_t count);
  List<ItemPtr> doClearContainer();

  template<typename T>
  RpcPromise<T> addSlavePromise(String const& message, JsonArray const& args, std::function<T(Json)> converter);

  void itemsUpdated();

  NetElementInt m_opened;

  NetElementBool m_crafting;
  NetElementFloat m_craftingProgress;

  ItemBagPtr m_items;
  NetElementBytes m_itemsNetState;

  // master only

  bool m_initialized;
  int m_count;
  int m_currentState;
  std::int64_t m_animationFrameCooldown;
  std::int64_t m_autoCloseCooldown;

  ItemRecipe m_goalRecipe;

  bool m_itemsUpdated;
  bool m_runUpdatedCallback;

  ContainerCallback m_containerCallback;

  EpochTimer m_ageItemsTimer;

  List<ItemPtr> m_lostItems;
};

}

export module star.container_object;

export namespace Star {
  using ::Star::ContainerObject;
  using ::Star::ContainerObjectPtr;
  using ::Star::ContainerObjectConstPtr;
  using ::Star::ContainerObjectWeakPtr;
  using ::Star::ContainerObjectConstWeakPtr;
  using ::Star::ContainerObjectUPtr;
  using ::Star::ContainerObjectConstUPtr;
}
