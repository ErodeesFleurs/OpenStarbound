#pragma once

#include "StarException.hpp"
#include "StarLruCache.hpp"
#include "StarSet.hpp"
#include "StarBTree.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarConfig.hpp"

import std;

namespace Star {

using DBException = ExceptionDerived<"DBException",IOException>;

class BTreeDatabase {
public:
  std::uint32_t const ContentIdentifierStringSize = 16;

  BTreeDatabase();
  BTreeDatabase(String const& contentIdentifier, size_t keySize);
  ~BTreeDatabase();

  // The underlying device will be allocated in "blocks" of this size.
  // The larger the block size, the larger that index and leaf nodes can be
  // before they need to be split, but it also means that more space is wasted
  // for index or leaf nodes that are not completely full.  Cannot be changed
  // once the database is opened.  Defaults to 2048.
  auto blockSize() const -> std::uint32_t;
  void setBlockSize(std::uint32_t blockSize);

  // Constant size of the database keys.  Should be much smaller than the block
  // size, cannot be changed once a database is opened.  Defaults zero, which
  // is invalid, so must be set if opening a new database.
  auto keySize() const -> std::uint32_t;
  void setKeySize(std::uint32_t keySize);

  // Must be no greater than ContentIdentifierStringSize large.  May not be
  // called when the database is opened.
  auto contentIdentifier() const -> String;
  void setContentIdentifier(String contentIdentifier);

  // Cache size for index nodes, defaults to 64
  auto indexCacheSize() const -> std::uint32_t;
  void setIndexCacheSize(std::uint32_t indexCacheSize);

  // If true, very write operation will immediately result in a commit.
  // Defaults to true.
  auto autoCommit() const -> bool;
  void setAutoCommit(bool autoCommit);

  auto ioDevice() const -> Ptr<IODevice>;
  void setIODevice(Ptr<IODevice> device);

  // If an existing database is opened, this will update the key size, block
  // size, and content identifier with those from the opened database.
  // Otherwise, it will use the currently set values.  Returns true if a new
  // database was created, false if an existing database was found and opened.
  auto open() -> bool;

  auto isOpen() const -> bool;

  auto contains(ByteArray const& k) -> bool;

  auto find(ByteArray const& k) -> std::optional<ByteArray>;
  auto find(ByteArray const& lower, ByteArray const& upper) -> List<std::pair<ByteArray, ByteArray>>;

  void forEach(ByteArray const& lower, ByteArray const& upper, std::function<void(ByteArray, ByteArray)> v);
  void forAll(std::function<void(ByteArray, ByteArray)> v);
  void recoverAll(std::function<void(ByteArray, ByteArray)> v, std::function<void(String const&, std::exception const&)> e);

  // Returns true if a value was overwritten
  auto insert(ByteArray const& k, ByteArray const& data) -> bool;

  // Returns true if the element was found and removed
  auto remove(ByteArray const& k) -> bool;

  // Remove all elements in the given range, returns keys removed.
  auto remove(ByteArray const& lower, ByteArray const& upper) -> List<ByteArray>;

  auto recordCount() -> std::uint64_t;

  // The depth of the index nodes in this database
  auto indexLevels() -> std::uint8_t;

  auto totalBlockCount() -> std::uint32_t;
  auto freeBlockCount() -> std::uint32_t;
  auto indexBlockCount() -> std::uint32_t;
  auto leafBlockCount() -> std::uint32_t;

  void commit();
  void rollback();

  void close(bool closeDevice = false);

private:
  using BlockIndex = std::uint32_t;
  static BlockIndex const InvalidBlockIndex = (BlockIndex)(-1);
  static std::uint32_t const HeaderSize = 512;

  // 8 byte magic file identifier
  static char const* const VersionMagic;
  static std::uint32_t const VersionMagicSize = 8;
  // 2 byte leaf and index start markers.
  static char const* const FreeIndexMagic;
  static char const* const IndexMagic;
  static char const* const LeafMagic;
  // static uint32_t const BlockMagicSize = 2;
  static std::size_t const BTreeRootSelectorBit = 32;
  static std::size_t const BTreeRootInfoStart = 33;
  static std::size_t const BTreeRootInfoSize = 17;

  struct FreeIndexBlock {
    BlockIndex nextFreeBlock;
    List<BlockIndex> freeBlocks;
  };

  struct IndexNode {
    [[nodiscard]] auto pointerCount() const -> std::size_t;
    [[nodiscard]] auto pointer(std::size_t i) const -> BlockIndex;
    void updatePointer(std::size_t i, BlockIndex p);

    [[nodiscard]] auto keyBefore(std::size_t i) const -> ByteArray const&;
    void updateKeyBefore(std::size_t i, ByteArray k);

