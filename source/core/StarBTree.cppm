module;
import std;
import star.list;
import star.format;

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
  //   std::expected<void, StarException> setNewRoot(Pointer pointer, bool isLeaf);
  //
  //   Index createIndex(Pointer beginPointer);
  //
  //   // Load an existing index.
  //   std::expected<Index, StarException> loadIndex(Pointer pointer);
  //
  //   size_t indexPointerCount(Index const& index);
  //   Pointer indexPointer(Index const& index, size_t i);
  //   void indexUpdatePointer(Index& index, size_t i, Pointer p);
  //
  //   Key indexKeyBefore(Index const& index, size_t i);
  //   void indexUpdateKeyBefore(Index& index, size_t i, Key k);
  //
  //   void indexRemoveBefore(Index& index, size_t i);
  //   void indexInsertAfter(Index& index, size_t i, Key k, Pointer p);
  //
  //   size_t indexLevel(Index const& index);
  //   void setIndexLevel(Index& index, size_t indexLevel);
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
  //   std::expected<Pointer, StarException> storeIndex(Index index);
  //
  //   // Index no longer part of BTree.  Index will not be used after delete.
  //   void deleteIndex(Index index);
  //
  //   // Should create new empty leaf.
  //   Leaf createLeaf();
  //
  //   std::expected<Leaf, StarException> loadLeaf(Pointer pointer);
  //
  //   size_t leafElementCount(Leaf const& leaf);
  //   Key leafKey(Leaf const& leaf, size_t i);
  //   Data leafData(Leaf const& leaf, size_t i);
  //
  //   void leafInsert(Leaf& leaf, size_t i, Key k, Data d);
  //   void leafRemove(Leaf& leaf, size_t i);
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
  //   std::expected<Pointer, StarException> storeLeaf(Leaf leaf);
  //
  //   // Leaf is no longer part of this BTree.  Leaf will not be used after
  //   // delete.
  //   std::expected<void, StarException> deleteLeaf(Leaf leaf);
  // };
  template <typename Base>
  class BTreeMixin : public Base {
    public:
    typedef typename Base::Key Key;
    typedef typename Base::Data Data;
    typedef typename Base::Pointer Pointer;

    typedef typename Base::Index Index;
    typedef typename Base::Leaf Leaf;

    [[nodiscard]] std::expected<bool, StarException> contains(Key const& k);

    [[nodiscard]] std::expected<std::optional<Data>, StarException> find(Key const& k);

    // Range is inclusve on lower bound and exclusive on upper bound.
    [[nodiscard]] std::expected<List<std::pair<Key, Data>>, StarException> find(Key const& lower, Key const& upper);

    // Visitor returns expected<void, StarException> as visitor(key, data).
    template <typename Visitor>
    [[nodiscard]] std::expected<void, StarException> forEach(Key const& lower, Key const& upper, Visitor&& visitor);

    // Visitor is called as visitor(key, data).
    template <typename Visitor>
    [[nodiscard]] std::expected<void, StarException> forAll(Visitor&& visitor);

    // Recover all key value pairs possible, reporting node errors during scan and
    // reading as much data as possible.  Visitor is called as visitor(key, data),
    // ErrorHandler returns expected<void, StarException> as
    // error(char const*, ExceptionDiagnostic const&).
    template <typename Visitor, typename ErrorHandler>
    [[nodiscard]] std::expected<void, StarException> recoverAll(Visitor&& visitor, ErrorHandler&& error);

    // Visitor is called either as visitor(Index const&) or visitor(Leaf const&).
    // Visitor returns expected<bool, StarException>: false halts traversal.
    template <typename Visitor>
    [[nodiscard]] std::expected<void, StarException> forAllNodes(Visitor&& visitor);

    // returns true if old value overwritten.
    [[nodiscard]] std::expected<bool, StarException> insert(Key k, Data data);

    // returns true if key was found.
    [[nodiscard]] std::expected<bool, StarException> remove(Key k);

    // Removes list of keys in the given range, returns count removed.
    // TODO: SLOW, right now does lots of different removes separately.  Need to
    // implement batch inserts and deletes.
    [[nodiscard]] std::expected<List<std::pair<Key, Data>>, StarException> remove(Key const& lower, Key const& upper);

    [[nodiscard]] std::expected<std::uint64_t, StarException> indexCount();
    [[nodiscard]] std::expected<std::uint64_t, StarException> leafCount();
    [[nodiscard]] std::expected<std::uint64_t, StarException> recordCount();

    [[nodiscard]] std::expected<std::uint32_t, StarException> indexLevels();

    [[nodiscard]] std::expected<void, StarException> createNewRoot();

    private:
    struct DataElement {
      Key key;
      Data data;
    };
    typedef List<DataElement> DataList;

    struct DataCollector {
      [[nodiscard]] std::expected<void, StarException> operator()(Key const& k, Data const& d);

      List<std::pair<Key, Data>> list;
    };

    struct RecordCounter {
      [[nodiscard]] std::expected<bool, StarException> operator()(Index const& index);
      [[nodiscard]] std::expected<bool, StarException> operator()(Leaf const& leaf);

      BTreeMixin* parent;
      std::uint64_t count;
    };

    struct IndexCounter {
      [[nodiscard]] std::expected<bool, StarException> operator()(Index const& index);
      [[nodiscard]] std::expected<bool, StarException> operator()(Leaf const&);

      BTreeMixin* parent;
      std::uint64_t count;
    };

    struct LeafCounter {
      [[nodiscard]] std::expected<bool, StarException> operator()(Index const& index);
      [[nodiscard]] std::expected<bool, StarException> operator()(Leaf const&);

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

    [[nodiscard]] std::expected<bool, StarException> contains(Index const& index, Key const& k);
    [[nodiscard]] std::expected<bool, StarException> contains(Leaf const& leaf, Key const& k);

    [[nodiscard]] std::expected<std::optional<Data>, StarException> find(Index const& index, Key const& k);
    [[nodiscard]] std::expected<std::optional<Data>, StarException> find(Leaf const& leaf, Key const& k);

    // Returns the highest key for the last leaf we have searched
    template <typename Visitor>
    [[nodiscard]] std::expected<Key, StarException> forEach(Index const& index, Key const& lower, Key const& upper, Visitor&& o);
    template <typename Visitor>
    [[nodiscard]] std::expected<Key, StarException> forEach(Leaf const& leaf, Key const& lower, Key const& upper, Visitor&& o);

    // Returns the highest key for the last leaf we have searched
    template <typename Visitor>
    [[nodiscard]] std::expected<Key, StarException> forAll(Index const& index, Visitor&& o);
    template <typename Visitor>
    [[nodiscard]] std::expected<Key, StarException> forAll(Leaf const& leaf, Visitor&& o);

    template <typename Visitor, typename ErrorHandler>
    [[nodiscard]] std::expected<void, StarException> recoverAll(Index const& index, Visitor&& o, ErrorHandler&& error);
    template <typename Visitor, typename ErrorHandler>
    [[nodiscard]] std::expected<void, StarException> recoverAll(Leaf const& leaf, Visitor&& o, ErrorHandler&& error);

    // Variable size values mean that merges can happen on inserts, so can't
    // split up into insert / remove methods
    [[nodiscard]] std::expected<void, StarException> modify(Leaf& leafNode, ModifyInfo& info);
    [[nodiscard]] std::expected<void, StarException> modify(Index& indexNode, ModifyInfo& info);
    [[nodiscard]] std::expected<bool, StarException> modify(DataElement e, ModifyAction action);

    // Traverses Indexes down the tree on the left side to get the least valued
    // key that is pointed to by any leaf under this index.  Needed when joining.
    [[nodiscard]] std::expected<Key, StarException> getLeftKey(Index const& index);

    template <typename Visitor>
    [[nodiscard]] std::expected<void, StarException> forAllNodes(Index const& index, Visitor&& visitor);

    std::pair<std::size_t, bool> leafFind(Leaf const& leaf, Key const& key);
    std::size_t indexFind(Index const& index, Key const& key);
  };

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::contains(Key const& k) {
    if (Base::rootIsLeaf()) {
      auto result99 = Base::loadLeaf(Base::rootPointer());
      if (!result99) return std::unexpected(std::move(result99.error()));
      auto result100 = contains((*result99), k);
      if (!result100) return std::unexpected(std::move(result100.error()));
      return std::move(*result100);
    } else {
      auto result101 = Base::loadIndex(Base::rootPointer());
      if (!result101) return std::unexpected(std::move(result101.error()));
      auto result102 = contains((*result101), k);
      if (!result102) return std::unexpected(std::move(result102.error()));
      return std::move(*result102);
    }
  }

  template <typename Base>
  [[nodiscard]] auto BTreeMixin<Base>::find(Key const& k) -> std::expected<std::optional<Data>, StarException> {
    if (Base::rootIsLeaf()) {
      auto result95 = Base::loadLeaf(Base::rootPointer());
      if (!result95) return std::unexpected(std::move(result95.error()));
      auto result96 = find((*result95), k);
      if (!result96) return std::unexpected(std::move(result96.error()));
      return std::move(*result96);
    } else {
      auto result97 = Base::loadIndex(Base::rootPointer());
      if (!result97) return std::unexpected(std::move(result97.error()));
      auto result98 = find((*result97), k);
      if (!result98) return std::unexpected(std::move(result98.error()));
      return std::move(*result98);
    }
  }

  template <typename Base>
  [[nodiscard]] auto BTreeMixin<Base>::find(Key const& lower, Key const& upper) -> std::expected<List<std::pair<Key, Data>>, StarException> {
    DataCollector collector;
    auto result94 = forEach(lower, upper, collector);
    if (!result94) return std::unexpected(std::move(result94.error()));

    return std::move(collector.list);
  }

  template <typename Base>
  template <typename Visitor>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::forEach(Key const& lower, Key const& upper, Visitor&& visitor) {
    if (Base::rootIsLeaf()) {
      auto result90 = Base::loadLeaf(Base::rootPointer());
      if (!result90) return std::unexpected(std::move(result90.error()));
      auto result91 = forEach((*result90), lower, upper, std::forward<Visitor>(visitor));
      if (!result91) return std::unexpected(std::move(result91.error()));

    } else {
      auto result92 = Base::loadIndex(Base::rootPointer());
      if (!result92) return std::unexpected(std::move(result92.error()));
      auto result93 = forEach((*result92), lower, upper, std::forward<Visitor>(visitor));
      if (!result93) return std::unexpected(std::move(result93.error()));

    }
    return {};
  }

  template <typename Base>
  template <typename Visitor>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::forAll(Visitor&& visitor) {
    if (Base::rootIsLeaf()) {
      auto result86 = Base::loadLeaf(Base::rootPointer());
      if (!result86) return std::unexpected(std::move(result86.error()));
      auto result87 = forAll((*result86), std::forward<Visitor>(visitor));
      if (!result87) return std::unexpected(std::move(result87.error()));

    } else {
      auto result88 = Base::loadIndex(Base::rootPointer());
      if (!result88) return std::unexpected(std::move(result88.error()));
      auto result89 = forAll((*result88), std::forward<Visitor>(visitor));
      if (!result89) return std::unexpected(std::move(result89.error()));

    }
    return {};
  }

  template <typename Base>
  template <typename Visitor, typename ErrorHandler>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::recoverAll(Visitor&& visitor, ErrorHandler&& error) {
    if (Base::rootIsLeaf()) {
      auto node = Base::loadLeaf(Base::rootPointer());
      if (!node) return error("Error loading root index or leaf node", ExceptionDiagnostic(node.error()));
      return recoverAll(*node, visitor, error);
    }
    auto node = Base::loadIndex(Base::rootPointer());
    if (!node) return error("Error loading root index or leaf node", ExceptionDiagnostic(node.error()));
    return recoverAll(*node, visitor, error);

    return {};
  }

  template <typename Base>
  template <typename Visitor>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::forAllNodes(Visitor&& visitor) {
    if (Base::rootIsLeaf()) {
      auto result82 = Base::loadLeaf(Base::rootPointer());
      if (!result82) return std::unexpected(std::move(result82.error()));
      auto result83 = visitor((*result82));
      if (!result83) return std::unexpected(std::move(result83.error()));

    } else {
      auto result84 = Base::loadIndex(Base::rootPointer());
      if (!result84) return std::unexpected(std::move(result84.error()));
      auto result85 = forAllNodes((*result84), std::forward<Visitor>(visitor));
      if (!result85) return std::unexpected(std::move(result85.error()));

    }
    return {};
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::insert(Key k, Data data) {
    auto result81 = modify(DataElement{std::move(k), std::move(data)}, InsertAction);
    if (!result81) return std::unexpected(std::move(result81.error()));
    return std::move(*result81);
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::remove(Key k) {
    auto result80 = modify(DataElement{std::move(k), Data()}, RemoveAction);
    if (!result80) return std::unexpected(std::move(result80.error()));
    return std::move(*result80);
  }

  template <typename Base>
  [[nodiscard]] auto BTreeMixin<Base>::remove(Key const& lower, Key const& upper) -> std::expected<List<std::pair<Key, Data>>, StarException> {
    DataCollector collector;
    auto result78 = forEach(lower, upper, collector);
    if (!result78) return std::unexpected(std::move(result78.error()));

    for (auto const& elem : collector.list) {
      auto result79 = remove(elem.first);
      if (!result79) return std::unexpected(std::move(result79.error()));

    }

    return std::move(collector.list);
  }

  template <typename Base>
  [[nodiscard]] std::expected<std::uint64_t, StarException> BTreeMixin<Base>::indexCount() {
    IndexCounter counter = {this, 0};
    auto result77 = forAllNodes(counter);
    if (!result77) return std::unexpected(std::move(result77.error()));

    return counter.count;
  }

  template <typename Base>
  [[nodiscard]] std::expected<std::uint64_t, StarException> BTreeMixin<Base>::leafCount() {
    LeafCounter counter = {this, 0};
    auto result76 = forAllNodes(counter);
    if (!result76) return std::unexpected(std::move(result76.error()));

    return counter.count;
  }

  template <typename Base>
  [[nodiscard]] std::expected<std::uint64_t, StarException> BTreeMixin<Base>::recordCount() {
    RecordCounter counter = {this, 0};
    auto result75 = forAllNodes(counter);
    if (!result75) return std::unexpected(std::move(result75.error()));

    return counter.count;
  }

  template <typename Base>
  [[nodiscard]] std::expected<std::uint32_t, StarException> BTreeMixin<Base>::indexLevels() {
    if (Base::rootIsLeaf()) {
      return 0;
    } else {
      auto result74 = Base::loadIndex(Base::rootPointer());
      if (!result74) return std::unexpected(std::move(result74.error()));
      return Base::indexLevel((*result74)) + 1;
    }
  }

  template <typename Base>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::createNewRoot() {
    auto result72 = Base::storeLeaf(Base::createLeaf());
    if (!result72) return std::unexpected(std::move(result72.error()));
    auto result73 = Base::setNewRoot((*result72), true);
    if (!result73) return std::unexpected(std::move(result73.error()));

    return {};
  }

  template <typename Base>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::DataCollector::operator()(Key const& k, Data const& d) {
    list.push_back({k, d});
    return {};
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::RecordCounter::operator()(Index const&) {
    return true;
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::RecordCounter::operator()(Leaf const& leaf) {
    count += parent->leafElementCount(leaf);
    return true;
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::IndexCounter::operator()(Index const& index) {
    ++count;
    if (parent->indexLevel(index) == 0) {
      return false;
    } else {
      return true;
    }
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::IndexCounter::operator()(Leaf const&) {
    return false;
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::LeafCounter::operator()(Index const& index) {
    if (parent->indexLevel(index) == 0) {

      count += parent->indexPointerCount(index);
      return false;
    } else {

      return true;
    }
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::LeafCounter::operator()(Leaf const&) {
    return false;
  }

  template <typename Base>
  BTreeMixin<Base>::ModifyInfo::ModifyInfo(ModifyAction a, DataElement e)
  : targetElement(std::move(e)), action(a) {
    found = false;
    state = Done;
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::contains(Index const& index, Key const& k) {
    std::size_t i = indexFind(index, k);
    if (Base::indexLevel(index) == 0) {
      auto result68 = Base::loadLeaf(Base::indexPointer(index, i));
      if (!result68) return std::unexpected(std::move(result68.error()));
      auto result69 = contains((*result68), k);
      if (!result69) return std::unexpected(std::move(result69.error()));
      return std::move(*result69);
    } else {
      auto result70 = Base::loadIndex(Base::indexPointer(index, i));
      if (!result70) return std::unexpected(std::move(result70.error()));
      auto result71 = contains((*result70), k);
      if (!result71) return std::unexpected(std::move(result71.error()));
      return std::move(*result71);
    }
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::contains(Leaf const& leaf, Key const& k) {
    return leafFind(leaf, k).second;
  }

  template <typename Base>
  [[nodiscard]] auto BTreeMixin<Base>::find(Index const& index, Key const& k) -> std::expected<std::optional<Data>, StarException> {
    std::size_t i = indexFind(index, k);
    if (Base::indexLevel(index) == 0) {
      auto result64 = Base::loadLeaf(Base::indexPointer(index, i));
      if (!result64) return std::unexpected(std::move(result64.error()));
      auto result65 = find((*result64), k);
      if (!result65) return std::unexpected(std::move(result65.error()));
      return std::move(*result65);
    } else {
      auto result66 = Base::loadIndex(Base::indexPointer(index, i));
      if (!result66) return std::unexpected(std::move(result66.error()));
      auto result67 = find((*result66), k);
      if (!result67) return std::unexpected(std::move(result67.error()));
      return std::move(*result67);
    }
  }

  template <typename Base>
  [[nodiscard]] auto BTreeMixin<Base>::find(Leaf const& leaf, Key const& k) -> std::expected<std::optional<Data>, StarException> {
    std::pair<std::size_t, bool> res = leafFind(leaf, k);
    if (res.second) {
      return std::optional<Data>(std::in_place, Base::leafData(leaf, res.first));
    } else {
      return {};
    }
  }

  template <typename Base>
  template <typename Visitor>
  [[nodiscard]] auto BTreeMixin<Base>::forEach(Index const& index, Key const& lower, Key const& upper, Visitor&& o) -> std::expected<Key, StarException> {
    std::size_t i = indexFind(index, lower);
    Key lastKey;

    if (Base::indexLevel(index) == 0) {
      auto result56 = Base::loadLeaf(Base::indexPointer(index, i));
      if (!result56) return std::unexpected(std::move(result56.error()));
      auto result57 = forEach((*result56), lower, upper, std::forward<Visitor>(o));
      if (!result57) return std::unexpected(std::move(result57.error()));
      lastKey = std::move(*result57);
    } else {
      auto result58 = Base::loadIndex(Base::indexPointer(index, i));
      if (!result58) return std::unexpected(std::move(result58.error()));
      auto result59 = forEach((*result58), lower, upper, std::forward<Visitor>(o));
      if (!result59) return std::unexpected(std::move(result59.error()));
      lastKey = std::move(*result59);
    }

    if (!(lastKey < upper)) {
      return lastKey;
    }

    while (i < Base::indexPointerCount(index) - 1) {

      ++i;

      // We're visiting the right side of the key, so if lastKey >=
      // indexKeyBefore(index, i), we have already visited this node via nextLeaf
      // pointers, so skip it.
      if (!(lastKey < Base::indexKeyBefore(index, i))) {
        continue;
      }

      if (Base::indexLevel(index) == 0) {
        auto result60 = Base::loadLeaf(Base::indexPointer(index, i));
        if (!result60) return std::unexpected(std::move(result60.error()));
        auto result61 = forEach((*result60), lower, upper, std::forward<Visitor>(o));
        if (!result61) return std::unexpected(std::move(result61.error()));
        lastKey = std::move(*result61);
      } else {
        auto result62 = Base::loadIndex(Base::indexPointer(index, i));
        if (!result62) return std::unexpected(std::move(result62.error()));
        auto result63 = forEach((*result62), lower, upper, std::forward<Visitor>(o));
        if (!result63) return std::unexpected(std::move(result63.error()));
        lastKey = std::move(*result63);
      }

      if (!(lastKey < upper)) {
        break;
      }
    }

    return lastKey;
  }

  template <typename Base>
  template <typename Visitor>
  [[nodiscard]] auto BTreeMixin<Base>::forEach(Leaf const& leaf, Key const& lower, Key const& upper, Visitor&& o) -> std::expected<Key, StarException> {
    if (Base::leafElementCount(leaf) == 0) {
      return Key();
    }

    std::size_t lowerIndex = leafFind(leaf, lower).first;

    for (std::size_t i = lowerIndex; i != Base::leafElementCount(leaf); ++i) {

      Key currentKey = Base::leafKey(leaf, i);
      if (!(currentKey < lower)) {

        if (currentKey < upper) {
          auto result53 = o(currentKey, Base::leafData(leaf, i));
          if (!result53) return std::unexpected(std::move(result53.error()));

        } else {
          return currentKey;
        }
      }
    }

    if (auto nextLeafPointer = Base::nextLeaf(leaf)) {
      auto result54 = Base::loadLeaf(*nextLeafPointer);
      if (!result54) return std::unexpected(std::move(result54.error()));
      auto result55 = forEach((*result54), lower, upper, o);
      if (!result55) return std::unexpected(std::move(result55.error()));
      return std::move(*result55);
    } else {
      return Base::leafKey(leaf, Base::leafElementCount(leaf) - 1);
    }
  }

  template <typename Base>
  template <typename Visitor>
  [[nodiscard]] auto BTreeMixin<Base>::forAll(Index const& index, Visitor&& o) -> std::expected<Key, StarException> {
    Key lastKey;
    for (std::size_t i = 0; i < Base::indexPointerCount(index); ++i) {

      // If we're to the right of a given key, but lastKey >= this key, then we
      // must have already visited this node via nextLeaf pointers, so we can
      // skip it.
      if (i > 0 && !(lastKey < Base::indexKeyBefore(index, i))) {
        continue;
      }

      if (Base::indexLevel(index) == 0) {
        auto result49 = Base::loadLeaf(Base::indexPointer(index, i));
        if (!result49) return std::unexpected(std::move(result49.error()));
        auto result50 = forAll((*result49), std::forward<Visitor>(o));
        if (!result50) return std::unexpected(std::move(result50.error()));
        lastKey = std::move(*result50);
      } else {
        auto result51 = Base::loadIndex(Base::indexPointer(index, i));
        if (!result51) return std::unexpected(std::move(result51.error()));
        auto result52 = forAll((*result51), std::forward<Visitor>(o));
        if (!result52) return std::unexpected(std::move(result52.error()));
        lastKey = std::move(*result52);
      }
    }

    return lastKey;
  }

  template <typename Base>
  template <typename Visitor>
  [[nodiscard]] auto BTreeMixin<Base>::forAll(Leaf const& leaf, Visitor&& o) -> std::expected<Key, StarException> {
    if (Base::leafElementCount(leaf) == 0) {
      return Key();
    }

    for (std::size_t i = 0; i != Base::leafElementCount(leaf); ++i) {

      Key currentKey = Base::leafKey(leaf, i);
      auto result46 = o(Base::leafKey(leaf, i), Base::leafData(leaf, i));
      if (!result46) return std::unexpected(std::move(result46.error()));

    }

    if (auto nextLeafPointer = Base::nextLeaf(leaf)) {
      auto result47 = Base::loadLeaf(*nextLeafPointer);
      if (!result47) return std::unexpected(std::move(result47.error()));
      auto result48 = forAll((*result47), std::forward<Visitor>(o));
      if (!result48) return std::unexpected(std::move(result48.error()));
      return std::move(*result48);
    } else {
      return Base::leafKey(leaf, Base::leafElementCount(leaf) - 1);
    }
  }

  template <typename Base>
  template <typename Visitor, typename ErrorHandler>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::recoverAll(Index const& index, Visitor&& visitor, ErrorHandler&& error) {
    for (std::size_t i = 0; i < Base::indexPointerCount(index); ++i) {
      if (Base::indexLevel(index) == 0) {
        auto node = Base::loadLeaf(Base::indexPointer(index, i));
        if (!node) {
          auto reported = error("Error loading leaf node", ExceptionDiagnostic(node.error()));
          if (!reported) return std::unexpected(std::move(reported.error()));
          continue;
        }
        auto recovered = recoverAll(*node, visitor, error);
        if (!recovered) return std::unexpected(std::move(recovered.error()));
      } else {
        auto node = Base::loadIndex(Base::indexPointer(index, i));
        if (!node) {
          auto reported = error("Error loading index node", ExceptionDiagnostic(node.error()));
          if (!reported) return std::unexpected(std::move(reported.error()));
          continue;
        }
        auto recovered = recoverAll(*node, visitor, error);
        if (!recovered) return std::unexpected(std::move(recovered.error()));
      }
    }
    return {};

  }

  template <typename Base>
  template <typename Visitor, typename ErrorHandler>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::recoverAll(Leaf const& leaf, Visitor&& visitor, ErrorHandler&& error) {
    for (std::size_t i = 0; i != Base::leafElementCount(leaf); ++i) {
      auto visited = visitor(Base::leafKey(leaf, i), Base::leafData(leaf, i));
      if (!visited) return error("Error reading leaf node", ExceptionDiagnostic(visited.error()));
    }
    return {};

  }

  template <typename Base>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::modify(Leaf& leafNode, ModifyInfo& info) {
    info.state = Done;

    std::pair<std::size_t, bool> res = leafFind(leafNode, info.targetElement.key);
    std::size_t i = res.first;
    if (res.second) {

      info.found = true;
      Base::leafRemove(leafNode, i);
    }

    // No change necessary.
    if (info.action == RemoveAction && !info.found) {
      return {};
    }

    if (info.action == InsertAction) {
      Base::leafInsert(leafNode, i, info.targetElement.key, std::move(info.targetElement.data));
    }

    auto splitResult = Base::leafSplit(leafNode);
    if (splitResult) {

      Base::setNextLeaf(*splitResult, Base::nextLeaf(leafNode));
      info.newKey = Base::leafKey(*splitResult, 0);
      auto result45 = Base::storeLeaf(std::move(*splitResult));
      if (!result45) return std::unexpected(std::move(result45.error()));
      info.newPointer = std::move(*result45);

      Base::setNextLeaf(leafNode, info.newPointer);
      info.state = LeafSplit;
    } else {
      if (Base::leafNeedsShift(leafNode)) {

        info.state = LeafNeedsJoin;
      } else {

        info.state = LeafNeedsUpdate;
      }
    }
    return {};
  }

  template <typename Base>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::modify(Index& indexNode, ModifyInfo& info) {
    std::size_t i = indexFind(indexNode, info.targetElement.key);
    Pointer nextPointer = Base::indexPointer(indexNode, i);

    Leaf lowerLeaf;
    Index lowerIndex;
    if (Base::indexLevel(indexNode) == 0) {

      auto result22 = Base::loadLeaf(nextPointer);
      if (!result22) return std::unexpected(std::move(result22.error()));
      lowerLeaf = std::move(*result22);
      auto result23 = modify(lowerLeaf, info);
      if (!result23) return std::unexpected(std::move(result23.error()));

    } else {

      auto result24 = Base::loadIndex(nextPointer);
      if (!result24) return std::unexpected(std::move(result24.error()));
      lowerIndex = std::move(*result24);
      auto result25 = modify(lowerIndex, info);
      if (!result25) return std::unexpected(std::move(result25.error()));

    }

    if (info.state == Done) {
      return {};
    }

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
          auto result26 = Base::loadLeaf(Base::indexPointer(indexNode, right));
          if (!result26) return std::unexpected(std::move(result26.error()));
          rightLeaf = std::move(*result26);
        } else {

          auto result27 = Base::loadLeaf(Base::indexPointer(indexNode, left));
          if (!result27) return std::unexpected(std::move(result27.error()));
          leftLeaf = std::move(*result27);
          rightLeaf = lowerLeaf;
        }

        if (!Base::leafShift(leftLeaf, rightLeaf)) {

          // Leaves not modified, just do the pending update.
          info.state = LeafNeedsUpdate;
        } else {
          if (Base::leafElementCount(rightLeaf) == 0) {

            // Leaves merged.
            Base::setNextLeaf(leftLeaf, Base::nextLeaf(rightLeaf));
            auto result28 = Base::deleteLeaf(std::move(rightLeaf));
            if (!result28) return std::unexpected(std::move(result28.error()));

            // Replace two sibling pointer elements with one pointing to merged
            // leaf.
            if (left != 0) {
              Base::indexUpdateKeyBefore(indexNode, left, Base::leafKey(leftLeaf, 0));
            }

            auto result29 = Base::storeLeaf(std::move(leftLeaf));
            if (!result29) return std::unexpected(std::move(result29.error()));
            Base::indexUpdatePointer(indexNode, left, (*result29));
            Base::indexRemoveBefore(indexNode, right);

            selfUpdated = true;
          } else {

            // Leaves shifted.
            auto result30 = Base::storeLeaf(std::move(leftLeaf));
            if (!result30) return std::unexpected(std::move(result30.error()));
            Base::indexUpdatePointer(indexNode, left, (*result30));

            // Right leaf first key changes on shift, so always need to update
            // left index node.
            Base::indexUpdateKeyBefore(indexNode, right, Base::leafKey(rightLeaf, 0));

            auto result31 = Base::storeLeaf(std::move(rightLeaf));
            if (!result31) return std::unexpected(std::move(result31.error()));
            Base::indexUpdatePointer(indexNode, right, (*result31));

            selfUpdated = true;
          }
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
          auto result32 = Base::loadIndex(Base::indexPointer(indexNode, right));
          if (!result32) return std::unexpected(std::move(result32.error()));
          rightIndex = std::move(*result32);
        } else {

          auto result33 = Base::loadIndex(Base::indexPointer(indexNode, left));
          if (!result33) return std::unexpected(std::move(result33.error()));
          leftIndex = std::move(*result33);
          rightIndex = lowerIndex;
        }

        auto result34 = getLeftKey(rightIndex);
        if (!result34) return std::unexpected(std::move(result34.error()));
        if (!Base::indexShift(leftIndex, (*result34), rightIndex)) {

          // Indexes not modified, just do the pending update.
          info.state = IndexNeedsUpdate;
        } else {
          if (Base::indexPointerCount(rightIndex) == 0) {

            // Indexes merged.
            Base::deleteIndex(std::move(rightIndex));

            // Replace two sibling pointer elements with one pointing to merged
            // index.
            if (left != 0) {
              auto result35 = getLeftKey(leftIndex);
              if (!result35) return std::unexpected(std::move(result35.error()));
              Base::indexUpdateKeyBefore(indexNode, left, (*result35));
            }

            auto result36 = Base::storeIndex(std::move(leftIndex));
            if (!result36) return std::unexpected(std::move(result36.error()));
            Base::indexUpdatePointer(indexNode, left, (*result36));
            Base::indexRemoveBefore(indexNode, right);

            selfUpdated = true;
          } else {

            // Indexes shifted.
            auto result37 = Base::storeIndex(std::move(leftIndex));
            if (!result37) return std::unexpected(std::move(result37.error()));
            Base::indexUpdatePointer(indexNode, left, (*result37));

            // Right index first key changes on shift, so always need to update
            // right index node.
            auto result38 = getLeftKey(rightIndex);
            if (!result38) return std::unexpected(std::move(result38.error()));
            Key keyForRight = std::move(*result38);
            auto result39 = Base::storeIndex(std::move(rightIndex));
            if (!result39) return std::unexpected(std::move(result39.error()));
            Base::indexUpdatePointer(indexNode, right, (*result39));
            Base::indexUpdateKeyBefore(indexNode, right, keyForRight);

            selfUpdated = true;
          }
        }
      }
    }

    if (info.state == LeafSplit) {

      auto result40 = Base::storeLeaf(std::move(lowerLeaf));
      if (!result40) return std::unexpected(std::move(result40.error()));
      Base::indexUpdatePointer(indexNode, i, (*result40));
      Base::indexInsertAfter(indexNode, i, info.newKey, info.newPointer);
      selfUpdated = true;
    }

    if (info.state == IndexSplit) {

      auto result41 = Base::storeIndex(std::move(lowerIndex));
      if (!result41) return std::unexpected(std::move(result41.error()));
      Base::indexUpdatePointer(indexNode, i, (*result41));
      Base::indexInsertAfter(indexNode, i, info.newKey, info.newPointer);
      selfUpdated = true;
    }

    if (info.state == LeafNeedsUpdate) {

      auto result42 = Base::storeLeaf(std::move(lowerLeaf));
      if (!result42) return std::unexpected(std::move(result42.error()));
      Pointer lowerLeafPointer = std::move(*result42);
      if (lowerLeafPointer != Base::indexPointer(indexNode, i)) {

        Base::indexUpdatePointer(indexNode, i, lowerLeafPointer);
        selfUpdated = true;
      }
    }

    if (info.state == IndexNeedsUpdate) {

      auto result43 = Base::storeIndex(std::move(lowerIndex));
      if (!result43) return std::unexpected(std::move(result43.error()));
      Pointer lowerIndexPointer = std::move(*result43);
      if (lowerIndexPointer != Base::indexPointer(indexNode, i)) {

        Base::indexUpdatePointer(indexNode, i, lowerIndexPointer);
        selfUpdated = true;
      }
    }

    auto splitResult = Base::indexSplit(indexNode);
    if (splitResult) {

      info.newKey = splitResult->first;
      auto result44 = Base::storeIndex(std::move(*splitResult).second);
      if (!result44) return std::unexpected(std::move(result44.error()));
      info.newPointer = std::move(*result44);
      info.state = IndexSplit;
      selfUpdated = true;
    } else {
      if (Base::indexNeedsShift(indexNode)) {

        info.state = IndexNeedsJoin;
      } else {
        if (selfUpdated) {

          info.state = IndexNeedsUpdate;
        } else {

          info.state = Done;
        }
      }
    }
    return {};
  }

  template <typename Base>
  [[nodiscard]] std::expected<bool, StarException> BTreeMixin<Base>::modify(DataElement e, ModifyAction action) {
    ModifyInfo info(action, std::move(e));

    Leaf lowerLeaf;
    Index lowerIndex;
    if (Base::rootIsLeaf()) {

      auto result9 = Base::loadLeaf(Base::rootPointer());
      if (!result9) return std::unexpected(std::move(result9.error()));
      lowerLeaf = std::move(*result9);
      auto result10 = modify(lowerLeaf, info);
      if (!result10) return std::unexpected(std::move(result10.error()));

    } else {

      auto result11 = Base::loadIndex(Base::rootPointer());
      if (!result11) return std::unexpected(std::move(result11.error()));
      lowerIndex = std::move(*result11);
      auto result12 = modify(lowerIndex, info);
      if (!result12) return std::unexpected(std::move(result12.error()));

    }

    if (info.state == IndexNeedsJoin) {

      if (Base::indexPointerCount(lowerIndex) == 1) {

        // If root index has single pointer, then make that the new root.

        // release index first (to support the common use case of delaying
        // removes until setNewRoot)
        Pointer pointer = Base::indexPointer(lowerIndex, 0);
        std::size_t level = Base::indexLevel(lowerIndex);
        Base::deleteIndex(std::move(lowerIndex));
        auto result13 = Base::setNewRoot(pointer, level == 0);
        if (!result13) return std::unexpected(std::move(result13.error()));

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
        auto result14 = Base::storeIndex(std::move(lowerIndex));
        if (!result14) return std::unexpected(std::move(result14.error()));
        newRoot = Base::createIndex((*result14));
        Base::setIndexLevel(newRoot, rootIndexLevel);
      } else {

        auto result15 = Base::storeLeaf(std::move(lowerLeaf));
        if (!result15) return std::unexpected(std::move(result15.error()));
        newRoot = Base::createIndex((*result15));
        Base::setIndexLevel(newRoot, 0);
      }
      Base::indexInsertAfter(newRoot, 0, info.newKey, info.newPointer);
      auto result16 = Base::storeIndex(std::move(newRoot));
      if (!result16) return std::unexpected(std::move(result16.error()));
      auto result17 = Base::setNewRoot((*result16), false);
      if (!result17) return std::unexpected(std::move(result17.error()));

    }

    if (info.state == IndexNeedsUpdate) {

      auto result18 = Base::storeIndex(std::move(lowerIndex));
      if (!result18) return std::unexpected(std::move(result18.error()));
      Pointer newRootPointer = std::move(*result18);
      if (newRootPointer != Base::rootPointer()) {
        auto result19 = Base::setNewRoot(newRootPointer, false);
        if (!result19) return std::unexpected(std::move(result19.error()));

      }
    }

    if (info.state == LeafNeedsUpdate) {

      auto result20 = Base::storeLeaf(std::move(lowerLeaf));
      if (!result20) return std::unexpected(std::move(result20.error()));
      Pointer newRootPointer = std::move(*result20);
      if (newRootPointer != Base::rootPointer()) {
        auto result21 = Base::setNewRoot(newRootPointer, true);
        if (!result21) return std::unexpected(std::move(result21.error()));

      }
    }

    return info.found;
  }

  template <typename Base>
  [[nodiscard]] auto BTreeMixin<Base>::getLeftKey(Index const& index) -> std::expected<Key, StarException> {
    if (Base::indexLevel(index) == 0) {

      auto result6 = Base::loadLeaf(Base::indexPointer(index, 0));
      if (!result6) return std::unexpected(std::move(result6.error()));
      Leaf leaf = std::move(*result6);
      return Base::leafKey(leaf, 0);
    } else {

      auto result7 = Base::loadIndex(Base::indexPointer(index, 0));
      if (!result7) return std::unexpected(std::move(result7.error()));
      auto result8 = getLeftKey((*result7));
      if (!result8) return std::unexpected(std::move(result8.error()));
      return std::move(*result8);
    }
  }

  template <typename Base>
  template <typename Visitor>
  [[nodiscard]] std::expected<void, StarException> BTreeMixin<Base>::forAllNodes(Index const& index, Visitor&& visitor) {
    auto result1 = visitor(index);
    if (!result1) return std::unexpected(std::move(result1.error()));
    if (!(*result1)) {
      return {};
    }

    for (std::size_t i = 0; i < Base::indexPointerCount(index); ++i) {

      if (Base::indexLevel(index) != 0) {

        auto result2 = Base::loadIndex(Base::indexPointer(index, i));
        if (!result2) return std::unexpected(std::move(result2.error()));
        auto result3 = forAllNodes((*result2), std::forward<Visitor>(visitor));
        if (!result3) return std::unexpected(std::move(result3.error()));

      } else {

        auto result4 = Base::loadLeaf(Base::indexPointer(index, i));
        if (!result4) return std::unexpected(std::move(result4.error()));
        auto result5 = visitor((*result4));
        if (!result5) return std::unexpected(std::move(result5.error()));
        if (!(*result5)) {
          return {};
        }
      }
    }
    return {};
  }

  template <typename Base>
  std::pair<std::size_t, bool> BTreeMixin<Base>::leafFind(Leaf const& leaf, Key const& key) {
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
  std::size_t BTreeMixin<Base>::indexFind(Index const& index, Key const& key) {
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

}

export module star.btree;

export namespace Star {
  using ::Star::BTreeMixin;
}
