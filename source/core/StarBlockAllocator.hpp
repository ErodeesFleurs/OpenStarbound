#pragma once

import std;

namespace Star {

// Constant size only allocator using fixed size blocks of memory.  much faster
// than general purpose allocators, but not thread safe.  Useful as the
// allocator for containers that mostly allocate one element at a time, such as
// std::list, std::map, std::set etc.
template <typename T, std::size_t BlockSize>
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

  template <class U>
  struct rebind {
    using other = BlockAllocator<U, BlockSize>;
  };

  BlockAllocator();
  // Copy constructed BlockAllocators of the same type share underlying
  // resources.
  BlockAllocator(BlockAllocator const& other) = default;
  BlockAllocator(BlockAllocator&& other) = default;
  // Copy constructed BlockAllocators of different type share no resources
  template <class U>
  BlockAllocator(BlockAllocator<U, BlockSize> const& other);

  auto operator=(BlockAllocator const& rhs) -> BlockAllocator& = default;
  auto operator=(BlockAllocator&& rhs) -> BlockAllocator& = default;

  // If n is != 1, will fall back on std::allocator<T>
  auto allocate(std::size_t n) -> T*;
  void deallocate(T* p, std::size_t n);

  template <typename... Args>
  void construct(pointer p, Args&&... args) const;
  void destroy(pointer p) const;

  // BlockAllocator will always be != to any other BlockAllocator instance
  template <class U>
  auto operator==(BlockAllocator<U, BlockSize> const& rhs) const -> bool;
  template <class U>
  auto operator!=(BlockAllocator<U, BlockSize> const& rhs) const -> bool;

private:
  template <typename OtherT, std::size_t OtherBlockSize>
  friend class BlockAllocator;

  using ChunkIndex =
    std::conditional_t<BlockSize <= std::numeric_limits<std::uint8_t>::max(), std::uint8_t,
                       std::conditional_t<BlockSize <= std::numeric_limits<std::uint16_t>::max(), std::uint16_t,
                                          std::conditional_t<BlockSize <= std::numeric_limits<std::uint32_t>::max(), std::uint32_t,
                                                             std::conditional_t<BlockSize <= std::numeric_limits<std::uint64_t>::max(), std::uint64_t, std::uintmax_t>>>>;

  static ChunkIndex const NullChunkIndex = std::numeric_limits<ChunkIndex>::max();

  struct Unallocated {
    ChunkIndex prev;
    ChunkIndex next;
  };

  struct alignas(T) alignas(Unallocated) Chunk {
    static constexpr std::size_t ChunkSize = std::max(sizeof(T), sizeof(Unallocated));
    std::array<std::byte, ChunkSize> data;
  };

  struct Block {
    auto allocate() -> T*;
    void deallocate(T* ptr);

    [[nodiscard]] auto full() const -> bool;
    [[nodiscard]] auto empty() const -> bool;

    auto chunkPointer(ChunkIndex chunkIndex) -> Chunk*;

    std::array<Chunk, BlockSize> chunks;
    ChunkIndex firstUnallocated = NullChunkIndex;
    ChunkIndex allocationCount = 0;
  };

  struct Data {
    std::vector<std::unique_ptr<Block>> blocks;
    Block* unfilledBlock;
    std::allocator<T> multiAllocator;
  };

  using BlockAllocatorFamily = std::unordered_map<std::type_index, std::shared_ptr<void>>;

  static auto getAllocatorData(BlockAllocatorFamily& family) -> Data*;

  std::shared_ptr<BlockAllocatorFamily> m_family;
  Data* m_data;
};

template <typename T, std::size_t BlockSize>
BlockAllocator<T, BlockSize>::BlockAllocator() {
  m_family = std::make_shared<BlockAllocatorFamily>();
  m_data = getAllocatorData(*m_family);
  m_data->blocks.reserve(32);
  m_data->unfilledBlock = nullptr;
}

template <typename T, std::size_t BlockSize>
template <class U>
BlockAllocator<T, BlockSize>::BlockAllocator(BlockAllocator<U, BlockSize> const& other)
    : m_family(other.m_family) {
  m_data = getAllocatorData(*m_family);
}