    void removeBefore(std::size_t i);
    void insertAfter(std::size_t i, ByteArray k, BlockIndex p);

    [[nodiscard]] auto indexLevel() const -> std::uint8_t;
    void setIndexLevel(std::uint8_t indexLevel);

    // count is number of elements to shift left *including* right's beginPointer
    void shiftLeft(ByteArray const& mid, IndexNode& right, std::size_t count);

    // count is number of elements to shift right
    void shiftRight(ByteArray const& mid, IndexNode& left, std::size_t count);

    // i should be index of pointer that will be the new beginPointer of right
    // node (cannot be 0).
    auto split(IndexNode& right, std::size_t i) -> ByteArray;

    struct Element {
      ByteArray key;
      BlockIndex pointer;
    };
    using ElementList = List<Element>;

    BlockIndex self;
    std::uint8_t level;
    std::optional<BlockIndex> beginPointer;
    ElementList pointers;
  };

  struct LeafNode {
    [[nodiscard]] auto count() const -> std::size_t;
    [[nodiscard]] auto key(size_t i) const -> ByteArray const&;
    [[nodiscard]] auto data(size_t i) const -> ByteArray const&;

    void insert(size_t i, ByteArray k, ByteArray d);
    void remove(size_t i);

    // count is number of elements to shift left
    void shiftLeft(LeafNode& right, size_t count);

    // count is number of elements to shift right
    void shiftRight(LeafNode& left, size_t count);

    // i should be index of element that will be the new start of right node.
    // Returns right index node.
    void split(LeafNode& right, size_t i);

    struct Element {
      ByteArray key;
      ByteArray data;
    };
    using ElementList = List<Element>;

    BlockIndex self;
    ElementList elements;
  };

  struct BTreeImpl {
    using Key = ByteArray;
    using Data = ByteArray;
    using Pointer = BlockIndex;

    using Index = std::shared_ptr<IndexNode>;
    using Leaf = std::shared_ptr<LeafNode>;

    auto rootPointer() -> Pointer;
    auto rootIsLeaf() -> bool;
    void setNewRoot(Pointer pointer, bool isLeaf);

    auto createIndex(Pointer beginPointer) -> Index;
    auto loadIndex(Pointer pointer) -> Index;
    auto indexNeedsShift(Index const& index) -> bool;
    auto indexShift(Index const& left, Key const& mid, Index const& right) -> bool;
    auto indexSplit(Index const& index) -> std::optional<std::pair<Key, Index>>;
    auto storeIndex(Index index) -> Pointer;
    void deleteIndex(Index index);

    auto createLeaf() -> Leaf;
    auto loadLeaf(Pointer pointer) -> Leaf;
    auto leafNeedsShift(Leaf const& l) -> bool;
    auto leafShift(Leaf& left, Leaf& right) -> bool;
    auto leafSplit(Leaf& leaf) -> std::optional<Leaf>;
    auto storeLeaf(Leaf leaf) -> Pointer;
    void deleteLeaf(Leaf leaf);

    auto indexPointerCount(Index const& index) -> size_t;
    auto indexPointer(Index const& index, std::size_t i) -> Pointer;
    void indexUpdatePointer(Index& index, std::size_t i, Pointer p);
    auto indexKeyBefore(Index const& index, std::size_t i) -> Key;
    void indexUpdateKeyBefore(Index& index, std::size_t i, Key k);
    void indexRemoveBefore(Index& index, std::size_t i);
    void indexInsertAfter(Index& index, std::size_t i, Key k, Pointer p);
    auto indexLevel(Index const& index) -> std::size_t;
    void setIndexLevel(Index& index, std::size_t indexLevel);

    auto leafElementCount(Leaf const& leaf) -> std::size_t;
    auto leafKey(Leaf const& leaf, std::size_t i) -> Key;
    auto leafData(Leaf const& leaf, std::size_t i) -> Data;
    void leafInsert(Leaf& leaf, std::size_t i, Key k, Data d);
    void leafRemove(Leaf& leaf, std::size_t i);
    auto nextLeaf(Leaf const& leaf) -> std::optional<Pointer>;
    void setNextLeaf(Leaf& leaf, std::optional<Pointer> n);

    BTreeDatabase* parent;
  };

  void readBlock(BlockIndex blockIndex, std::size_t blockOffset, char* block, std::size_t size) const;
  auto readBlock(BlockIndex blockIndex) const -> ByteArray;
  void updateBlock(BlockIndex blockIndex, ByteArray const& block);

