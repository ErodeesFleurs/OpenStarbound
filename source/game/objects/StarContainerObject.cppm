module;
import star.game_timers;
import std;
import star.type_identity;
import star.memory;
import star.json;
import star.id_map;
import star.random;
import star.weighted_pool;
import star.casting;
import star.strong_typedef;
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

import star.lua_components;
import star.lua_animation_component;
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
class Root;

class ContainerObject : public Object, public virtual ContainerEntity {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<ContainerObject, Object, ContainerEntity>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<ContainerObject>();
  }

  ContainerObject(Root& rootContext, std::shared_ptr<ObjectConfig const> config, Json const& parameters);

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

  std::shared_ptr<ItemBag const> itemBag() const override;

  void containerOpen() override;
  void containerClose() override;

  void startCrafting() override;
  void stopCrafting() override;
  bool isCrafting() const override;
  float craftingProgress() const override;

  void burnContainerContents() override;

  RpcPromise<std::shared_ptr<Item>> addItems(std::shared_ptr<Item> const& items) override;
  RpcPromise<std::shared_ptr<Item>> putItems(std::size_t slot, std::shared_ptr<Item> const& items) override;
  RpcPromise<std::shared_ptr<Item>> takeItems(std::size_t slot, std::size_t count = NPos) override;
  RpcPromise<std::shared_ptr<Item>> swapItems(std::size_t slot, std::shared_ptr<Item> const& items, bool tryCombine = true) override;
  RpcPromise<std::shared_ptr<Item>> applyAugment(std::size_t slot, std::shared_ptr<Item> const& augment) override;
  RpcPromise<bool> consumeItems(ItemDescriptor const& descriptor) override;
  RpcPromise<bool> consumeItems(std::size_t slot, std::size_t count) override;
  RpcPromise<List<std::shared_ptr<Item>>> clearContainer() override;

protected:
  void getNetStates(bool initial) override;
  void setNetStates() override;

  void readStoredData(Json const& diskStore) override;
  Json writeStoredData() const override;

private:
  typedef std::function<void(ContainerObject*)> ContainerCallback;

  ItemRecipe recipeForMaterials(List<std::shared_ptr<Item>> const& inputItems);
  void tickCrafting(float dt);

  std::shared_ptr<Item> doAddItems(std::shared_ptr<Item> const& items);
  std::shared_ptr<Item> doStackItems(std::shared_ptr<Item> const& items);
  std::shared_ptr<Item> doPutItems(std::size_t slot, std::shared_ptr<Item> const& items);
  std::shared_ptr<Item> doTakeItems(std::size_t slot, std::size_t count = NPos);
  std::shared_ptr<Item> doSwapItems(std::size_t slot, std::shared_ptr<Item> const& items, bool tryCombine = true);
  std::shared_ptr<Item> doApplyAugment(std::size_t slot, std::shared_ptr<Item> const& augment);
  bool doConsumeItems(ItemDescriptor const& descriptor);
  bool doConsumeItems(std::size_t slot, std::size_t count);
  List<std::shared_ptr<Item>> doClearContainer();

  template<typename T>
  RpcPromise<T> addSlavePromise(String const& message, JsonArray const& args, std::function<T(Json)> converter);

  void itemsUpdated();

  NetElementInt m_opened;

  NetElementBool m_crafting;
  NetElementFloat m_craftingProgress;

  std::shared_ptr<ItemBag> m_items;
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

  List<std::shared_ptr<Item>> m_lostItems;
};

}

export module star.container_object;

export namespace Star {
  using ::Star::ContainerObject;
}
