module;
import std;
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
import star.container_entity;

namespace Star {

class Root;

typedef List<std::shared_ptr<Item>> ContainerResult;

class ContainerInteractor {
public:
  ContainerInteractor(Root& root);

  void openContainer(std::shared_ptr<ContainerEntity> containerEntity);
  void closeContainer();

  bool containerOpen() const;

  // Returns NullEntityId if no container is open
  EntityId openContainerId() const;

  // This does not perform any checks; make sure to check if it is valid if you use it!
  std::shared_ptr<ContainerEntity> const& openContainer() const;

  List<ContainerResult> pullContainerResults();

  void swapInContainer(std::size_t slot, std::shared_ptr<Item> const& items);
  void addToContainer(std::shared_ptr<Item> const& items);
  void takeFromContainerSlot(std::size_t slot, std::size_t count);
  void applyAugmentInContainer(std::size_t slot, std::shared_ptr<Item> const& augment);

  void startCraftingInContainer();
  void stopCraftingInContainer();
  void burnContainer();
  void clearContainer();

private:
  static ContainerResult resultFromItem(std::shared_ptr<Item> const& items);

  Root* m_root;
  mutable std::shared_ptr<ContainerEntity> m_openContainer;
  List<RpcPromise<ContainerResult>> m_pendingResults;
};

}

export module star.container_interactor;

export namespace Star {
  using ::Star::ContainerInteractor;
  using ::Star::ContainerResult;
}
