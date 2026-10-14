module;
import std;
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
import star.container_entity;

namespace Star {

class ContainerInteractor;
using ContainerInteractorPtr = std::shared_ptr<ContainerInteractor>;
using ContainerInteractorConstPtr = std::shared_ptr<ContainerInteractor const>;
using ContainerInteractorWeakPtr = std::weak_ptr<ContainerInteractor>;
using ContainerInteractorConstWeakPtr = std::weak_ptr<ContainerInteractor const>;
using ContainerInteractorUPtr = std::unique_ptr<ContainerInteractor>;
using ContainerInteractorConstUPtr = std::unique_ptr<ContainerInteractor const>;

typedef List<ItemPtr> ContainerResult;

class ContainerInteractor {
public:
  void openContainer(ContainerEntityPtr containerEntity);
  void closeContainer();

  bool containerOpen() const;

  // Returns NullEntityId if no container is open
  EntityId openContainerId() const;

  // This does not perform any checks; make sure to check if it is valid if you use it!
  ContainerEntityPtr const& openContainer() const;

  List<ContainerResult> pullContainerResults();

  void swapInContainer(std::size_t slot, ItemPtr const& items);
  void addToContainer(ItemPtr const& items);
  void takeFromContainerSlot(std::size_t slot, std::size_t count);
  void applyAugmentInContainer(std::size_t slot, ItemPtr const& augment);

  void startCraftingInContainer();
  void stopCraftingInContainer();
  void burnContainer();
  void clearContainer();

private:
  static ContainerResult resultFromItem(ItemPtr const& items);

  mutable ContainerEntityPtr m_openContainer;
  List<RpcPromise<ContainerResult>> m_pendingResults;
};

}

export module star.container_interactor;

export namespace Star {
  using ::Star::ContainerInteractor;
  using ::Star::ContainerInteractorPtr;
  using ::Star::ContainerInteractorConstPtr;
  using ::Star::ContainerInteractorWeakPtr;
  using ::Star::ContainerInteractorConstWeakPtr;
  using ::Star::ContainerInteractorUPtr;
  using ::Star::ContainerInteractorConstUPtr;
  using ::Star::ContainerResult;
}
