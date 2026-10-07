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


import star.container_interactor;

namespace Star {

void ContainerInteractor::openContainer(ContainerEntityPtr containerEntity) {
  if (m_openContainer && m_openContainer->inWorld())
    m_openContainer->containerClose();
  m_openContainer = std::move(containerEntity);
  if (m_openContainer) {
    starAssert(m_openContainer->inWorld());
    m_openContainer->containerOpen();
  }
}

void ContainerInteractor::closeContainer() {
  openContainer({});
}

bool ContainerInteractor::containerOpen() const {
  return openContainerId() != NullEntityId;
}

EntityId ContainerInteractor::openContainerId() const {
  if (m_openContainer && !m_openContainer->inWorld())
    m_openContainer = {};

  if (m_openContainer)
    return m_openContainer->entityId();

  return NullEntityId;
}

// Make sure to check if this is valid when you use it!
ContainerEntityPtr const& ContainerInteractor::openContainer() const {
  if (m_openContainer && !m_openContainer->inWorld())
    m_openContainer = {};

  return m_openContainer;
}

List<ContainerResult> ContainerInteractor::pullContainerResults() {
  List<List<ItemPtr>> results;
  eraseWhere(m_pendingResults, [&results](auto& promise) {
      if (auto res = promise.result())
        results.append(res.take());
      return promise.finished();
    });
  return results;
}

void ContainerInteractor::swapInContainer(size_t slot, ItemPtr const& items) {
  if (auto container = openContainer())
    m_pendingResults.append(container->swapItems(slot, items).wrap(resultFromItem));
}

void ContainerInteractor::addToContainer(ItemPtr const& items) {
  if (auto container = openContainer())
    m_pendingResults.append(container->addItems(items).wrap(resultFromItem));
}

void ContainerInteractor::takeFromContainerSlot(size_t slot, size_t count) {
  if (auto container = openContainer())
    m_pendingResults.append(container->takeItems(slot, count).wrap(resultFromItem));
}

void ContainerInteractor::applyAugmentInContainer(size_t slot, ItemPtr const& augment) {
  if (auto container = openContainer())
    m_pendingResults.append(container->applyAugment(slot, augment).wrap(resultFromItem));
}

void ContainerInteractor::startCraftingInContainer() {
  if (auto container = openContainer())
    container->startCrafting();
}

void ContainerInteractor::stopCraftingInContainer() {
  if (auto container = openContainer())
    container->stopCrafting();
}

void ContainerInteractor::burnContainer() {
  if (auto container = openContainer())
    container->burnContainerContents();
}

void ContainerInteractor::clearContainer() {
  if (auto container = openContainer())
    m_pendingResults.append(container->clearContainer());
}

ContainerResult ContainerInteractor::resultFromItem(ItemPtr const& item) {
  if (item)
    return {item};
  else
    return {};
}

}
