#pragma once

#include "StarList.hpp"

import std;

namespace Star {

// Mixin class for implementing a simple B+ Tree style database.  LOTS of
// possibilities for improvement, especially in batch deletes / inserts.
//
// The Base class itself must have the following interface:
//
// struct Base {
//   typedef KeyT Key;
//   typedef DataT Data;
//   typedef PointerT Pointer;
//
//   // Index and Leaf types may either be a literal struct, or a pointer, or a
//   // handle or whatever.  They are meant to be opaque.
//   typedef IndexT Index;
//   typedef LeafT Leaf;
//
//   Pointer rootPointer();
//   bool rootIsLeaf();
//   void setNewRoot(Pointer pointer, bool isLeaf);
//
//   Index createIndex(Pointer beginPointer);
//
//   // Load an existing index.
//   Index loadIndex(Pointer pointer);
//
//   std::size_t indexPointerCount(Index const& index);
//   Pointer indexPointer(Index const& index, std::size_t i);
//   void indexUpdatePointer(Index& index, std::size_t i, Pointer p);
//
//   Key indexKeyBefore(Index const& index, std::size_t i);
//   void indexUpdateKeyBefore(Index& index, std::size_t i, Key k);
//
//   void indexRemoveBefore(Index& index, std::size_t i);
//   void indexInsertAfter(Index& index, std::size_t i, Key k, Pointer p);
//
//   std::size_t indexLevel(Index const& index);
//   void setIndexLevel(Index& index, std::size_t indexLevel);
//
//   // Should return true if index should try to shift elements into this index
//   // from sibling index.
//   bool indexNeedsShift(Index const& index);
//
//   // Should return false if no shift done.  If merging, always merge to the
//   // left.
//   bool indexShift(Index& left, Key const& mid, Index& right);
//
//   // If a split has occurred, split right and return the mid-key and new
//   // right node.
//   std::optional<pair<Key, Index>> indexSplit(Index& index);
//
//   // Index updated, needs storing.  Return pointer to stored index (may
//   // change).  Index will not be used after store.
//   Pointer storeIndex(Index index);
//
//   // Index no longer part of BTree.  Index will not be used after delete.
//   void deleteIndex(Index index);
//
//   // Should create new empty leaf.
//   Leaf createLeaf();
//
//   Leaf loadLeaf(Pointer pointer);
//
//   std::size_t leafElementCount(Leaf const& leaf);
//   Key leafKey(Leaf const& leaf, std::size_t i);
//   Data leafData(Leaf const& leaf, std::size_t i);
//
//   void leafInsert(Leaf& leaf, std::size_t i, Key k, Data d);
//   void leafRemove(Leaf& leaf, std::size_t i);
//
//   // Set and get next-leaf pointers.  It is not required that next-leaf
//   // pointers be kept or that they be valid, so nextLeaf may return nothing.
//   void setNextLeaf(Leaf& leaf, std::optional<Pointer> n);
//   std::optional<Pointer> nextLeaf(Leaf const& leaf);
//
//   // Should return true if leaf should try to shift elements into this leaf
//   // from sibling leaf.
//   bool leafNeedsShift(Leaf const& l);
//
//   // Should return false if no change necessary.  If merging, always merge to
//   // the left.
//   bool leafShift(Leaf& left, Leaf& right);
//
//   // Always split right and return new right node if split occurs.
//   std::optional<Leaf> leafSplit(Leaf& leaf);
//
//   // Leaf has been updated, and needs to be written to storage.  Return new
//   // pointer (may be different).  Leaf will not be used after store.
//   Pointer storeLeaf(Leaf leaf);
//
//   // Leaf is no longer part of this BTree.  Leaf will not be used after
//   // delete.
//   void deleteLeaf(Leaf leaf);
// };
template <typename Base>
class BTreeMixin : public Base {
public:
  using Key = typename Base::Key;
  using Data = typename Base::Data;
  using Pointer = typename Base::Pointer;

  using Index = typename Base::Index;
  using Leaf = typename Base::Leaf;

  auto contains(Key const& key) -> bool;

  auto find(Key const& key) -> std::optional<Data>;

  auto find(Key const& lower, Key const& upper) -> List<std::pair<Key, Data>>;

