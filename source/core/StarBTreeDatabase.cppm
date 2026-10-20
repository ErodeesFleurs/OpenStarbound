module;
import std;
import star.byte_array;
import star.string;
import star.list;
import star.map;
import star.io_device;
import star.maybe;
import star.format;
import star.set;
import star.btree;
import star.ordered_map;
import star.lru_cache;
import star.data_stream_devices;
import star.thread;
import star.logging;

namespace Star {

struct DBExceptionTag {
  static constexpr char const* name() { return "DBException"; }
};
using DBException = StarError<DBExceptionTag, IOException>;

class BTreeDatabase {
public:
  std::uint32_t const ContentIdentifierStringSize = 16;

  explicit BTreeDatabase(Logger& logger);
  BTreeDatabase(Logger& logger, String const& contentIdentifier, std::size_t keySize);
  // Never commits.  Call commit()/close() explicitly to report save failures.
  ~BTreeDatabase() noexcept;

  // The underlying device will be allocated in "blocks" of this size.
  // The larger the block size, the larger that index and leaf nodes can be
  // before they need to be split, but it also means that more space is wasted
  // for index or leaf nodes that are not completely full.  Cannot be changed
  // once the database is opened.  Defaults to 2048.
  std::uint32_t blockSize() const;
  void setBlockSize(std::uint32_t blockSize);

  // Constant size of the database keys.  Should be much smaller than the block
  // size, cannot be changed once a database is opened.  Defaults zero, which
  // is invalid, so must be set if opening a new database.
  std::uint32_t keySize() const;
  void setKeySize(std::uint32_t keySize);

  // Must be no greater than ContentIdentifierStringSize large.  May not be
  // called when the database is opened.
  String contentIdentifier() const;
  void setContentIdentifier(String contentIdentifier);

  // Cache size for index nodes, defaults to 64
  std::uint32_t indexCacheSize() const;
  void setIndexCacheSize(std::uint32_t indexCacheSize);

  // If true, very write operation will immediately result in a commit.
  // Defaults to true.
  bool autoCommit() const;
  void setAutoCommit(bool autoCommit);

  std::shared_ptr<IODevice> ioDevice() const;
  void setIODevice(std::shared_ptr<IODevice> device);

  // If an existing database is opened, this will update the key size, block
  // size, and content identifier with those from the opened database.
  // Otherwise, it will use the currently set values.  Returns true if a new
  // database was created, false if an existing database was found and opened.
  bool open();

  bool isOpen() const;

  bool contains(ByteArray const& k);

  Maybe<ByteArray> find(ByteArray const& k);
  List<std::pair<ByteArray, ByteArray>> find(ByteArray const& lower, ByteArray const& upper);

  void forEach(ByteArray const& lower, ByteArray const& upper, std::function<void(ByteArray, ByteArray)> v);
  void forAll(std::function<void(ByteArray, ByteArray)> v);
  void recoverAll(std::function<void(ByteArray, ByteArray)> v, std::function<void(String const&, std::exception const&)> e);

  // Returns true if a value was overwritten
  bool insert(ByteArray const& k, ByteArray const& data);

  // Returns true if the element was found and removed
  bool remove(ByteArray const& k);

  // Remove all elements in the given range, returns keys removed.
  List<ByteArray> remove(ByteArray const& lower, ByteArray const& upper);

  std::uint64_t recordCount();

  // The depth of the index nodes in this database
  std::uint8_t indexLevels();

  std::uint32_t totalBlockCount();
  std::uint32_t freeBlockCount();
  std::uint32_t indexBlockCount();
  std::uint32_t leafBlockCount();

  void commit();
  void rollback();

  // Commits pending changes and optionally closes the device.  A commit
  // failure leaves the database open so the caller can report it or roll back.
  void close(bool closeDevice = false);

private:
  typedef std::uint32_t BlockIndex;
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
    std::size_t pointerCount() const;
    BlockIndex pointer(std::size_t i) const;
    void updatePointer(std::size_t i, BlockIndex p);

    ByteArray const& keyBefore(std::size_t i) const;
    void updateKeyBefore(std::size_t i, ByteArray k);

    void removeBefore(std::size_t i);
    void insertAfter(std::size_t i, ByteArray k, BlockIndex p);

    std::uint8_t indexLevel() const;
    void setIndexLevel(std::uint8_t indexLevel);

    // count is number of elements to shift left *including* right's beginPointer
    void shiftLeft(ByteArray const& mid, IndexNode& right, std::size_t count);

    // count is number of elements to shift right
    void shiftRight(ByteArray const& mid, IndexNode& left, std::size_t count);

    // i should be index of pointer that will be the new beginPointer of right
    // node (cannot be 0).
    ByteArray split(IndexNode& right, std::size_t i);

    struct Element {
      ByteArray key;
      BlockIndex pointer;
    };
    typedef List<Element> ElementList;

    BlockIndex self;
    std::uint8_t level;
    Maybe<BlockIndex> beginPointer;
    ElementList pointers;
  };

  struct LeafNode {
    std::size_t count() const;
    ByteArray const& key(std::size_t i) const;
    ByteArray const& data(std::size_t i) const;

    void insert(std::size_t i, ByteArray k, ByteArray d);
    void remove(std::size_t i);

    // count is number of elements to shift left
    void shiftLeft(LeafNode& right, std::size_t count);