  void rawReadBlock(BlockIndex blockIndex, std::size_t blockOffset, char* block, std::size_t size) const;
  void rawWriteBlock(BlockIndex blockIndex, std::size_t blockOffset, char const* block, std::size_t size);

  void updateHeadFreeIndexBlock(BlockIndex newHead);

  auto readFreeIndexBlock(BlockIndex blockIndex) -> FreeIndexBlock;
  void writeFreeIndexBlock(BlockIndex blockIndex, FreeIndexBlock indexBlock);

  auto leafSize(std::shared_ptr<LeafNode> const& leaf) const -> std::uint32_t;
  auto maxIndexPointers() const -> std::uint32_t;

  auto dataSize(ByteArray const& d) const -> std::uint32_t;
  auto leafTailBlocks(BlockIndex leafPointer) -> List<BlockIndex>;

  void freeBlock(BlockIndex b);
  auto reserveBlock() -> BlockIndex;
  auto makeEndBlock() -> BlockIndex;

  void dirty();
  void writeRoot();
  void readRoot();
  void doCommit();
  void commitWrites();
  auto tryFlatten() -> bool;
  auto flattenVisitor(BTreeImpl::Index& index, BlockIndex& count) -> bool;

  void checkIfOpen(char const* methodName, bool shouldBeOpen) const;
  void checkBlockIndex(std::size_t blockIndex) const;
  void checkKeySize(ByteArray const& k) const;
  auto maxFreeIndexLength() const -> std::uint32_t;

  mutable ReadersWriterMutex m_lock;

  BTreeMixin<BTreeImpl> m_impl;

  Ptr<IODevice> m_device;
  bool m_open;

  std::uint32_t m_blockSize;
  String m_contentIdentifier;
  std::uint32_t m_keySize;

  bool m_autoCommit;

  // Reading values can mutate the index cache, so the index cache is kept
  // using a different lock.  It is only necessary to acquire this lock when
  // NOT holding the main writer lock, because if the main writer lock is held
  // then no other method would be loading an index anyway.
  mutable SpinLock m_indexCacheSpinLock;
  LruCache<BlockIndex, std::shared_ptr<IndexNode>> m_indexCache;

  BlockIndex m_headFreeIndexBlock;
  std::int64_t m_deviceSize;
  BlockIndex m_root;
  bool m_rootIsLeaf;
  bool m_usingAltRoot;
  bool m_dirty;

  // Blocks that can be freely allocated and written to without violating
  // atomic consistency.
  Set<BlockIndex> m_availableBlocks;

  // Blocks that have been written in uncommitted portions of the tree.
  Set<BlockIndex> m_uncommitted;

  // Temporarily holds written data so that it can be rolled back.
  mutable Map<BlockIndex, ByteArray> m_uncommittedWrites;
};

// Version of BTreeDatabase that hashes keys with SHA-256 to produce a unique
// constant size key.
class BTreeSha256Database : private BTreeDatabase {
public:
  BTreeSha256Database();
  BTreeSha256Database(String const& contentIdentifier);

  // Keys can be arbitrary size, actual key is the SHA-256 checksum of the key.
  auto contains(ByteArray const& key) -> bool;
  auto find(ByteArray const& key) -> std::optional<ByteArray>;
  auto insert(ByteArray const& key, ByteArray const& value) -> bool;
  auto remove(ByteArray const& key) -> bool;

  // Convenience string versions of access methods.  Equivalent to the utf8
  // bytes of the string minus the null terminator.
  auto contains(String const& key) -> bool;
  auto find(String const& key) -> std::optional<ByteArray>;
  auto insert(String const& key, ByteArray const& value) -> bool;
  auto remove(String const& key) -> bool;

  using BTreeDatabase::ContentIdentifierStringSize;
  using BTreeDatabase::blockSize;
  using BTreeDatabase::setBlockSize;
  using BTreeDatabase::contentIdentifier;
  using BTreeDatabase::setContentIdentifier;
  using BTreeDatabase::indexCacheSize;
  using BTreeDatabase::setIndexCacheSize;
  using BTreeDatabase::autoCommit;
  using BTreeDatabase::setAutoCommit;
  using BTreeDatabase::ioDevice;
  using BTreeDatabase::setIODevice;
  using BTreeDatabase::open;
  using BTreeDatabase::isOpen;
  using BTreeDatabase::recordCount;
  using BTreeDatabase::indexLevels;
  using BTreeDatabase::totalBlockCount;
  using BTreeDatabase::freeBlockCount;
  using BTreeDatabase::indexBlockCount;
  using BTreeDatabase::leafBlockCount;
  using BTreeDatabase::commit;
  using BTreeDatabase::rollback;
  using BTreeDatabase::close;
};

}