  // Visitor is called as visitor(key, data).
  template <typename Visitor>
  void forEach(Key const& lower, Key const& upper, Visitor&& visitor);

  // Visitor is called as visitor(key, data).
  template <typename Visitor>
  void forAll(Visitor&& visitor);

  // Recover all key value pairs possible, catching exceptions during scan and
  // reading as much data as possible.  Visitor is called as visitor(key, data),
  // ErrorHandler is called as error(char const*, std::exception const&)
  template <typename Visitor, typename ErrorHandler>
  void recoverAll(Visitor&& visitor, ErrorHandler&& error);

  // Visitor is called either as visitor(Index const&) or visitor(Leaf const&).
  // Return false to halt traversal, true to continue.
  template <typename Visitor>
  void forAllNodes(Visitor&& visitor);

  // returns true if old value overwritten.
  auto insert(Key k, Data data) -> bool;

  // returns true if key was found.
  auto remove(Key k) -> bool;

  // Removes list of keys in the given range, returns count removed.
  // TODO: SLOW, right now does lots of different removes separately.  Need to
  // implement batch inserts and deletes.
  auto remove(Key const& lower, Key const& upper) -> List<std::pair<Key, Data>>;

  auto indexCount() -> std::uint64_t;
  auto leafCount() -> std::uint64_t;
  auto recordCount() -> std::uint64_t;

  auto indexLevels() -> std::uint32_t;

  void createNewRoot();

private:
  struct DataElement {
    Key key;
    Data data;
  };
  using DataList = List<DataElement>;

  struct DataCollector {
    void operator()(Key const& k, Data const& d);

    List<std::pair<Key, Data>> list;
  };

  struct RecordCounter {
    auto operator()(Index const& index) -> bool;
    auto operator()(Leaf const& leaf) -> bool;

    BTreeMixin* parent;
    std::uint64_t count;
  };

  struct IndexCounter {
    auto operator()(Index const& index) -> bool;
    auto operator()(Leaf const&) -> bool;

    BTreeMixin* parent;
    std::uint64_t count;
  };

  struct LeafCounter {
    auto operator()(Index const& index) -> bool;
    auto operator()(Leaf const&) -> bool;

    BTreeMixin* parent;
    std::uint64_t count;
  };

  enum ModifyAction {
    InsertAction,
    RemoveAction
  };

  enum ModifyState {
    LeafNeedsJoin,
    IndexNeedsJoin,
    LeafSplit,
    IndexSplit,
    LeafNeedsUpdate,
    IndexNeedsUpdate,
    Done
  };

  struct ModifyInfo {
    ModifyInfo(ModifyAction a, DataElement e);

    DataElement targetElement;
    ModifyAction action;
    bool found;
    ModifyState state;

    Key newKey;
    Pointer newPointer;
  };

  auto contains(Index const& index, Key const& k) -> bool;
  auto contains(Leaf const& leaf, Key const& k) -> bool;

  auto find(Index const& index, Key const& key) -> std::optional<Data>;
  auto find(Leaf const& leaf, Key const& key) -> std::optional<Data>;

  // Returns the highest key for the last leaf we have searched
  template <typename Visitor>
  auto forEach(Index const& index, Key const& lower, Key const& upper, Visitor&& o) -> Key;
  template <typename Visitor>
  auto forEach(Leaf const& leaf, Key const& lower, Key const& upper, Visitor&& o) -> Key;

  // Returns the highest key for the last leaf we have searched
  template <typename Visitor>
  auto forAll(Index const& index, Visitor&& o) -> Key;
  template <typename Visitor>
  auto forAll(Leaf const& leaf, Visitor&& o) -> Key;

  template <typename Visitor, typename ErrorHandler>
  void recoverAll(Index const& index, Visitor&& o, ErrorHandler&& error);
  template <typename Visitor, typename ErrorHandler>
  void recoverAll(Leaf const& leaf, Visitor&& o, ErrorHandler&& error);

  // Variable size values mean that merges can happen on inserts, so can't
  // split up into insert / remove methods
  void modify(Leaf& leafNode, ModifyInfo& info);
  void modify(Index& indexNode, ModifyInfo& info);
  auto modify(DataElement e, ModifyAction action) -> bool;