    // count is number of elements to shift right
    void shiftRight(LeafNode& left, std::size_t count);

    // i should be index of element that will be the new start of right node.
    // Returns right index node.
    void split(LeafNode& right, std::size_t i);

    struct Element {
      ByteArray key;
      ByteArray data;
    };
    typedef List<Element> ElementList;

    BlockIndex self;
    ElementList elements;
  };

  struct BTreeImpl {
    typedef ByteArray Key;
    typedef ByteArray Data;
    typedef BlockIndex Pointer;

    typedef std::shared_ptr<IndexNode> Index;
    typedef std::shared_ptr<LeafNode> Leaf;

    Pointer rootPointer();
    bool rootIsLeaf();
    void setNewRoot(Pointer pointer, bool isLeaf);

    Index createIndex(Pointer beginPointer);
    Index loadIndex(Pointer pointer);
    bool indexNeedsShift(Index const& index);
    bool indexShift(Index const& left, Key const& mid, Index const& right);
    Maybe<std::pair<Key, Index>> indexSplit(Index const& index);
    Pointer storeIndex(Index index);
    void deleteIndex(Index index);

    Leaf createLeaf();
    Leaf loadLeaf(Pointer pointer);
    bool leafNeedsShift(Leaf const& l);
    bool leafShift(Leaf& left, Leaf& right);
    Maybe<Leaf> leafSplit(Leaf& leaf);
    Pointer storeLeaf(Leaf leaf);
    void deleteLeaf(Leaf leaf);

    std::size_t indexPointerCount(Index const& index);
    Pointer indexPointer(Index const& index, std::size_t i);
    void indexUpdatePointer(Index& index, std::size_t i, Pointer p);
    Key indexKeyBefore(Index const& index, std::size_t i);
    void indexUpdateKeyBefore(Index& index, std::size_t i, Key k);
    void indexRemoveBefore(Index& index, std::size_t i);
    void indexInsertAfter(Index& index, std::size_t i, Key k, Pointer p);
    std::size_t indexLevel(Index const& index);
    void setIndexLevel(Index& index, std::size_t indexLevel);

    std::size_t leafElementCount(Leaf const& leaf);
    Key leafKey(Leaf const& leaf, std::size_t i);
    Data leafData(Leaf const& leaf, std::size_t i);
    void leafInsert(Leaf& leaf, std::size_t i, Key k, Data d);
    void leafRemove(Leaf& leaf, std::size_t i);
    Maybe<Pointer> nextLeaf(Leaf const& leaf);
    void setNextLeaf(Leaf& leaf, Maybe<Pointer> n);

    BTreeDatabase* parent;
  };

  void readBlock(BlockIndex blockIndex, std::size_t blockOffset, char* block, std::size_t size) const;
  ByteArray readBlock(BlockIndex blockIndex) const;
  void updateBlock(BlockIndex blockIndex, ByteArray const& block);

  void rawReadBlock(BlockIndex blockIndex, std::size_t blockOffset, char* block, std::size_t size) const;
  void rawWriteBlock(BlockIndex blockIndex, std::size_t blockOffset, char const* block, std::size_t size);

  void updateHeadFreeIndexBlock(BlockIndex newHead);

  FreeIndexBlock readFreeIndexBlock(BlockIndex blockIndex);
  void writeFreeIndexBlock(BlockIndex blockIndex, FreeIndexBlock indexBlock);

  std::uint32_t leafSize(std::shared_ptr<LeafNode> const& leaf) const;
  std::uint32_t maxIndexPointers() const;

  std::uint32_t dataSize(ByteArray const& d) const;
  List<BlockIndex> leafTailBlocks(BlockIndex leafPointer);

  void freeBlock(BlockIndex b);
  BlockIndex reserveBlock();
  BlockIndex makeEndBlock();

  void dirty();
  void writeRoot();
  void readRoot();
  void doCommit();
  void commitWrites();
  bool tryFlatten();
  bool flattenVisitor(BTreeImpl::Index& index, BlockIndex& count);

  void checkIfOpen(char const* methodName, bool shouldBeOpen) const;
  void checkBlockIndex(std::size_t blockIndex) const;
  void checkKeySize(ByteArray const& k) const;
  std::uint32_t maxFreeIndexLength() const;

  Logger& m_logger;
  mutable ReadersWriterMutex m_lock;

  BTreeMixin<BTreeImpl> m_impl;

  std::shared_ptr<IODevice> m_device;
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
  StreamOffset m_deviceSize;
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
  explicit BTreeSha256Database(Logger& logger);
  BTreeSha256Database(Logger& logger, String const& contentIdentifier);

  // Keys can be arbitrary size, actual key is the SHA-256 checksum of the key.
  bool contains(ByteArray const& key);
  Maybe<ByteArray> find(ByteArray const& key);
  bool insert(ByteArray const& key, ByteArray const& value);
  bool remove(ByteArray const& key);

  // Convenience string versions of access methods.  Equivalent to the utf8
  // bytes of the string minus the null terminator.
  bool contains(String const& key);
  Maybe<ByteArray> find(String const& key);
  bool insert(String const& key, ByteArray const& value);
  bool remove(String const& key);

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

export module star.btree_database;

export namespace Star {
  using ::Star::DBExceptionTag;
  using ::Star::DBException;
  using ::Star::BTreeDatabase;
  using ::Star::BTreeSha256Database;
}
