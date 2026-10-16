import star.block_allocator;
import std;
import star.map;

#include "gtest/gtest.h"

using namespace Star;

TEST(BlockAllocatorTest, All) {
  HashMap<int, int, std::hash<int>, std::equal_to<int>, BlockAllocator<std::pair<int const, int>, 4096>> testMap;
  testMap.reserve(32768);
  for (std::size_t i = 0; i < 65536; ++i)
    testMap[i] = i;

  for (auto const& p : testMap)
    EXPECT_EQ(p.first, p.second);
}

namespace {
template <typename T>
void exerciseBlockReuse() {
  BlockAllocator<T, 4> allocator;
  auto sharedAllocator = allocator;
  std::array<T*, 160> pointers;
  // More than the initial block-vector capacity, with both full and partial
  // blocks participating in address lookup and freelist reuse.
  for (std::size_t i = 0; i < pointers.size(); ++i) {
    pointers[i] = allocator.allocate(1);
    allocator.construct(pointers[i], T{static_cast<unsigned char>(i)});
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(pointers[i]) % alignof(T), 0u);
  }
  for (std::size_t i = 1; i < pointers.size(); i += 2) {
    sharedAllocator.destroy(pointers[i]);
    sharedAllocator.deallocate(pointers[i], 1);
  }
  for (std::size_t i = 1; i < pointers.size(); i += 2) {
    pointers[i] = allocator.allocate(1);
    allocator.construct(pointers[i], T{static_cast<unsigned char>(i)});
  }
  for (std::size_t i = 0; i < pointers.size(); ++i)
    EXPECT_EQ(pointers[i]->value, static_cast<unsigned char>(i));
  for (std::size_t i = 0; i < pointers.size(); ++i) {
    auto index = i * 37 % pointers.size();
    sharedAllocator.destroy(pointers[index]);
    sharedAllocator.deallocate(pointers[index], 1);
  }
  auto reused = allocator.allocate(1);
  allocator.construct(reused, T{42});
  EXPECT_EQ(reused->value, 42);
  allocator.destroy(reused);
  allocator.deallocate(reused, 1);
}
}

TEST(BlockAllocatorTest, ChunkStrideAndBlockLookup) {
  struct Tiny {
    unsigned char value;
  };
  struct alignas(64) Aligned {
    unsigned char value;
  };
  exerciseBlockReuse<Tiny>();
  exerciseBlockReuse<Aligned>();
}

TEST(BlockAllocatorTest, MultipleElementFallback) {
  BlockAllocator<int, 4> allocator;
  auto elements = allocator.allocate(3);
  for (int i = 0; i < 3; ++i)
    allocator.construct(elements + i, i + 10);
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(elements[i], i + 10);
    allocator.destroy(elements + i);
  }
  allocator.deallocate(elements, 3);
}
