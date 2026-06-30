#pragma once

#include <array>
#include <functional>
#include <vector>
#include <unordered_map>
#include <limits>
#include <memory>
#include <typeindex>

#include "StarException.hpp"

namespace Star {

// Constant size only allocator using fixed size blocks of memory.  much faster
// than general purpose allocators, but not thread safe.  Useful as the
// allocator for containers that mostly allocate one element at a time, such as
// std::list, std::map, std::set etc.
template <typename T, size_t BlockSize>
class BlockAllocator {
public:
  using value_type = T;

  using pointer = T*;
  using const_pointer = T const*;

  using reference = T&;
  using const_reference = T const&;

  // Allocator can be shared, but since it is NOT thread safe this should not
  // be done by default.
  using propagate_on_container_copy_assignment = std::false_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;

  static constexpr size_t DefaultBlockPreallocation = 32;

  template <class U>
  struct rebind {
    using other = BlockAllocator<U, BlockSize>;
  };

  BlockAllocator();
  // Copy constructed BlockAllocators of the same type share underlying
  // resources.
  BlockAllocator(BlockAllocator const& other) = default;
  BlockAllocator(BlockAllocator&& other) noexcept = default;
  // Copy constructed BlockAllocators of different type share no resources
  template <class U>
  BlockAllocator(BlockAllocator<U, BlockSize> const& other);

  BlockAllocator& operator=(BlockAllocator const& rhs) = default;
  BlockAllocator& operator=(BlockAllocator&& rhs) noexcept = default;

  // If n is != 1, will fall back on std::allocator<T>
  T* allocate(size_t n);
  void deallocate(T* p, size_t n);

  template <typename... Args>
  void construct(pointer p, Args&&... args) const;
  void destroy(pointer p) const;

  // BlockAllocator will always be != to any other BlockAllocator instance
  template <class U>
  bool operator==(BlockAllocator<U, BlockSize> const& rhs) const;
  template <class U>
  bool operator!=(BlockAllocator<U, BlockSize> const& rhs) const;

private:
  template <typename OtherT, size_t OtherBlockSize>
  friend class BlockAllocator;

  using ChunkIndex =
    std::conditional_t<BlockSize <= std::numeric_limits<uint8_t>::max(), uint8_t,
      std::conditional_t<BlockSize <= std::numeric_limits<uint16_t>::max(), uint16_t,
        std::conditional_t<BlockSize <= std::numeric_limits<uint32_t>::max(), uint32_t,
          std::conditional_t<BlockSize <= std::numeric_limits<uint64_t>::max(), uint64_t, uintmax_t>>>>;

  static ChunkIndex const NullChunkIndex = std::numeric_limits<ChunkIndex>::max();

  struct Unallocated {
    ChunkIndex prev;
    ChunkIndex next;
  };

  static constexpr size_t ChunkSize = sizeof(T) > sizeof(Unallocated) ? sizeof(T) : sizeof(Unallocated);
  static constexpr size_t ChunkAlignment = alignof(T) > alignof(Unallocated) ? alignof(T) : alignof(Unallocated);

  struct alignas(ChunkAlignment) Chunk {
    unsigned char storage[ChunkSize];
  };

  struct Block {
    T* allocate();
    void deallocate(T* ptr);

    bool full() const;
    bool empty() const;

    void const* chunkStorage(ChunkIndex chunkIndex) const;
    void* chunkStorage(ChunkIndex chunkIndex);
    T* valuePointer(ChunkIndex chunkIndex);
    Unallocated* unallocatedPointer(ChunkIndex chunkIndex);
    ChunkIndex chunkIndexFor(T* ptr) const;

    std::array<Chunk, BlockSize> chunks;
    ChunkIndex firstUnallocated = NullChunkIndex;
    ChunkIndex allocationCount = 0;
  };

  struct Data {
    std::vector<unique_ptr<Block>> blocks;
    Block* unfilledBlock;
    std::allocator<T> multiAllocator;
  };

  using BlockAllocatorFamily = std::unordered_map<std::type_index, shared_ptr<void>>;

  static Data* getAllocatorData(BlockAllocatorFamily& family);