  // Traverses Indexes down the tree on the left side to get the least valued
  // key that is pointed to by any leaf under this index.  Needed when joining.
  auto getLeftKey(Index const& index) -> Key;

  template <typename Visitor>
  void forAllNodes(Index const& index, Visitor&& visitor);

  auto leafFind(Leaf const& leaf, Key const& key) -> std::pair<std::size_t, bool>;
  auto indexFind(Index const& index, Key const& key) -> std::size_t;
};

template <typename Base>
auto BTreeMixin<Base>::contains(Key const& k) -> bool {
  if (Base::rootIsLeaf())
    return contains(Base::loadLeaf(Base::rootPointer()), k);
  else
    return contains(Base::loadIndex(Base::rootPointer()), k);
}

template <typename Base>
auto BTreeMixin<Base>::find(Key const& k) -> std::optional<Data> {
  if (Base::rootIsLeaf())
    return find(Base::loadLeaf(Base::rootPointer()), k);
  else
    return find(Base::loadIndex(Base::rootPointer()), k);
}

template <typename Base>
auto BTreeMixin<Base>::find(Key const& lower, Key const& upper) -> List<std::pair<Key, Data>> {
  DataCollector collector;
  forEach(lower, upper, collector);
  return collector.list;
}

template <typename Base>
template <typename Visitor>
void BTreeMixin<Base>::forEach(Key const& lower, Key const& upper, Visitor&& visitor) {
  if (Base::rootIsLeaf())
    forEach(Base::loadLeaf(Base::rootPointer()), lower, upper, std::forward<Visitor>(visitor));
  else
    forEach(Base::loadIndex(Base::rootPointer()), lower, upper, std::forward<Visitor>(visitor));
}

template <typename Base>
template <typename Visitor>
void BTreeMixin<Base>::forAll(Visitor&& visitor) {
  if (Base::rootIsLeaf())
    forAll(Base::loadLeaf(Base::rootPointer()), std::forward<Visitor>(visitor));
  else
    forAll(Base::loadIndex(Base::rootPointer()), std::forward<Visitor>(visitor));
}

template <typename Base>
template <typename Visitor, typename ErrorHandler>
void BTreeMixin<Base>::recoverAll(Visitor&& visitor, ErrorHandler&& error) {
  try {
    if (Base::rootIsLeaf())
      recoverAll(Base::loadLeaf(Base::rootPointer()), std::forward<Visitor>(visitor), std::forward<ErrorHandler>(error));
    else
      recoverAll(Base::loadIndex(Base::rootPointer()), std::forward<Visitor>(visitor), std::forward<ErrorHandler>(error));
  } catch (std::exception const& e) {
    error("Error loading root index or leaf node", e);
  }
}

template <typename Base>
template <typename Visitor>
void BTreeMixin<Base>::forAllNodes(Visitor&& visitor) {
  if (Base::rootIsLeaf())
    visitor(Base::loadLeaf(Base::rootPointer()));
  else
    forAllNodes(Base::loadIndex(Base::rootPointer()), std::forward<Visitor>(visitor));
}

template <typename Base>
auto BTreeMixin<Base>::insert(Key k, Data data) -> bool {
  return modify(DataElement{std::move(k), std::move(data)}, InsertAction);
}

template <typename Base>
auto BTreeMixin<Base>::remove(Key k) -> bool {
  return modify(DataElement{std::move(k), Data()}, RemoveAction);
}

template <typename Base>
auto BTreeMixin<Base>::remove(Key const& lower, Key const& upper) -> List<std::pair<Key, Data>> {
  DataCollector collector;
  forEach(lower, upper, collector);

  for (auto const& elem : collector.list)
    remove(elem.first);

  return collector.list;
}

template <typename Base>
auto BTreeMixin<Base>::indexCount() -> std::uint64_t {
  IndexCounter counter = {this, 0};
  forAllNodes(counter);
  return counter.count;
}

template <typename Base>
auto BTreeMixin<Base>::leafCount() -> std::uint64_t {
  LeafCounter counter = {this, 0};
  forAllNodes(counter);
  return counter.count;
}

template <typename Base>
auto BTreeMixin<Base>::recordCount() -> std::uint64_t {
  RecordCounter counter = {this, 0};
  forAllNodes(counter);
  return counter.count;
}

template <typename Base>
auto BTreeMixin<Base>::indexLevels() -> std::uint32_t {
  if (Base::rootIsLeaf())
    return 0;
  else
    return Base::indexLevel(Base::loadIndex(Base::rootPointer())) + 1;
}

template <typename Base>
void BTreeMixin<Base>::createNewRoot() {
  Base::setNewRoot(Base::storeLeaf(Base::createLeaf()), true);
}

template <typename Base>
void BTreeMixin<Base>::DataCollector::operator()(Key const& k, Data const& d) {
  list.push_back({k, d});
}

template <typename Base>
auto BTreeMixin<Base>::RecordCounter::operator()(Index const&) -> bool {
  return true;
}

template <typename Base>
auto BTreeMixin<Base>::RecordCounter::operator()(Leaf const& leaf) -> bool {
  count += parent->leafElementCount(leaf);
  return true;
}

template <typename Base>
auto BTreeMixin<Base>::IndexCounter::operator()(Index const& index) -> bool {
  ++count;
  if (parent->indexLevel(index) == 0)
    return false;
  else
    return true;
}

template <typename Base>
auto BTreeMixin<Base>::IndexCounter::operator()(Leaf const&) -> bool {
  return false;
}

template <typename Base>
auto BTreeMixin<Base>::LeafCounter::operator()(Index const& index) -> bool {
  if (parent->indexLevel(index) == 0) {
    count += parent->indexPointerCount(index);
    return false;
  } else {
    return true;
  }
}

template <typename Base>
auto BTreeMixin<Base>::LeafCounter::operator()(Leaf const&) -> bool {
  return false;
}

template <typename Base>
BTreeMixin<Base>::ModifyInfo::ModifyInfo(ModifyAction a, DataElement e)
    : targetElement(std::move(e)), action(a) {
  found = false;
  state = Done;
}

template <typename Base>
auto BTreeMixin<Base>::contains(Index const& index, Key const& k) -> bool {
  std::size_t i = indexFind(index, k);
  if (Base::indexLevel(index) == 0)
    return contains(Base::loadLeaf(Base::indexPointer(index, i)), k);
  else
    return contains(Base::loadIndex(Base::indexPointer(index, i)), k);
}

template <typename Base>
auto BTreeMixin<Base>::contains(Leaf const& leaf, Key const& k) -> bool {
  return leafFind(leaf, k).second;
}

template <typename Base>
auto BTreeMixin<Base>::find(Index const& index, Key const& k) -> std::optional<Data> {
  std::size_t i = indexFind(index, k);
  if (Base::indexLevel(index) == 0)
    return find(Base::loadLeaf(Base::indexPointer(index, i)), k);
  else
    return find(Base::loadIndex(Base::indexPointer(index, i)), k);
}

template <typename Base>
auto BTreeMixin<Base>::find(Leaf const& leaf, Key const& k) -> std::optional<Data> {
  std::pair<std::size_t, bool> res = leafFind(leaf, k);
  if (res.second)
    return Base::leafData(leaf, res.first);
  else
    return std::nullopt;
}

template <typename Base>
template <typename Visitor>
auto BTreeMixin<Base>::forEach(Index const& index, Key const& lower, Key const& upper, Visitor&& o) -> Key {
  std::size_t i = indexFind(index, lower);
  Key lastKey;

  if (Base::indexLevel(index) == 0)
    lastKey = forEach(Base::loadLeaf(Base::indexPointer(index, i)), lower, upper, std::forward<Visitor>(o));
  else
    lastKey = forEach(Base::loadIndex(Base::indexPointer(index, i)), lower, upper, std::forward<Visitor>(o));

  if (!(lastKey < upper))
    return lastKey;

  while (i < Base::indexPointerCount(index) - 1) {
    ++i;

    // We're visiting the right side of the key, so if lastKey >=
    // indexKeyBefore(index, i), we have already visited this node via nextLeaf
    // pointers, so skip it.
    if (!(lastKey < Base::indexKeyBefore(index, i)))
      continue;

    if (Base::indexLevel(index) == 0)
      lastKey = forEach(Base::loadLeaf(Base::indexPointer(index, i)), lower, upper, std::forward<Visitor>(o));
    else
      lastKey = forEach(Base::loadIndex(Base::indexPointer(index, i)), lower, upper, std::forward<Visitor>(o));

    if (!(lastKey < upper))
      break;
  }

  return lastKey;
}

template <typename Base>
template <typename Visitor>
auto BTreeMixin<Base>::forEach(Leaf const& leaf, Key const& lower, Key const& upper, Visitor&& o) -> Key {
  if (Base::leafElementCount(leaf) == 0)
    return Key();

  std::size_t lowerIndex = leafFind(leaf, lower).first;

  for (std::size_t i = lowerIndex; i != Base::leafElementCount(leaf); ++i) {
    Key currentKey = Base::leafKey(leaf, i);
    if (!(currentKey < lower)) {
      if (currentKey < upper)
        o(currentKey, Base::leafData(leaf, i));
      else
        return currentKey;
    }
  }

  if (auto nextLeafPointer = Base::nextLeaf(leaf))
    return forEach(Base::loadLeaf(*nextLeafPointer), lower, upper, o);
  else
    return Base::leafKey(leaf, Base::leafElementCount(leaf) - 1);
}

template <typename Base>
template <typename Visitor>
auto BTreeMixin<Base>::forAll(Index const& index, Visitor&& o) -> Key {
  Key lastKey;
  for (std::size_t i = 0; i < Base::indexPointerCount(index); ++i) {
    // If we're to the right of a given key, but lastKey >= this key, then we
    // must have already visited this node via nextLeaf pointers, so we can
    // skip it.
    if (i > 0 && !(lastKey < Base::indexKeyBefore(index, i)))
      continue;

    if (Base::indexLevel(index) == 0)
      lastKey = forAll(Base::loadLeaf(Base::indexPointer(index, i)), std::forward<Visitor>(o));
    else
      lastKey = forAll(Base::loadIndex(Base::indexPointer(index, i)), std::forward<Visitor>(o));
  }

  return lastKey;
}

template <typename Base>
template <typename Visitor>
auto BTreeMixin<Base>::forAll(Leaf const& leaf, Visitor&& o) -> Key {
  if (Base::leafElementCount(leaf) == 0)
    return Key();

  for (std::size_t i = 0; i != Base::leafElementCount(leaf); ++i) {
    Key currentKey = Base::leafKey(leaf, i);
    o(Base::leafKey(leaf, i), Base::leafData(leaf, i));
  }

  if (auto nextLeafPointer = Base::nextLeaf(leaf))
    return forAll(Base::loadLeaf(*nextLeafPointer), std::forward<Visitor>(o));
  else
    return Base::leafKey(leaf, Base::leafElementCount(leaf) - 1);
}

template <typename Base>
template <typename Visitor, typename ErrorHandler>
void BTreeMixin<Base>::recoverAll(Index const& index, Visitor&& visitor, ErrorHandler&& error) {
  try {
    for (std::size_t i = 0; i < Base::indexPointerCount(index); ++i) {
      if (Base::indexLevel(index) == 0) {
        try {
          recoverAll(Base::loadLeaf(Base::indexPointer(index, i)), std::forward<Visitor>(visitor), std::forward<ErrorHandler>(error));
        } catch (std::exception const& e) {
          error("Error loading leaf node", e);
        }
      } else {
        try {
          recoverAll(Base::loadIndex(Base::indexPointer(index, i)), std::forward<Visitor>(visitor), std::forward<ErrorHandler>(error));
        } catch (std::exception const& e) {
          error("Error loading index node", e);
        }
      }
    }
  } catch (std::exception const& e) {
    error("Error reading index node", e);
  }
}

template <typename Base>
template <typename Visitor, typename ErrorHandler>
void BTreeMixin<Base>::recoverAll(Leaf const& leaf, Visitor&& visitor, ErrorHandler&& error) {
  try {
    for (std::size_t i = 0; i != Base::leafElementCount(leaf); ++i) {
      Key currentKey = Base::leafKey(leaf, i);
      visitor(Base::leafKey(leaf, i), Base::leafData(leaf, i));
    }
  } catch (std::exception const& e) {
    error("Error reading leaf node", e);
  }
}

template <typename Base>
void BTreeMixin<Base>::modify(Leaf& leafNode, ModifyInfo& info) {
  info.state = Done;

  std::pair<std::size_t, bool> res = leafFind(leafNode, info.targetElement.key);
  std::size_t i = res.first;
  if (res.second) {
    info.found = true;
    Base::leafRemove(leafNode, i);
  }

  // No change necessary.
  if (info.action == RemoveAction && !info.found)
    return;

  if (info.action == InsertAction)
    Base::leafInsert(leafNode, i, info.targetElement.key, std::move(info.targetElement.data));

  auto splitResult = Base::leafSplit(leafNode);
  if (splitResult) {
    Base::setNextLeaf(*splitResult, Base::nextLeaf(leafNode));
    info.newKey = Base::leafKey(*splitResult, 0);
    info.newPointer = Base::storeLeaf(std::move(*splitResult));

    Base::setNextLeaf(leafNode, info.newPointer);
    info.state = LeafSplit;
  } else if (Base::leafNeedsShift(leafNode)) {
    info.state = LeafNeedsJoin;
  } else {
    info.state = LeafNeedsUpdate;
  }
}

template <typename Base>
void BTreeMixin<Base>::modify(Index& indexNode, ModifyInfo& info) {
  std::size_t i = indexFind(indexNode, info.targetElement.key);
  Pointer nextPointer = Base::indexPointer(indexNode, i);

  Leaf lowerLeaf;
  Index lowerIndex;
  if (Base::indexLevel(indexNode) == 0) {
    lowerLeaf = Base::loadLeaf(nextPointer);
    modify(lowerLeaf, info);
  } else {
    lowerIndex = Base::loadIndex(nextPointer);
    modify(lowerIndex, info);
  }

  if (info.state == Done)
    return;

  bool selfUpdated = false;

  std::size_t left = 0;
  std::size_t right = 0;
  if (i != 0 && i == Base::indexPointerCount(indexNode) - 1) {
    left = i - 1;
    right = i;
  } else {
    left = i;
    right = i + 1;
  }

  if (info.state == LeafNeedsJoin) {
    if (Base::indexPointerCount(indexNode) < 2) {
      // Don't have enough leaves to join, just do the pending update.
      info.state = LeafNeedsUpdate;
    } else {
      Leaf leftLeaf;
      Leaf rightLeaf;

      if (left == i) {
        leftLeaf = lowerLeaf;
        rightLeaf = Base::loadLeaf(Base::indexPointer(indexNode, right));
      } else {
        leftLeaf = Base::loadLeaf(Base::indexPointer(indexNode, left));
        rightLeaf = lowerLeaf;
      }

      if (!Base::leafShift(leftLeaf, rightLeaf)) {
        // Leaves not modified, just do the pending update.
        info.state = LeafNeedsUpdate;
      } else if (Base::leafElementCount(rightLeaf) == 0) {
        // Leaves merged.
        Base::setNextLeaf(leftLeaf, Base::nextLeaf(rightLeaf));
        Base::deleteLeaf(std::move(rightLeaf));

        // Replace two sibling pointer elements with one pointing to merged
        // leaf.
        if (left != 0)
          Base::indexUpdateKeyBefore(indexNode, left, Base::leafKey(leftLeaf, 0));

        Base::indexUpdatePointer(indexNode, left, Base::storeLeaf(std::move(leftLeaf)));
        Base::indexRemoveBefore(indexNode, right);

        selfUpdated = true;
      } else {
        // Leaves shifted.
        Base::indexUpdatePointer(indexNode, left, Base::storeLeaf(std::move(leftLeaf)));

        // Right leaf first key changes on shift, so always need to update
        // left index node.
        Base::indexUpdateKeyBefore(indexNode, right, Base::leafKey(rightLeaf, 0));

        Base::indexUpdatePointer(indexNode, right, Base::storeLeaf(std::move(rightLeaf)));

        selfUpdated = true;
      }
    }
  }

  if (info.state == IndexNeedsJoin) {
    if (Base::indexPointerCount(indexNode) < 2) {
      // Don't have enough indexes to join, just do the pending update.
      info.state = IndexNeedsUpdate;
    } else {
      Index leftIndex;
      Index rightIndex;

      if (left == i) {
        leftIndex = lowerIndex;
        rightIndex = Base::loadIndex(Base::indexPointer(indexNode, right));
      } else {
        leftIndex = Base::loadIndex(Base::indexPointer(indexNode, left));
        rightIndex = lowerIndex;
      }

      if (!Base::indexShift(leftIndex, getLeftKey(rightIndex), rightIndex)) {
        // Indexes not modified, just do the pending update.
        info.state = IndexNeedsUpdate;

      } else if (Base::indexPointerCount(rightIndex) == 0) {
        // Indexes merged.
        Base::deleteIndex(std::move(rightIndex));

        // Replace two sibling pointer elements with one pointing to merged
        // index.
        if (left != 0)
          Base::indexUpdateKeyBefore(indexNode, left, getLeftKey(leftIndex));

        Base::indexUpdatePointer(indexNode, left, Base::storeIndex(std::move(leftIndex)));
        Base::indexRemoveBefore(indexNode, right);

        selfUpdated = true;
      } else {
        // Indexes shifted.
        Base::indexUpdatePointer(indexNode, left, Base::storeIndex(std::move(leftIndex)));

        // Right index first key changes on shift, so always need to update
        // right index node.
        Key keyForRight = getLeftKey(rightIndex);
        Base::indexUpdatePointer(indexNode, right, Base::storeIndex(std::move(rightIndex)));
        Base::indexUpdateKeyBefore(indexNode, right, keyForRight);

        selfUpdated = true;
      }
    }
  }

  if (info.state == LeafSplit) {
    Base::indexUpdatePointer(indexNode, i, Base::storeLeaf(std::move(lowerLeaf)));
    Base::indexInsertAfter(indexNode, i, info.newKey, info.newPointer);
    selfUpdated = true;
  }

  if (info.state == IndexSplit) {
    Base::indexUpdatePointer(indexNode, i, Base::storeIndex(std::move(lowerIndex)));
    Base::indexInsertAfter(indexNode, i, info.newKey, info.newPointer);
    selfUpdated = true;
  }

  if (info.state == LeafNeedsUpdate) {
    Pointer lowerLeafPointer = Base::storeLeaf(std::move(lowerLeaf));
    if (lowerLeafPointer != Base::indexPointer(indexNode, i)) {
      Base::indexUpdatePointer(indexNode, i, lowerLeafPointer);
      selfUpdated = true;
    }
  }

  if (info.state == IndexNeedsUpdate) {
    Pointer lowerIndexPointer = Base::storeIndex(std::move(lowerIndex));
    if (lowerIndexPointer != Base::indexPointer(indexNode, i)) {
      Base::indexUpdatePointer(indexNode, i, lowerIndexPointer);
      selfUpdated = true;
    }
  }

  auto splitResult = Base::indexSplit(indexNode);
  if (splitResult) {
    info.newKey = splitResult->first;
    info.newPointer = Base::storeIndex(std::move(*splitResult).second);
    info.state = IndexSplit;
    selfUpdated = true;
  } else if (Base::indexNeedsShift(indexNode)) {
    info.state = IndexNeedsJoin;
  } else if (selfUpdated) {
    info.state = IndexNeedsUpdate;
  } else {
    info.state = Done;
  }
}

template <typename Base>
auto BTreeMixin<Base>::modify(DataElement e, ModifyAction action) -> bool {
  ModifyInfo info(action, std::move(e));

  Leaf lowerLeaf;
  Index lowerIndex;
  if (Base::rootIsLeaf()) {
    lowerLeaf = Base::loadLeaf(Base::rootPointer());
    modify(lowerLeaf, info);
  } else {
    lowerIndex = Base::loadIndex(Base::rootPointer());
    modify(lowerIndex, info);
  }

  if (info.state == IndexNeedsJoin) {
    if (Base::indexPointerCount(lowerIndex) == 1) {
      // If root index has single pointer, then make that the new root.

      // release index first (to support the common use case of delaying
      // removes until setNewRoot)
      Pointer pointer = Base::indexPointer(lowerIndex, 0);
      std::size_t level = Base::indexLevel(lowerIndex);
      Base::deleteIndex(std::move(lowerIndex));
      Base::setNewRoot(pointer, level == 0);
    } else {
      // Else just update.
      info.state = IndexNeedsUpdate;
    }
  }

  if (info.state == LeafNeedsJoin) {
    // Ignore NeedsJoin on LeafNode root, just update.
    info.state = LeafNeedsUpdate;
  }

  if (info.state == LeafSplit || info.state == IndexSplit) {
    Index newRoot;
    if (info.state == IndexSplit) {
      auto rootIndexLevel = Base::indexLevel(lowerIndex) + 1;
      newRoot = Base::createIndex(Base::storeIndex(std::move(lowerIndex)));
      Base::setIndexLevel(newRoot, rootIndexLevel);
    } else {
      newRoot = Base::createIndex(Base::storeLeaf(std::move(lowerLeaf)));
      Base::setIndexLevel(newRoot, 0);
    }
    Base::indexInsertAfter(newRoot, 0, info.newKey, info.newPointer);
    Base::setNewRoot(Base::storeIndex(std::move(newRoot)), false);
  }

  if (info.state == IndexNeedsUpdate) {
    Pointer newRootPointer = Base::storeIndex(std::move(lowerIndex));
    if (newRootPointer != Base::rootPointer())
      Base::setNewRoot(newRootPointer, false);
  }

  if (info.state == LeafNeedsUpdate) {
    Pointer newRootPointer = Base::storeLeaf(std::move(lowerLeaf));
    if (newRootPointer != Base::rootPointer())
      Base::setNewRoot(newRootPointer, true);
  }

  return info.found;
}

template <typename Base>
auto BTreeMixin<Base>::getLeftKey(Index const& index) -> Key {
  if (Base::indexLevel(index) == 0) {
    Leaf leaf = Base::loadLeaf(Base::indexPointer(index, 0));
    return Base::leafKey(leaf, 0);
  } else {
    return getLeftKey(Base::loadIndex(Base::indexPointer(index, 0)));
  }
}

template <typename Base>
template <typename Visitor>
void BTreeMixin<Base>::forAllNodes(Index const& index, Visitor&& visitor) {
  if (!visitor(index))
    return;

  for (std::size_t i = 0; i < Base::indexPointerCount(index); ++i) {
    if (Base::indexLevel(index) != 0) {
      forAllNodes(Base::loadIndex(Base::indexPointer(index, i)), std::forward<Visitor>(visitor));
    } else {
      if (!visitor(Base::loadLeaf(Base::indexPointer(index, i))))
        return;
    }
  }
}

template <typename Base>
auto BTreeMixin<Base>::leafFind(Leaf const& leaf, Key const& key) -> std::pair<std::size_t, bool> {
  // Return lower bound binary search result.
  std::size_t size = Base::leafElementCount(leaf);
  if (size == 0)
    return {0, false};

  std::size_t len = size;
  std::size_t first = 0;
  std::size_t middle = 0;
  std::size_t half;
  while (len > 0) {
    half = len / 2;
    middle = first + half;
    if (Base::leafKey(leaf, middle) < key) {
      first = middle + 1;
      len = len - half - 1;
    } else {
      len = half;
    }
  }
  return std::make_pair(first, first < size && !(key < Base::leafKey(leaf, first)));
}

template <typename Base>
auto BTreeMixin<Base>::indexFind(Index const& index, Key const& key) -> std::size_t {
  // Return upper bound binary search result of range [1, size];
  std::size_t size = Base::indexPointerCount(index);
  if (size == 0)
    return 0;

  std::size_t len = size - 1;
  std::size_t first = 1;
  std::size_t middle = 1;
  std::size_t half;
  while (len > 0) {
    half = len / 2;
    middle = first + half;
    if (key < Base::indexKeyBefore(index, middle)) {
      len = half;
    } else {
      first = middle + 1;
      len = len - half - 1;
    }
  }
  return first - 1;
}

}// namespace Star