template <typename T, std::size_t BlockSize>
auto BlockAllocator<T, BlockSize>::allocate(std::size_t n) -> T* {
  if (n == 1) {
    if (m_data->unfilledBlock == nullptr) {
      for (auto const& p : m_data->blocks) {
        if (!p->full()) {
          m_data->unfilledBlock = p.get();
          break;
        }
      }

      if (!m_data->unfilledBlock) {
        auto block = std::make_unique<Block>();
        m_data->unfilledBlock = block.get();
        auto sortedPosition = std::lower_bound(m_data->blocks.begin(), m_data->blocks.end(), block.get(), [](std::unique_ptr<Block> const& a, Block* b) -> auto {
          return a.get() < b;
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

template <typename T, std::size_t BlockSize>
void BlockAllocator<T, BlockSize>::deallocate(T* p, std::size_t n) {
  if (n == 1) {

    auto i = std::upper_bound(m_data->blocks.begin(), m_data->blocks.end(), p, [](T* a, std::unique_ptr<Block> const& b) -> auto {
      return a < (T*)b->chunkPointer(0);
    });

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

template <typename T, std::size_t BlockSize>
template <typename... Args>
void BlockAllocator<T, BlockSize>::construct(pointer p, Args&&... args) const {
  new (p) T(std::forward<Args>(args)...);
}

template <typename T, std::size_t BlockSize>
void BlockAllocator<T, BlockSize>::destroy(pointer p) const {
  p->~T();
}

template <typename T, std::size_t BlockSize>
template <class U>
auto BlockAllocator<T, BlockSize>::operator==(BlockAllocator<U, BlockSize> const& rhs) const -> bool {
  return m_family == rhs.m_family;
}

template <typename T, std::size_t BlockSize>
template <class U>
auto BlockAllocator<T, BlockSize>::operator!=(BlockAllocator<U, BlockSize> const& rhs) const -> bool {
  return m_family != rhs.m_family;
}

template <typename T, std::size_t BlockSize>
auto BlockAllocator<T, BlockSize>::Block::allocate() -> T* {

  T* allocated;
  if (firstUnallocated == NullChunkIndex) {
    allocated = (T*)chunkPointer(allocationCount);
  } else {
    void* chunk = chunkPointer(firstUnallocated);
    firstUnallocated = ((Unallocated*)chunk)->next;
    if (firstUnallocated != NullChunkIndex)
      ((Unallocated*)chunkPointer(firstUnallocated))->prev = NullChunkIndex;
    allocated = (T*)chunk;
  }

  ++allocationCount;
  return allocated;
}

template <typename T, std::size_t BlockSize>
void BlockAllocator<T, BlockSize>::Block::deallocate(T* ptr) {

  ChunkIndex chunkIndex = ptr - (T*)chunkPointer(0);

  auto c = (Unallocated*)chunkPointer(chunkIndex);
  c->prev = NullChunkIndex;
  c->next = firstUnallocated;
  if (firstUnallocated != NullChunkIndex)
    ((Unallocated*)chunkPointer(firstUnallocated))->prev = chunkIndex;
  firstUnallocated = chunkIndex;
  --allocationCount;
}

template <typename T, std::size_t BlockSize>
auto BlockAllocator<T, BlockSize>::Block::full() const -> bool {
  return allocationCount == BlockSize;
}

template <typename T, std::size_t BlockSize>
auto BlockAllocator<T, BlockSize>::Block::empty() const -> bool {
  return allocationCount == 0;
}

template <typename T, std::size_t BlockSize>
auto BlockAllocator<T, BlockSize>::Block::chunkPointer(ChunkIndex chunkIndex) -> Chunk* {
  return &chunks[chunkIndex];
}

template <typename T, std::size_t BlockSize>
auto BlockAllocator<T, BlockSize>::getAllocatorData(BlockAllocatorFamily& family) -> typename BlockAllocator<T, BlockSize>::Data* {
  auto& dataptr = family[typeid(Data)];
  if (!dataptr)
    dataptr = std::make_shared<Data>();
  return (Data*)dataptr.get();
}

}// namespace Star