  shared_ptr<BlockAllocatorFamily> m_family;
  Data* m_data;
};

template <typename T, size_t BlockSize>
BlockAllocator<T, BlockSize>::BlockAllocator() {
  m_family = make_shared<BlockAllocatorFamily>();
  m_data = getAllocatorData(*m_family);
  m_data->blocks.reserve(DefaultBlockPreallocation);
  m_data->unfilledBlock = nullptr;
}

template <typename T, size_t BlockSize>
template <class U>
BlockAllocator<T, BlockSize>::BlockAllocator(BlockAllocator<U, BlockSize> const& other)
  : m_family(other.m_family) {
  m_data = getAllocatorData(*m_family);
}

template <typename T, size_t BlockSize>
T* BlockAllocator<T, BlockSize>::allocate(size_t n) {
  if (n == 1) {
    if (m_data->unfilledBlock == nullptr) {
      for (auto const& p : m_data->blocks) {
        if (!p->full()) {
          m_data->unfilledBlock = p.get();
          break;
        }
      }

      if (!m_data->unfilledBlock) {
        auto block = make_unique<Block>();
        m_data->unfilledBlock = block.get();
        auto sortedPosition = std::lower_bound(m_data->blocks.begin(), m_data->blocks.end(), block.get(), [](std::unique_ptr<Block> const& a, Block* b) {
            return std::less<void const*>()(a->chunkStorage(0), b->chunkStorage(0));
          });
        m_data->blocks.insert(sortedPosition, std::move(block));
      }
    }

    auto allocated = m_data->unfilledBlock->allocate();
    if (m_data->unfilledBlock->full())
      m_data->unfilledBlock = nullptr;
    return allocated;
  } else {
    return m_data->multiAllocator.allocate(n);
  }
}

template <typename T, size_t BlockSize>
void BlockAllocator<T, BlockSize>::deallocate(T* p, size_t n) {
  if (n == 1) {
    starAssert(p);

    auto i = std::upper_bound(m_data->blocks.begin(), m_data->blocks.end(), p, [](T* a, std::unique_ptr<Block> const& b) {
        return std::less<void const*>()(a, b->chunkStorage(0));
      });

    starAssert(i != m_data->blocks.begin());
    --i;

    (*i)->deallocate(p);

    if (!m_data->unfilledBlock) {
      m_data->unfilledBlock = i->get();
    } else if ((*i)->empty()) {
      if (m_data->unfilledBlock != i->get())
        m_data->blocks.erase(i);
    }
  } else {
    m_data->multiAllocator.deallocate(p, n);
  }
}

template <typename T, size_t BlockSize>
template <typename... Args>
void BlockAllocator<T, BlockSize>::construct(pointer p, Args&&... args) const {
  std::construct_at(p, std::forward<Args>(args)...);
}

template <typename T, size_t BlockSize>
void BlockAllocator<T, BlockSize>::destroy(pointer p) const {
  std::destroy_at(p);
}

template <typename T, size_t BlockSize>
template <class U>
bool BlockAllocator<T, BlockSize>::operator==(BlockAllocator<U, BlockSize> const& rhs) const {
  return m_family == rhs.m_family;
}

template <typename T, size_t BlockSize>
template <class U>
bool BlockAllocator<T, BlockSize>::operator!=(BlockAllocator<U, BlockSize> const& rhs) const {
  return m_family != rhs.m_family;
}

template <typename T, size_t BlockSize>
T* BlockAllocator<T, BlockSize>::Block::allocate() {
  starAssert(allocationCount < BlockSize);

  T* allocated;
  if (firstUnallocated == NullChunkIndex) {
    allocated = valuePointer(allocationCount);
  } else {
    auto unallocated = unallocatedPointer(firstUnallocated);
    starAssert(unallocated->prev == NullChunkIndex);
    allocated = valuePointer(firstUnallocated);
    firstUnallocated = unallocated->next;
    if (firstUnallocated != NullChunkIndex)
      unallocatedPointer(firstUnallocated)->prev = NullChunkIndex;
  }

  ++allocationCount;
  return allocated;
}

template <typename T, size_t BlockSize>
void BlockAllocator<T, BlockSize>::Block::deallocate(T* ptr) {
  starAssert(allocationCount > 0);

  ChunkIndex chunkIndex = chunkIndexFor(ptr);
  starAssert(valuePointer(chunkIndex) == ptr);

  auto c = unallocatedPointer(chunkIndex);
  c->prev = NullChunkIndex;
  c->next = firstUnallocated;
  if (firstUnallocated != NullChunkIndex)
    unallocatedPointer(firstUnallocated)->prev = chunkIndex;
  firstUnallocated = chunkIndex;
  --allocationCount;
}

template <typename T, size_t BlockSize>
bool BlockAllocator<T, BlockSize>::Block::full() const {
  return allocationCount == BlockSize;
}

template <typename T, size_t BlockSize>
bool BlockAllocator<T, BlockSize>::Block::empty() const {
  return allocationCount == 0;
}

template <typename T, size_t BlockSize>
void const* BlockAllocator<T, BlockSize>::Block::chunkStorage(ChunkIndex chunkIndex) const {
  starAssert(chunkIndex < BlockSize);
  return chunks[chunkIndex].storage;
}

template <typename T, size_t BlockSize>
void* BlockAllocator<T, BlockSize>::Block::chunkStorage(ChunkIndex chunkIndex) {
  starAssert(chunkIndex < BlockSize);
  return chunks[chunkIndex].storage;
}

template <typename T, size_t BlockSize>
T* BlockAllocator<T, BlockSize>::Block::valuePointer(ChunkIndex chunkIndex) {
  return std::launder(reinterpret_cast<T*>(chunkStorage(chunkIndex)));
}

template <typename T, size_t BlockSize>
auto BlockAllocator<T, BlockSize>::Block::unallocatedPointer(ChunkIndex chunkIndex) -> Unallocated* {
  return std::launder(reinterpret_cast<Unallocated*>(chunkStorage(chunkIndex)));
}

template <typename T, size_t BlockSize>
auto BlockAllocator<T, BlockSize>::Block::chunkIndexFor(T* ptr) const -> ChunkIndex {
  auto first = static_cast<unsigned char const*>(chunkStorage(0));
  auto current = reinterpret_cast<unsigned char const*>(ptr);
  auto offset = current - first;
  starAssert(offset >= 0);
  starAssert(offset % sizeof(Chunk) == 0);
  auto index = offset / sizeof(Chunk);
  starAssert(index < BlockSize);
  return static_cast<ChunkIndex>(index);
}

template <typename T, size_t BlockSize>
typename BlockAllocator<T, BlockSize>::Data* BlockAllocator<T, BlockSize>::getAllocatorData(BlockAllocatorFamily& family) {
  auto& dataptr = family[typeid(Data)];
  if (!dataptr)
    dataptr = make_shared<Data>();
  return static_cast<Data*>(dataptr.get());
}

}
