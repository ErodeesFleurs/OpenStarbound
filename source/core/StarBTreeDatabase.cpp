#include "StarBTreeDatabase.hpp"

#include "StarDataStreamDevices.hpp"
#include "StarLogging.hpp"
#include "StarSha256.hpp"
#include "StarTime.hpp"
#include "StarVlqEncoding.hpp"

import std;

namespace Star {

BTreeDatabase::BTreeDatabase() {
  m_impl.parent = this;
  m_open = false;
  m_deviceSize = 0;
  m_blockSize = 2048;
  m_headFreeIndexBlock = InvalidBlockIndex;
  m_keySize = 0;
  m_autoCommit = true;
  m_indexCache.setMaxSize(64);
  m_root = InvalidBlockIndex;
  m_rootIsLeaf = false;
  m_usingAltRoot = false;
}

BTreeDatabase::BTreeDatabase(String const& contentIdentifier, std::size_t keySize)
    : BTreeDatabase() {
  setContentIdentifier(contentIdentifier);
  setKeySize(keySize);
}

BTreeDatabase::~BTreeDatabase() {
  close();
}

auto BTreeDatabase::blockSize() const -> std::uint32_t {
  ReadLocker readLocker(m_lock);
  return m_blockSize;
}

void BTreeDatabase::setBlockSize(std::uint32_t blockSize) {
  WriteLocker writeLocker(m_lock);
  checkIfOpen("setBlockSize", false);
  m_blockSize = blockSize;
}

auto BTreeDatabase::keySize() const -> std::uint32_t {
  ReadLocker readLocker(m_lock);
  return m_keySize;
}

void BTreeDatabase::setKeySize(std::uint32_t keySize) {
  WriteLocker writeLocker(m_lock);
  checkIfOpen("setKeySize", false);
  m_keySize = keySize;
}

auto BTreeDatabase::contentIdentifier() const -> String {
  ReadLocker readLocker(m_lock);
  return m_contentIdentifier;
}

void BTreeDatabase::setContentIdentifier(String contentIdentifier) {
  WriteLocker writeLocker(m_lock);
  checkIfOpen("setContentIdentifier", false);
  m_contentIdentifier = std::move(contentIdentifier);
}

auto BTreeDatabase::indexCacheSize() const -> std::uint32_t {
  SpinLocker lock(m_indexCacheSpinLock);
  return m_indexCache.maxSize();
}

void BTreeDatabase::setIndexCacheSize(std::uint32_t indexCacheSize) {
  SpinLocker lock(m_indexCacheSpinLock);
  m_indexCache.setMaxSize(indexCacheSize);
}

auto BTreeDatabase::autoCommit() const -> bool {
  ReadLocker readLocker(m_lock);
  return m_autoCommit;
}

void BTreeDatabase::setAutoCommit(bool autoCommit) {
  WriteLocker writeLocker(m_lock);
  m_autoCommit = autoCommit;
  if (m_autoCommit)
    doCommit();
}

auto BTreeDatabase::ioDevice() const -> Ptr<IODevice> {
  ReadLocker readLocker(m_lock);
  return m_device;
}

void BTreeDatabase::setIODevice(Ptr<IODevice> device) {
  WriteLocker writeLocker(m_lock);
  checkIfOpen("setIODevice", false);
  m_device = std::move(device);
}

auto BTreeDatabase::isOpen() const -> bool {
  ReadLocker readLocker(m_lock);
  return m_open;
}

auto BTreeDatabase::open() -> bool {
  WriteLocker writeLocker(m_lock);
  if (m_open)
    return false;

  if (!m_device)
    throw DBException("BlockStorage::open called with no IODevice set");

  if (!m_device->isOpen())
    m_device->open(IOMode::ReadWrite);

  m_open = true;

  if (m_device->size() > 0) {
    DataStreamIODevice ds(m_device);
    ds.seek(0);

    auto magic = ds.readBytes(VersionMagicSize);
    if (magic != ByteArray::fromCString(VersionMagic))
      throw DBException("Device is not a valid BTreeDatabase file");

    m_blockSize = ds.read<std::uint32_t>();

    auto contentIdentifier = ds.readBytes(ContentIdentifierStringSize);
    contentIdentifier.appendByte('\0');
    m_contentIdentifier = String(contentIdentifier.ptr());
    m_keySize = ds.read<std::uint32_t>();

    readRoot();

    if (m_device->isWritable())
      m_device->resize(m_deviceSize);

    return false;

  } else {
    m_deviceSize = HeaderSize;
    m_device->resize(m_deviceSize);
    m_headFreeIndexBlock = InvalidBlockIndex;

    DataStreamIODevice ds(m_device);
    ds.seek(0);

    ds.writeData(VersionMagic, VersionMagicSize);
    ds.write<std::uint32_t>(m_blockSize);

    if (m_contentIdentifier.empty())
      throw DBException("Opening new database and no content identifier set!");

    if (m_contentIdentifier.utf8Size() > ContentIdentifierStringSize)
      throw DBException("contentIdentifier in BTreeDatabase implementation is greater than maximum identifier length");
    if (m_keySize == 0)
      throw DBException("key size is not set opening a new BTreeDatabase");

    ByteArray contentIdentifier = m_contentIdentifier.utf8Bytes();
    contentIdentifier.resize(ContentIdentifierStringSize, 0);
    ds.writeBytes(contentIdentifier);
    ds.write(m_keySize);

    m_impl.createNewRoot();
    doCommit();

    return true;
  }
}

auto BTreeDatabase::contains(ByteArray const& k) -> bool {
  ReadLocker readLocker(m_lock);
  checkKeySize(k);
  return m_impl.contains(k);
}

auto BTreeDatabase::find(ByteArray const& k) -> std::optional<ByteArray> {
  ReadLocker readLocker(m_lock);
  checkKeySize(k);
  return m_impl.find(k);
}

auto BTreeDatabase::find(ByteArray const& lower, ByteArray const& upper) -> List<std::pair<ByteArray, ByteArray>> {
  ReadLocker readLocker(m_lock);
  checkKeySize(lower);
  checkKeySize(upper);
  return m_impl.find(lower, upper);
}

void BTreeDatabase::forEach(ByteArray const& lower, ByteArray const& upper, std::function<void(ByteArray, ByteArray)> v) {
  ReadLocker readLocker(m_lock);
  checkKeySize(lower);
  checkKeySize(upper);
  m_impl.forEach(lower, upper, std::move(v));
}

void BTreeDatabase::forAll(std::function<void(ByteArray, ByteArray)> v) {
  ReadLocker readLocker(m_lock);
  m_impl.forAll(std::move(v));
}

void BTreeDatabase::recoverAll(std::function<void(ByteArray, ByteArray)> v, std::function<void(String const&, std::exception const&)> e) {
  ReadLocker readLocker(m_lock);
  m_impl.recoverAll(std::move(v), std::move(e));
}

auto BTreeDatabase::insert(ByteArray const& k, ByteArray const& data) -> bool {
  WriteLocker writeLocker(m_lock);
  checkKeySize(k);
  return m_impl.insert(k, data);
}

auto BTreeDatabase::remove(ByteArray const& k) -> bool {
  WriteLocker writeLocker(m_lock);
  checkKeySize(k);
  return m_impl.remove(k);
}

auto BTreeDatabase::recordCount() -> std::uint64_t {
  ReadLocker readLocker(m_lock);
  return m_impl.recordCount();
}

auto BTreeDatabase::indexLevels() -> std::uint8_t {
  ReadLocker readLocker(m_lock);
  return m_impl.indexLevels();
}

auto BTreeDatabase::totalBlockCount() -> std::uint32_t {
  ReadLocker readLocker(m_lock);
  checkIfOpen("totalBlockCount", true);
  return (m_device->size() - HeaderSize) / m_blockSize;
}

auto BTreeDatabase::freeBlockCount() -> std::uint32_t {
  ReadLocker readLocker(m_lock);
  checkIfOpen("freeBlockCount", true);

  // Go through every FreeIndexBlock in the chain and count all of the tracked
  // free blocks.
  BlockIndex count = 0;
  BlockIndex indexBlockIndex = m_headFreeIndexBlock;
  while (indexBlockIndex != InvalidBlockIndex) {
    FreeIndexBlock indexBlock = readFreeIndexBlock(indexBlockIndex);
    count += 1 + indexBlock.freeBlocks.size();
    indexBlockIndex = indexBlock.nextFreeBlock;
  }

  count += m_availableBlocks.size();

  // Include untracked blocks at the end of the file in the free count.
  count += (m_device->size() - m_deviceSize) / m_blockSize;

  return count;
}

auto BTreeDatabase::indexBlockCount() -> std::uint32_t {
  ReadLocker readLocker(m_lock);
  checkIfOpen("indexBlockCount", true);
  // Indexes are simply one index per block
  return m_impl.indexCount();
}

auto BTreeDatabase::leafBlockCount() -> std::uint32_t {
  WriteLocker writeLocker(m_lock);
  checkIfOpen("leafBlockCount", true);

  struct LeafBlocksVisitor {
    auto operator()(std::shared_ptr<IndexNode> const&) -> bool {
      return true;
    }

    auto operator()(std::shared_ptr<LeafNode> const& leaf) -> bool {
      leafBlockCount += 1 + parent->leafTailBlocks(leaf->self).size();
      return true;
    }

    BTreeDatabase* parent = nullptr;
    BlockIndex leafBlockCount = 0;
  };

  LeafBlocksVisitor visitor;
  visitor.parent = this;
  m_impl.forAllNodes(visitor);

  return visitor.leafBlockCount;
}

void BTreeDatabase::commit() {
  WriteLocker writeLocker(m_lock);
  doCommit();
}

void BTreeDatabase::rollback() {
  WriteLocker writeLocker(m_lock);

  m_availableBlocks.clear();
  m_indexCache.clear();
  m_uncommittedWrites.clear();
  m_uncommitted.clear();

  readRoot();

  if (m_device->isWritable())
    m_device->resize(m_deviceSize);
}

void BTreeDatabase::close(bool closeDevice) {
  WriteLocker writeLocker(m_lock);
  if (m_open) {
    if (!tryFlatten())
      doCommit();

    m_indexCache.clear();

    m_open = false;
    if (closeDevice && m_device && m_device->isOpen())
      m_device->close();
  }
}

BTreeDatabase::BlockIndex const BTreeDatabase::InvalidBlockIndex;
std::uint32_t const BTreeDatabase::HeaderSize;
char const* const BTreeDatabase::VersionMagic = "BTreeDB5";
std::uint32_t const BTreeDatabase::VersionMagicSize;
char const* const BTreeDatabase::IndexMagic = "II";
char const* const BTreeDatabase::LeafMagic = "LL";
char const* const BTreeDatabase::FreeIndexMagic = "FF";
std::size_t const BTreeDatabase::BTreeRootSelectorBit;
std::size_t const BTreeDatabase::BTreeRootInfoStart;
std::size_t const BTreeDatabase::BTreeRootInfoSize;

auto BTreeDatabase::IndexNode::pointerCount() const -> std::size_t {
  // If no begin pointer is set then the index is simply uninitialized.
  if (!beginPointer)
    return 0;
  else
    return pointers.size() + 1;
}

auto BTreeDatabase::IndexNode::pointer(std::size_t i) const -> BlockIndex {
  if (i == 0)
    return *beginPointer;
  else
    return pointers.at(i - 1).pointer;
}

void BTreeDatabase::IndexNode::updatePointer(std::size_t i, BlockIndex p) {
  if (i == 0)
    *beginPointer = p;
  else
    pointers.at(i - 1).pointer = p;
}

auto BTreeDatabase::IndexNode::keyBefore(std::size_t i) const -> ByteArray const& {
  return pointers.at(i - 1).key;
}

void BTreeDatabase::IndexNode::updateKeyBefore(std::size_t i, ByteArray k) {
  pointers.at(i - 1).key = k;
}

void BTreeDatabase::IndexNode::removeBefore(std::size_t i) {
  if (i == 0) {
    beginPointer = pointers.at(0).pointer;
    pointers.eraseAt(0);
  } else {
    pointers.eraseAt(i - 1);
  }
}

void BTreeDatabase::IndexNode::insertAfter(std::size_t i, ByteArray k, BlockIndex p) {
  pointers.insertAt(i, Element{.key = k, .pointer = p});
}

auto BTreeDatabase::IndexNode::indexLevel() const -> std::uint8_t {
  return level;
}

void BTreeDatabase::IndexNode::setIndexLevel(std::uint8_t indexLevel) {
  level = indexLevel;
}

void BTreeDatabase::IndexNode::shiftLeft(ByteArray const& mid, IndexNode& right, std::size_t count) {
  count = std::min(right.pointerCount(), count);

  if (count == 0)
    return;

  pointers.append(Element{.key = mid, .pointer = *right.beginPointer});

  auto s = right.pointers.begin();
  std::advance(s, count - 1);
  pointers.insert(pointers.end(), right.pointers.begin(), s);

  right.pointers.erase(right.pointers.begin(), s);
  if (right.pointers.size() != 0) {
    right.beginPointer = right.pointers.at(0).pointer;
    right.pointers.eraseAt(0);
  } else {
    right.beginPointer.reset();
  }
}

void BTreeDatabase::IndexNode::shiftRight(ByteArray const& mid, IndexNode& left, std::size_t count) {
  count = std::min(left.pointerCount(), count);

  if (count == 0)
    return;
  --count;

  pointers.insert(pointers.begin(), Element{.key = mid, .pointer = *beginPointer});

  auto s = left.pointers.begin();
  std::advance(s, left.pointers.size() - count);
  pointers.insert(pointers.begin(), s, left.pointers.end());

  left.pointers.erase(s, left.pointers.end());
  if (left.pointers.size() != 0) {
    beginPointer = left.pointers.at(left.pointers.size() - 1).pointer;
    left.pointers.eraseAt(left.pointers.size() - 1);
  } else {
    beginPointer = std::move(*left.beginPointer);
  }
}

auto BTreeDatabase::IndexNode::split(IndexNode& right, std::size_t i) -> ByteArray {
  auto s = pointers.begin();
  std::advance(s, i - 1);

  right.beginPointer = s->pointer;
  ByteArray midKey = s->key;
  right.level = level;
  ++s;

  right.pointers.insert(right.pointers.begin(), s, pointers.end());
  --s;

  pointers.erase(s, pointers.end());

  return midKey;
}

auto BTreeDatabase::LeafNode::count() const -> std::size_t {
  return elements.size();
}

auto BTreeDatabase::LeafNode::key(std::size_t i) const -> ByteArray const& {
  return elements.at(i).key;
}

auto BTreeDatabase::LeafNode::data(std::size_t i) const -> ByteArray const& {
  return elements.at(i).data;
}

void BTreeDatabase::LeafNode::insert(std::size_t i, ByteArray k, ByteArray d) {
  elements.insertAt(i, Element{.key = std::move(k), .data = std::move(d)});
}

void BTreeDatabase::LeafNode::remove(std::size_t i) {
  elements.eraseAt(i);
}

void BTreeDatabase::LeafNode::shiftLeft(LeafNode& right, std::size_t count) {
  count = std::min(right.count(), count);

  if (count == 0)
    return;

  auto s = right.elements.begin();
  std::advance(s, count);

  elements.insert(elements.end(), right.elements.begin(), s);
  right.elements.erase(right.elements.begin(), s);
}

void BTreeDatabase::LeafNode::shiftRight(LeafNode& left, std::size_t count) {
  count = std::min(left.count(), count);

  if (count == 0)
    return;

  auto s = left.elements.begin();
  std::advance(s, left.elements.size() - count);

  elements.insert(elements.begin(), s, left.elements.end());
  left.elements.erase(s, left.elements.end());
}

void BTreeDatabase::LeafNode::split(LeafNode& right, std::size_t i) {
  auto s = elements.begin();
  std::advance(s, i);

  right.elements.insert(right.elements.begin(), s, elements.end());
  elements.erase(s, elements.end());
}

auto BTreeDatabase::BTreeImpl::rootPointer() -> Pointer {
  return parent->m_root;
}

auto BTreeDatabase::BTreeImpl::rootIsLeaf() -> bool {
  return parent->m_rootIsLeaf;
}

void BTreeDatabase::BTreeImpl::setNewRoot(Pointer pointer, bool isLeaf) {
  parent->m_root = pointer;
  parent->m_rootIsLeaf = isLeaf;

  if (parent->m_autoCommit)
    parent->doCommit();
}

auto BTreeDatabase::BTreeImpl::createIndex(Pointer beginPointer) -> Index {
  auto index = std::make_shared<IndexNode>();
  index->self = InvalidBlockIndex;
  index->level = 0;
  index->beginPointer = beginPointer;
  return index;
}

auto BTreeDatabase::BTreeImpl::loadIndex(Pointer pointer) -> Index {
  SpinLocker lock(parent->m_indexCacheSpinLock);
  if (auto index = parent->m_indexCache.ptr(pointer))
    return *index;
  lock.unlock();

  auto index = std::make_shared<IndexNode>();

  DataStreamBuffer buffer(parent->readBlock(pointer));

  if (buffer.readBytes(2) != ByteArray(IndexMagic, 2))
    throw DBException("Error, incorrect index block signature.");

  index->self = pointer;

  index->level = buffer.read<std::uint8_t>();
  auto s = buffer.read<std::uint32_t>();
  index->beginPointer = buffer.read<BlockIndex>();
  index->pointers.resize(s);
  for (std::uint32_t i = 0; i < s; ++i) {
    auto& e = index->pointers[i];
    e.key = buffer.readBytes(parent->m_keySize);
    e.pointer = buffer.read<BlockIndex>();
  }

  lock.lock();
  parent->m_indexCache.set(pointer, index);
  return index;
}

auto BTreeDatabase::BTreeImpl::indexNeedsShift(Index const& index) -> bool {
  return index->pointerCount() < (parent->maxIndexPointers() + 1) / 2;
}

auto BTreeDatabase::BTreeImpl::indexShift(Index const& left, Key const& mid, Index const& right) -> bool {
  if (left->pointerCount() + right->pointerCount() <= parent->maxIndexPointers()) {
    left->shiftLeft(mid, *right, right->pointerCount());
    return true;
  } else {
    if (indexNeedsShift(right)) {
      right->shiftRight(mid, *left, 1);
      return true;
    } else if (indexNeedsShift(left)) {
      left->shiftLeft(mid, *right, 1);
      return true;
    } else {
      return false;
    }
  }
}

auto BTreeDatabase::BTreeImpl::indexSplit(Index const& index) -> std::optional<std::pair<Key, Index>> {
  if (index->pointerCount() <= parent->maxIndexPointers())
    return {};

  auto right = std::make_shared<IndexNode>();
  right->self = InvalidBlockIndex;
  Key k = index->split(*right, (index->pointerCount() + 1) / 2);
  return make_pair(k, right);
}

auto BTreeDatabase::BTreeImpl::storeIndex(Index index) -> Pointer {
  if (index->self != InvalidBlockIndex) {
    if (!parent->m_uncommitted.contains(index->self)) {
      parent->freeBlock(index->self);
      parent->m_indexCache.remove(index->self);
      index->self = InvalidBlockIndex;
    }
  }

  if (index->self == InvalidBlockIndex)
    index->self = parent->reserveBlock();

  DataStreamBuffer buffer(parent->m_blockSize);
  buffer.writeData(IndexMagic, 2);

  buffer.write<std::uint8_t>(index->level);
  buffer.write<std::uint32_t>(index->pointers.size());
  buffer.write<BlockIndex>(*index->beginPointer);
  for (auto& pointer : index->pointers) {
    buffer.writeBytes(pointer.key);
    buffer.write<BlockIndex>(pointer.pointer);
  }

  parent->updateBlock(index->self, buffer.data());

  parent->m_indexCache.set(index->self, index);
  return index->self;
}

void BTreeDatabase::BTreeImpl::deleteIndex(Index index) {
  parent->m_indexCache.remove(index->self);
  parent->freeBlock(index->self);
}

auto BTreeDatabase::BTreeImpl::createLeaf() -> Leaf {
  auto leaf = std::make_shared<LeafNode>();
  leaf->self = InvalidBlockIndex;
  return leaf;
}

auto BTreeDatabase::BTreeImpl::loadLeaf(Pointer pointer) -> Leaf {
  auto leaf = std::make_shared<LeafNode>();
  leaf->self = pointer;

  BlockIndex currentLeafBlock = leaf->self;
  DataStreamBuffer leafBuffer;
  leafBuffer.reset(parent->m_blockSize);
  parent->readBlock(currentLeafBlock, 0, leafBuffer.ptr(), parent->m_blockSize);

  if (leafBuffer.readBytes(2) != ByteArray(LeafMagic, 2))
    throw DBException("Error, incorrect leaf block signature.");

  DataStreamFunctions leafInput([&](char* data, std::size_t len) -> std::size_t {
    std::size_t pos = 0;
    std::size_t left = len;

    while (left > 0) {
      if (leafBuffer.pos() + left < parent->m_blockSize - sizeof(BlockIndex)) {
        leafBuffer.readData(data + pos, left);
        pos += left;
        left = 0;
      } else {
        std::size_t toRead = parent->m_blockSize - sizeof(BlockIndex) - leafBuffer.pos();
        leafBuffer.readData(data + pos, toRead);
        pos += toRead;
        left -= toRead;
      }

      if (leafBuffer.pos() == (parent->m_blockSize - sizeof(BlockIndex)) && left > 0) {
        currentLeafBlock = leafBuffer.read<BlockIndex>();
        if (currentLeafBlock != InvalidBlockIndex) {
          leafBuffer.reset(parent->m_blockSize);
          parent->readBlock(currentLeafBlock, 0, leafBuffer.ptr(), parent->m_blockSize);

          if (leafBuffer.readBytes(2) != ByteArray(LeafMagic, 2))
            throw DBException("Error, incorrect leaf block signature.");

        } else {
          throw DBException("Leaf read off end of Leaf list.");
        }
      }
    }

    return len;
  },
                                {});

  auto count = leafInput.read<std::uint32_t>();
  leaf->elements.resize(count);
  for (std::uint32_t i = 0; i < count; ++i) {
    auto& element = leaf->elements[i];
    element.key = leafInput.readBytes(parent->m_keySize);
    element.data = leafInput.read<ByteArray>();
  }

  return leaf;
}

auto BTreeDatabase::BTreeImpl::leafNeedsShift(Leaf const& l) -> bool {
  return parent->leafSize(l) < parent->m_blockSize / 2;
}

auto BTreeDatabase::BTreeImpl::leafShift(Leaf& left, Leaf& right) -> bool {
  if (left->count() == 0) {
    left->shiftLeft(*right, right->count());
    return true;
  }

  if (right->count() == 0)
    return true;

  std::uint32_t leftSize = parent->leafSize(left);
  std::uint32_t rightSize = parent->leafSize(right);
  if (leftSize + rightSize < parent->m_blockSize) {
    left->shiftLeft(*right, right->count());
    return true;
  }

  // TODO: Shifting algorithm is bad, could potentially want to shift more
  // than one element here.
  std::uint32_t rightBeginSize = parent->m_keySize + parent->dataSize(right->elements[0].data);
  std::uint32_t leftEndSize = parent->m_keySize + parent->dataSize(left->elements[left->elements.size() - 1].data);
  if (leftSize < rightSize - rightBeginSize && leftSize + rightBeginSize < parent->m_blockSize) {
    left->shiftLeft(*right, 1);
    return true;
  } else if (rightSize < leftSize - leftEndSize && rightSize + leftEndSize < parent->m_blockSize) {
    right->shiftRight(*left, 1);
    return true;
  }

  return false;
}

auto BTreeDatabase::BTreeImpl::leafSplit(Leaf& leaf) -> std::optional<Leaf> {
  if (leaf->elements.size() < 2)
    return {};

  std::uint32_t size = 6;
  bool boundaryFound = false;
  std::uint32_t boundary = 0;
  for (std::uint32_t i = 0; i < leaf->elements.size(); ++i) {
    size += parent->m_keySize;
    size += parent->dataSize(leaf->elements[i].data);
    if (size > parent->m_blockSize - sizeof(BlockIndex) && !boundaryFound) {
      boundary = i;
      boundaryFound = true;
    }
  }
  if (boundary == 0)
    boundary = 1;

  if (size < parent->m_blockSize * 2 - 2 * sizeof(BlockIndex) - 4) {
    return {};
  } else {
    auto right = std::make_shared<LeafNode>();
    right->self = InvalidBlockIndex;
    leaf->split(*right, boundary);
    return right;
  }
}

auto BTreeDatabase::BTreeImpl::storeLeaf(Leaf leaf) -> Pointer {
  if (leaf->self != InvalidBlockIndex) {
    List<BlockIndex> tailBlocks = parent->leafTailBlocks(leaf->self);
    for (unsigned int tailBlock : tailBlocks)
      parent->freeBlock(tailBlock);

    if (!parent->m_uncommitted.contains(leaf->self)) {
      parent->freeBlock(leaf->self);
      leaf->self = InvalidBlockIndex;
    }
  }

  if (leaf->self == InvalidBlockIndex)
    leaf->self = parent->reserveBlock();

  BlockIndex currentLeafBlock = leaf->self;
  DataStreamBuffer leafBuffer;
  leafBuffer.reset(parent->m_blockSize);
  leafBuffer.writeData(LeafMagic, 2);

  DataStreamFunctions leafOutput({}, [&](char const* data, std::size_t len) -> std::size_t {
    std::size_t pos = 0;
    std::size_t left = len;

    while (true) {
      std::size_t toWrite = left;
      if (toWrite > parent->m_blockSize - leafBuffer.pos() - sizeof(BlockIndex))
        toWrite = parent->m_blockSize - leafBuffer.pos() - sizeof(BlockIndex);

      if (toWrite != 0) {
        leafBuffer.writeData(data + pos, toWrite);
        left -= toWrite;
        pos += toWrite;
      }

      if (left == 0)
        break;

      if (leafBuffer.pos() == (parent->m_blockSize - sizeof(BlockIndex))) {
        BlockIndex nextBlock = parent->reserveBlock();
        leafBuffer.write<BlockIndex>(nextBlock);
        parent->updateBlock(currentLeafBlock, leafBuffer.data());
        currentLeafBlock = nextBlock;
        leafBuffer.reset(parent->m_blockSize);
        leafBuffer.writeData(LeafMagic, 2);
      }
    }

    return len;
  });

  leafOutput.write<std::uint32_t>(leaf->elements.size());

  for (auto& element : leaf->elements) {
    leafOutput.writeBytes(element.key);
    leafOutput.write(element.data);
  }

  leafBuffer.seek(parent->m_blockSize - sizeof(BlockIndex));
  leafBuffer.write<BlockIndex>(InvalidBlockIndex);
  parent->updateBlock(currentLeafBlock, leafBuffer.data());

  return leaf->self;
}

void BTreeDatabase::BTreeImpl::deleteLeaf(Leaf leaf) {
  List<BlockIndex> tailBlocks = parent->leafTailBlocks(leaf->self);
  for (unsigned int tailBlock : tailBlocks)
    parent->freeBlock(tailBlock);

  parent->freeBlock(leaf->self);
}

auto BTreeDatabase::BTreeImpl::indexPointerCount(Index const& index) -> std::size_t {
  return index->pointerCount();
}

auto BTreeDatabase::BTreeImpl::indexPointer(Index const& index, std::size_t i) -> Pointer {
  return index->pointer(i);
}

void BTreeDatabase::BTreeImpl::indexUpdatePointer(Index& index, std::size_t i, Pointer p) {
  index->updatePointer(i, p);
}

auto BTreeDatabase::BTreeImpl::indexKeyBefore(Index const& index, std::size_t i) -> Key {
  return index->keyBefore(i);
}

void BTreeDatabase::BTreeImpl::indexUpdateKeyBefore(Index& index, std::size_t i, Key k) {
  index->updateKeyBefore(i, k);
}

void BTreeDatabase::BTreeImpl::indexRemoveBefore(Index& index, std::size_t i) {
  index->removeBefore(i);
}

void BTreeDatabase::BTreeImpl::indexInsertAfter(Index& index, std::size_t i, Key k, Pointer p) {
  index->insertAfter(i, k, p);
}

auto BTreeDatabase::BTreeImpl::indexLevel(Index const& index) -> std::size_t {
  return index->indexLevel();
}

void BTreeDatabase::BTreeImpl::setIndexLevel(Index& index, std::size_t indexLevel) {
  index->setIndexLevel(indexLevel);
}

auto BTreeDatabase::BTreeImpl::leafElementCount(Leaf const& leaf) -> std::size_t {
  return leaf->count();
}

auto BTreeDatabase::BTreeImpl::leafKey(Leaf const& leaf, std::size_t i) -> Key {
  return leaf->key(i);
}

auto BTreeDatabase::BTreeImpl::leafData(Leaf const& leaf, std::size_t i) -> Data {
  return leaf->data(i);
}

void BTreeDatabase::BTreeImpl::leafInsert(Leaf& leaf, std::size_t i, Key k, Data d) {
  leaf->insert(i, std::move(k), std::move(d));
}

void BTreeDatabase::BTreeImpl::leafRemove(Leaf& leaf, std::size_t i) {
  leaf->remove(i);
}

auto BTreeDatabase::BTreeImpl::nextLeaf(Leaf const&) -> std::optional<Pointer> {
  return {};
}

void BTreeDatabase::BTreeImpl::setNextLeaf(Leaf&, std::optional<Pointer>) {}

void BTreeDatabase::readBlock(BlockIndex blockIndex, std::size_t blockOffset, char* block, std::size_t size) const {
  checkBlockIndex(blockIndex);
  rawReadBlock(blockIndex, blockOffset, block, size);
}

auto BTreeDatabase::readBlock(BlockIndex blockIndex) const -> ByteArray {
  ByteArray block(m_blockSize, 0);
  readBlock(blockIndex, 0, block.ptr(), m_blockSize);
  return block;
}

void BTreeDatabase::updateBlock(BlockIndex blockIndex, ByteArray const& block) {
  checkBlockIndex(blockIndex);
  rawWriteBlock(blockIndex, 0, block.ptr(), block.size());
}

void BTreeDatabase::rawReadBlock(BlockIndex blockIndex, std::size_t blockOffset, char* block, std::size_t size) const {
  if (blockOffset > m_blockSize || size > m_blockSize - blockOffset)
    throw DBException::format("Read past end of block, offset: {} size {}", blockOffset, size);

  if (size <= 0)
    return;

  if (auto buffer = m_uncommittedWrites.ptr(blockIndex))
    buffer->copyTo(block, blockOffset, size);
  else
    m_device->readFullAbsolute(HeaderSize + blockIndex * (std::int64_t)m_blockSize + blockOffset, block, size);
}

void BTreeDatabase::rawWriteBlock(BlockIndex blockIndex, std::size_t blockOffset, char const* block, std::size_t size) {
  if (blockOffset > m_blockSize || size > m_blockSize - blockOffset)
    throw DBException::format("Write past end of block, offset: {} size {}", blockOffset, size);

  if (size <= 0)
    return;

  std::int64_t blockStart = HeaderSize + blockIndex * (std::int64_t)m_blockSize;
  auto buffer = m_uncommittedWrites.find(blockIndex);
  if (buffer == m_uncommittedWrites.end())
    buffer = m_uncommittedWrites.emplace(blockIndex, m_device->readBytesAbsolute(blockStart, m_blockSize)).first;

  buffer->second.writeFrom(block, blockOffset, size);
}

auto BTreeDatabase::readFreeIndexBlock(BlockIndex blockIndex) -> FreeIndexBlock {
  checkBlockIndex(blockIndex);

  ByteArray magic(2, 0);
  rawReadBlock(blockIndex, 0, magic.ptr(), 2);
  if (magic != ByteArray(FreeIndexMagic, 2))
    throw DBException::format("Internal exception! block {} missing free index block marker!", blockIndex);

  FreeIndexBlock freeIndexBlock;
  DataStreamBuffer buffer(std::max(sizeof(BlockIndex), (std::size_t)4));

  rawReadBlock(blockIndex, 2, buffer.ptr(), sizeof(BlockIndex));
  buffer.seek(0);
  freeIndexBlock.nextFreeBlock = buffer.read<BlockIndex>();

  rawReadBlock(blockIndex, 2 + sizeof(BlockIndex), buffer.ptr(), 4);
  buffer.seek(0);
  std::size_t numFree = buffer.read<std::uint32_t>();

  for (std::size_t i = 0; i < numFree; ++i) {
    rawReadBlock(blockIndex, 6 + sizeof(BlockIndex) + sizeof(BlockIndex) * i, buffer.ptr(), sizeof(BlockIndex));
    buffer.seek(0);
    freeIndexBlock.freeBlocks.append(buffer.read<BlockIndex>());
  }

  return freeIndexBlock;
}

void BTreeDatabase::writeFreeIndexBlock(BlockIndex blockIndex, FreeIndexBlock indexBlock) {
  checkBlockIndex(blockIndex);

  rawWriteBlock(blockIndex, 0, FreeIndexMagic, 2);
  DataStreamBuffer buffer(std::max(sizeof(BlockIndex), (std::size_t)4));

  buffer.seek(0);
  buffer.write<BlockIndex>(indexBlock.nextFreeBlock);
  rawWriteBlock(blockIndex, 2, buffer.ptr(), sizeof(BlockIndex));

  buffer.seek(0);
  buffer.write<std::uint32_t>(indexBlock.freeBlocks.size());
  rawWriteBlock(blockIndex, 2 + sizeof(BlockIndex), buffer.ptr(), 4);

  for (std::size_t i = 0; i < indexBlock.freeBlocks.size(); ++i) {
    buffer.seek(0);
    buffer.write<BlockIndex>(indexBlock.freeBlocks[i]);
    rawWriteBlock(blockIndex, 6 + sizeof(BlockIndex) + sizeof(BlockIndex) * i, buffer.ptr(), sizeof(BlockIndex));
  }
}

auto BTreeDatabase::leafSize(std::shared_ptr<LeafNode> const& leaf) const -> std::uint32_t {
  std::size_t s = 6;
  for (auto& element : leaf->elements) {
    s += m_keySize;
    s += dataSize(element.data);
  }
  return s;
}

auto BTreeDatabase::maxIndexPointers() const -> std::uint32_t {
  // 2 for magic, 1 byte for level, sizeof(BlockIndex) for beginPointer, 4
  // for size.
  return (m_blockSize - 2 - 1 - sizeof(BlockIndex) - 4) / (m_keySize + sizeof(BlockIndex)) + 1;
}

auto BTreeDatabase::dataSize(ByteArray const& d) const -> std::uint32_t {
  return vlqUSize(d.size()) + d.size();
}

auto BTreeDatabase::leafTailBlocks(BlockIndex leafPointer) -> List<BlockIndex> {
  List<BlockIndex> tailBlocks;
  DataStreamBuffer pointerBuffer(sizeof(BlockIndex));
  while (leafPointer != InvalidBlockIndex) {
    readBlock(leafPointer, m_blockSize - sizeof(BlockIndex), pointerBuffer.ptr(), sizeof(BlockIndex));
    pointerBuffer.seek(0);
    leafPointer = pointerBuffer.read<BlockIndex>();
    if (leafPointer != InvalidBlockIndex)
      tailBlocks.append(leafPointer);
  }
  return tailBlocks;
}

void BTreeDatabase::freeBlock(BlockIndex b) {
  if (m_uncommitted.contains(b))
    m_uncommitted.remove(b);
  if (m_uncommittedWrites.contains(b))
    m_uncommittedWrites.remove(b);

  m_availableBlocks.add(b);
}

auto BTreeDatabase::reserveBlock() -> BlockIndex {
  if (m_availableBlocks.empty()) {
    if (m_headFreeIndexBlock != InvalidBlockIndex) {
      // If available, make available all the blocks in the first free index
      // block.
      FreeIndexBlock indexBlock = readFreeIndexBlock(m_headFreeIndexBlock);
      for (auto const& b : indexBlock.freeBlocks)
        m_availableBlocks.add(b);
      m_availableBlocks.add(m_headFreeIndexBlock);
      m_headFreeIndexBlock = indexBlock.nextFreeBlock;
    }

    if (m_availableBlocks.empty()) {
      // If we still don't have any available blocks, just add a block to the
      // end of the file.
      m_availableBlocks.add(makeEndBlock());
    }
  }

  BlockIndex block = m_availableBlocks.takeFirst();
  m_uncommitted.add(block);
  return block;
}

auto BTreeDatabase::makeEndBlock() -> BlockIndex {
  BlockIndex blockCount = (m_deviceSize - HeaderSize) / m_blockSize;
  m_deviceSize += m_blockSize;
  m_device->resize(m_deviceSize);
  return blockCount;
}

void BTreeDatabase::writeRoot() {
  DataStreamIODevice ds(m_device);
  // First write the root info to whichever section we are not currently using
  ds.seek(BTreeRootInfoStart + (m_usingAltRoot ? 0 : BTreeRootInfoSize));
  ds.write<BlockIndex>(m_headFreeIndexBlock);
  ds.write<std::int64_t>(m_deviceSize);
  ds.write<BlockIndex>(m_root);
  ds.write<bool>(m_rootIsLeaf);

  // Then flush all the pending changes.
  m_device->sync();

  // Then switch headers by writing the single bit that switches them
  m_usingAltRoot = !m_usingAltRoot;
  ds.seek(BTreeRootSelectorBit);
  ds.write(m_usingAltRoot);

  // Then flush this single bit write to make sure it happens before anything
  // else.
  m_device->sync();
}

void BTreeDatabase::readRoot() {
  DataStreamIODevice ds(m_device);
  ds.seek(BTreeRootSelectorBit);
  ds.read(m_usingAltRoot);

  ds.seek(BTreeRootInfoStart + (m_usingAltRoot ? BTreeRootInfoSize : 0));
  m_headFreeIndexBlock = ds.read<BlockIndex>();
  m_deviceSize = ds.read<std::int64_t>();
  m_root = ds.read<BlockIndex>();
  m_rootIsLeaf = ds.read<bool>();
}

void BTreeDatabase::doCommit() {
  if (m_availableBlocks.empty() && m_uncommitted.empty())
    return;

  if (!m_availableBlocks.empty()) {
    // First, read the existing head FreeIndexBlock, if it exists
    FreeIndexBlock indexBlock = FreeIndexBlock{.nextFreeBlock = InvalidBlockIndex, .freeBlocks = {}};

    auto newBlock = [&]() -> BlockIndex {
      if (!m_availableBlocks.empty())
        return m_availableBlocks.takeFirst();
      else
        return makeEndBlock();
    };

    if (m_headFreeIndexBlock != InvalidBlockIndex)
      indexBlock = readFreeIndexBlock(m_headFreeIndexBlock);
    else
      m_headFreeIndexBlock = newBlock();

    // Then, we need to write all the available blocks to the FreeIndexBlock chain.
    while (true) {
      // If we have room on our current FreeIndexBlock, just add a block to it.
      if (!m_availableBlocks.empty() && indexBlock.freeBlocks.size() < maxFreeIndexLength()) {
        BlockIndex toAdd = m_availableBlocks.takeFirst();
        indexBlock.freeBlocks.append(toAdd);
      } else {
        // Update the current head free index block.
        writeFreeIndexBlock(m_headFreeIndexBlock, indexBlock);

        // If we're out of blocks to free, then we're done
        if (m_availableBlocks.empty())
          break;

        // If our head free index block is full, then
        // need to write a new head free index block.
        if (indexBlock.freeBlocks.size() >= maxFreeIndexLength()) {
          indexBlock.nextFreeBlock = m_headFreeIndexBlock;
          indexBlock.freeBlocks.clear();

          m_headFreeIndexBlock = newBlock();
          writeFreeIndexBlock(m_headFreeIndexBlock, indexBlock);
        }
      }
    }
  }

  commitWrites();
  writeRoot();
  m_uncommitted.clear();
}

void BTreeDatabase::commitWrites() {
  for (auto& write : m_uncommittedWrites)
    m_device->writeFullAbsolute(HeaderSize + write.first * (std::int64_t)m_blockSize, write.second.ptr(), m_blockSize);

  m_device->sync();
  m_uncommittedWrites.clear();
}

auto BTreeDatabase::tryFlatten() -> bool {
  if (m_headFreeIndexBlock == InvalidBlockIndex || m_rootIsLeaf || !m_device->isWritable())
    return false;

  BlockIndex freeBlockCount = 0;
  BlockIndex indexBlockIndex = m_headFreeIndexBlock;
  while (indexBlockIndex != InvalidBlockIndex) {
    FreeIndexBlock indexBlock = readFreeIndexBlock(indexBlockIndex);
    freeBlockCount += 1 + indexBlock.freeBlocks.size();
    indexBlockIndex = indexBlock.nextFreeBlock;
  }

  BlockIndex expectedBlockCount = (m_deviceSize - HeaderSize) / m_blockSize;
  float free = float(freeBlockCount) / float(expectedBlockCount);
  if (free < 0.05f)
    return false;

  Logger::info("[BTreeDatabase] File '{}' is {:.2f}% free space, flattening", m_device->deviceName(), free * 100.f);

  indexBlockIndex = m_headFreeIndexBlock;
  {
    List<BlockIndex> availableBlocksList;
    do {
      FreeIndexBlock indexBlock = readFreeIndexBlock(indexBlockIndex);
      availableBlocksList.appendAll(indexBlock.freeBlocks);
      availableBlocksList.append(indexBlockIndex);
      indexBlockIndex = indexBlock.nextFreeBlock;
    } while (indexBlockIndex != InvalidBlockIndex);
    m_headFreeIndexBlock = InvalidBlockIndex;

    sort(availableBlocksList);
    for (auto& availableBlock : availableBlocksList)
      m_availableBlocks.insert(m_availableBlocks.end(), availableBlock);
  }

  BlockIndex count = 1;// 1 to include root index

  double start = Time::monotonicTime();
  auto index = m_impl.loadIndex(m_impl.rootPointer());
  if (flattenVisitor(index, count)) {
    m_impl.deleteIndex(index);
    index->self = InvalidBlockIndex;
    m_root = m_impl.storeIndex(index);
  }

  m_availableBlocks.clear();
  m_device->resize(m_deviceSize = HeaderSize + (std::int64_t)m_blockSize * count);

  m_indexCache.clear();
  commitWrites();
  writeRoot();
  m_uncommitted.clear();

  Logger::info("[BTreeDatabase] Finished flattening '{}' in {:.2f} milliseconds", m_device->deviceName(), (Time::monotonicTime() - start) * 1000.f);
  return true;
}

auto BTreeDatabase::flattenVisitor(BTreeImpl::Index& index, BlockIndex& count) -> bool {
  auto pointerCount = index->pointerCount();
  count += pointerCount;
  bool canStore = !m_availableBlocks.empty();

  bool needsStore = false;
  if (m_impl.indexLevel(index) == 0) {
    for (std::size_t i = 0; i != pointerCount; ++i) {
      auto indexPointer = index->pointer(i);
      auto tailBlocks = leafTailBlocks(indexPointer);
      if (canStore) {
        bool leafNeedsStore = m_availableBlocks.first() < indexPointer;

        if (!leafNeedsStore)
          for (std::size_t i = 0; !leafNeedsStore && i != tailBlocks.size(); ++i)
            if (m_availableBlocks.first() < tailBlocks[i])
              leafNeedsStore = true;

        if (leafNeedsStore) {
          auto leaf = m_impl.loadLeaf(indexPointer);
          m_impl.deleteLeaf(leaf);
          leaf->self = InvalidBlockIndex;
          index->updatePointer(i, m_impl.storeLeaf(leaf));
          tailBlocks = leafTailBlocks(leaf->self);
          needsStore = true;
        }
        canStore = !m_availableBlocks.empty();
      }
      count += tailBlocks.size();
    }
  } else {
    for (std::size_t i = 0; i != pointerCount; ++i) {
      auto childIndex = m_impl.loadIndex(index->pointer(i));
      if (canStore && flattenVisitor(childIndex, count)) {
        m_impl.deleteIndex(childIndex);
        childIndex->self = InvalidBlockIndex;
        index->updatePointer(i, m_impl.storeIndex(childIndex));
        canStore = !m_availableBlocks.empty();
        needsStore = true;
      }
    }
  }
  return needsStore || (canStore && m_availableBlocks.first() < index->self);
}

void BTreeDatabase::checkIfOpen(char const* methodName, bool shouldBeOpen) const {
  if (shouldBeOpen && !m_open)
    throw DBException::format("BTreeDatabase method '{}' called when not open, must be open.", methodName);
  else if (!shouldBeOpen && m_open)
    throw DBException::format("BTreeDatabase method '{}' called when open, cannot call when open.", methodName);
}

void BTreeDatabase::checkBlockIndex(std::size_t blockIndex) const {
  BlockIndex blockCount = (m_deviceSize - HeaderSize) / m_blockSize;
  if (blockIndex >= blockCount)
    throw DBException::format("blockIndex: {} out of block range", blockIndex);
}

void BTreeDatabase::checkKeySize(ByteArray const& k) const {
  if (k.size() != m_keySize)
    throw DBException::format("Wrong key size {}", k.size());
}

auto BTreeDatabase::maxFreeIndexLength() const -> std::uint32_t {
  return (m_blockSize / sizeof(BlockIndex)) - 2 - sizeof(BlockIndex) - 4;
}

BTreeSha256Database::BTreeSha256Database() {
  setKeySize(32);
}

BTreeSha256Database::BTreeSha256Database(String const& contentIdentifier) {
  setKeySize(32);
  setContentIdentifier(contentIdentifier);
}

auto BTreeSha256Database::contains(ByteArray const& key) -> bool {
  return BTreeDatabase::contains(sha256(key));
}

auto BTreeSha256Database::find(ByteArray const& key) -> std::optional<ByteArray> {
  return BTreeDatabase::find(sha256(key));
}

auto BTreeSha256Database::insert(ByteArray const& key, ByteArray const& value) -> bool {
  return BTreeDatabase::insert(sha256(key), value);
}

auto BTreeSha256Database::remove(ByteArray const& key) -> bool {
  return BTreeDatabase::remove(sha256(key));
}

auto BTreeSha256Database::contains(String const& key) -> bool {
  return BTreeDatabase::contains(sha256(key));
}

auto BTreeSha256Database::find(String const& key) -> std::optional<ByteArray> {
  return BTreeDatabase::find(sha256(key));
}

auto BTreeSha256Database::insert(String const& key, ByteArray const& value) -> bool {
  return BTreeDatabase::insert(sha256(key), value);
}

auto BTreeSha256Database::remove(String const& key) -> bool {
  return BTreeDatabase::remove(sha256(key));
}

}// namespace Star
