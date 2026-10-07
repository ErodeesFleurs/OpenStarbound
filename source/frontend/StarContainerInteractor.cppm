module;
#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"

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

STAR_CLASS(ContainerInteractor);

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

  void swapInContainer(size_t slot, ItemPtr const& items);
  void addToContainer(ItemPtr const& items);
  void takeFromContainerSlot(size_t slot, size_t count);
  void applyAugmentInContainer(size_t slot, ItemPtr const& augment);

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
