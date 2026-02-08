# C++26 Modernization Analysis for OpenStarbound

## Executive Summary

OpenStarbound is already using C++26 with the module system (`import std`), but contains many custom wrappers that duplicate standard library functionality. This document analyzes redundant wrappers and proposes replacements with modern C++ standard library features.

## Current State

- **C++ Version**: C++26 (CMAKE_CXX_STANDARD 26)
- **Modules**: Enabled (CMAKE_CXX_MODULE_STD ON, using `import std`)
- **Total Source Files**: 1127 C++ files
- **Core Utilities**: ~170+ files in `source/core/`

## Redundant Wrappers Analysis

### 1. Optional Type Wrapper - `Maybe<T>` ❌ REDUNDANT

**Current Implementation**: `source/core/StarMaybe.hpp`
- Wraps `std::optional<T>` internally
- Adds custom methods: `isValid()`, `isNothing()`, `ptr()`, `value()`, `take()`, `put()`, `apply()`, `sequence()`
- **Usage**: ~440 occurrences in codebase

**Standard Library Equivalent**: `std::optional<T>` (C++17)
```cpp
// Current (redundant)
Maybe<int> value = 42;
if (value.isValid()) { ... }

// Standard library
std::optional<int> value = 42;
if (value.has_value()) { ... }  // or just: if (value)
```

**Recommendation**: 
- ✅ **Replace with `std::optional<T>` directly**
- Custom methods like `apply()` and `sequence()` can be standalone functions using standard algorithms
- The codebase already uses `std::optional` internally in 254+ places

---

### 2. Mutex Wrappers ❌ PARTIALLY REDUNDANT

**Current Implementation**: `source/core/StarThread.hpp`

#### 2.1 `Mutex` - Wraps `std::mutex` with `std::unique_ptr`
```cpp
class Mutex {
  std::unique_ptr<std::mutex> m_mutex;
  void lock() { m_mutex->lock(); }
  void unlock() { m_mutex->unlock(); }
  bool tryLock() { return m_mutex->try_lock(); }
};
```

**Issue**: Unnecessary indirection via unique_ptr. `std::mutex` is already movable in C++11+.

**Recommendation**: ✅ **Use `std::mutex` directly**

---

#### 2.2 `RecursiveMutex` - Wraps `std::recursive_mutex` with `std::unique_ptr`
```cpp
class RecursiveMutex {
  std::unique_ptr<std::recursive_mutex> m_mutex;
  void lock() { m_mutex->lock(); }
  void unlock() { m_mutex->unlock(); }
  bool tryLock() { return m_mutex->try_lock(); }
};
```

**Recommendation**: ✅ **Use `std::recursive_mutex` directly**

---

#### 2.3 `ConditionVariable` - Wraps `std::condition_variable` with `std::unique_ptr`
```cpp
class ConditionVariable {
  std::unique_ptr<std::condition_variable> m_cv;
  void wait(Mutex& mutex, std::optional<unsigned> millis = {});
  void signal() { m_cv->notify_one(); }
  void broadcast() { m_cv->notify_all(); }
};
```

**Recommendation**: ✅ **Use `std::condition_variable` directly**

---

#### 2.4 `ReadersWriterMutex` - Wraps `std::shared_mutex` with `std::unique_ptr`
```cpp
class ReadersWriterMutex {
  std::unique_ptr<std::shared_mutex> m_mutex;
  void readLock() { m_mutex->lock_shared(); }
  void writeLock() { m_mutex->lock(); }
  // ...
};
```

**Recommendation**: ✅ **Use `std::shared_mutex` directly** (C++17)

---

### 3. RAII Lock Guards ❌ REDUNDANT

**Current Implementation**: Custom RAII wrappers
- **Usage**: 
  - `MutexLocker` / `MLocker<Mutex>`: ~531 occurrences
  - `RecursiveMutexLocker` / `MLocker<RecursiveMutex>`: included in above
  - `ReadLocker`: ~59 occurrences (estimate from 118 total)
  - `WriteLocker`: ~59 occurrences (estimate from 118 total)
  - `SpinLocker` / `MLocker<SpinLock>`: included in above

#### 3.1 `MLocker<T>` - Custom RAII lock guard
```cpp
template <typename MutexType>
class MLocker {
  MutexType& m_mutex;
  bool m_locked{};
public:
  explicit MLocker(MutexType& ref, bool lock = true);
  ~MLocker() { unlock(); }
  void lock();
  void unlock();
  bool tryLock();
  bool isLocked() const;
};
```

**Standard Library Equivalent**: 
- `std::lock_guard<T>` - Simple RAII (C++11)
- `std::unique_lock<T>` - RAII with manual lock/unlock (C++11)
- `std::scoped_lock<T...>` - Multi-mutex RAII (C++17)

**Recommendation**: ✅ **Replace with standard lock guards**
```cpp
// Current
MutexLocker locker(mutex);

// Standard library
std::lock_guard<std::mutex> lock(mutex);
// or if manual control needed:
std::unique_lock<std::mutex> lock(mutex);
```

---

#### 3.2 `ReadLocker` / `WriteLocker` - Reader-writer lock guards
```cpp
class ReadLocker {
  ReadersWriterMutex& m_lock;
  bool m_locked{};
  // ...
};
class WriteLocker {
  ReadersWriterMutex& m_lock;
  bool m_locked{};
  // ...
};
```

**Standard Library Equivalent**:
- `std::shared_lock<std::shared_mutex>` - For readers (C++14)
- `std::unique_lock<std::shared_mutex>` - For writers (C++11)

**Recommendation**: ✅ **Replace with standard shared_lock**
```cpp
// Current
ReadLocker readLock(rwMutex);
WriteLocker writeLock(rwMutex);

// Standard library
std::shared_lock<std::shared_mutex> readLock(sharedMutex);
std::unique_lock<std::shared_mutex> writeLock(sharedMutex);
```

---

### 4. Thread Wrappers ⚠️ MIXED

#### 4.1 `Thread` - Abstract thread base class
**Status**: ⚠️ **Domain-specific** - Provides run() virtual method pattern. Keep as-is or migrate to callbacks.

#### 4.2 `ThreadFunction<T>` - Function wrapper with exception forwarding
**Current Implementation**: Wraps `std::thread` with exception handling
```cpp
ThreadFunction<void> task([]() { /* work */ }, "TaskName");
task.finish();  // blocks and rethrows exceptions
```

**Standard Library Equivalent**: `std::jthread` (C++20)
- Automatic joining on destruction
- Built-in stop token support
- Exception handling via `std::future` / `std::promise`

**Recommendation**: ⚠️ **Consider `std::jthread` + `std::future<T>` for exception propagation**

However, the current `ThreadFunction` provides:
- Automatic exception forwarding
- Named threads for debugging
- Return value storage

**Decision**: Keep for now due to specific API requirements, but could be modernized.

---

### 5. Container Wrappers ⚠️ MIXED

#### 5.1 `List<T>`, `Set<T>`, `Map<K,V>` - Utility mixins
**Status**: ⚠️ **Convenience wrappers** - Add utility methods like `first()`, `last()`, `filtered()`, `sorted()`, `transformed()`

**Modern Alternative**: C++20 Ranges
```cpp
// Current
List<int> numbers = {1, 2, 3, 4, 5};
auto evens = numbers.filtered([](int x) { return x % 2 == 0; });

// C++20 Ranges
std::vector<int> numbers = {1, 2, 3, 4, 5};
auto evens = numbers | std::views::filter([](int x) { return x % 2 == 0; });
```

**Recommendation**: ⚠️ **Evaluate migration to ranges** - Large refactor, medium priority

---

#### 5.2 `SmallVector<T>`, `StaticVector<T>` - Stack-allocated vectors
**Status**: ✅ **Keep** - Performance optimization (Small Buffer Optimization)
- Not in standard library
- Domain-specific optimization for game performance

---

#### 5.3 `FlatHashMap`, `FlatHashSet`, `FlatHashTable` - Custom hash tables
**Status**: ⚠️ **Performance-critical** - Likely faster than `std::unordered_map`
- Similar to Abseil's `flat_hash_map`
- **Recommendation**: Keep unless benchmarks show `std::unordered_map` is comparable

---

### 6. Variant and Either ❌ REDUNDANT

#### 6.1 `Variant<...>` - Custom variant implementation
**Current**: `source/core/StarVariant.hpp`

**Standard Library Equivalent**: `std::variant` (C++17)

**Recommendation**: ✅ **Replace with `std::variant`** - Direct replacement available

---

#### 6.2 `Either<L,R>` - Sum type for two alternatives
**Current**: `source/core/StarEither.hpp`

**Standard Library Equivalent**: `std::expected<T,E>` (C++23)

**Recommendation**: ✅ **Replace with `std::expected`** when compiler support is stable
- C++23 feature, check compiler version
- Provides better error handling semantics
- Fallback: Keep Either or use `std::variant<T, E>` temporarily

---

### 7. Atomic Shared Pointer ⚠️ PARTIALLY REDUNDANT

#### `AtomicSharedPtr<T>` - Thread-safe shared pointer with RCU
**Current**: Uses `SpinLock` + `std::shared_ptr`

**Standard Library Equivalent**: `std::atomic<std::shared_ptr<T>>` (C++20)

**Recommendation**: ⚠️ **Evaluate std::atomic<shared_ptr>** when compiler support is mature
- C++20 feature with spotty support
- Current implementation may be more portable
- Test performance before replacing

---

## Summary of Recommendations

### Immediate Actions (High Value, Low Risk)

| Wrapper | Standard Replacement | Occurrences | Effort | Risk |
|---------|---------------------|-------------|--------|------|
| `Maybe<T>` | `std::optional<T>` | ~440 | High | Low |
| `MLocker<T>` | `std::lock_guard` / `std::unique_lock` | ~531 | High | Low |
| `ReadLocker` / `WriteLocker` | `std::shared_lock` / `std::unique_lock` | ~118 | Medium | Low |
| `Mutex` / `RecursiveMutex` | Direct `std::mutex` / `std::recursive_mutex` | Many | Medium | Low |
| `ConditionVariable` | Direct `std::condition_variable` | Few | Low | Low |
| `ReadersWriterMutex` | Direct `std::shared_mutex` | Many | Medium | Low |

### Future Considerations (Evaluate)

| Wrapper | Standard Replacement | Priority | Notes |
|---------|---------------------|----------|-------|
| `Variant<...>` | `std::variant` | Medium | C++17 feature, should work |
| `Either<L,R>` | `std::expected<T,E>` | Low | C++23, check compiler support |
| `List/Set/Map` mixins | C++20 Ranges | Low | Large refactor, convenience trade-off |
| `ThreadFunction<T>` | `std::jthread` + `std::future` | Low | API redesign needed |
| `AtomicSharedPtr<T>` | `std::atomic<shared_ptr>` | Low | C++20, verify support |

### Keep As-Is (Domain-Specific or Performance-Critical)

- `SmallVector<T>` / `StaticVector<T>` - Stack optimization
- `FlatHashMap` / `FlatHashSet` - Performance-critical
- `RefPtr<T>` - Intrusive reference counting (specific use case)
- `SpinLock` - Performance-critical (though std has spinlock in some implementations)
- Math types (`Vector`, `Array`, `Matrix3`, etc.) - Domain-specific
- `BTree`, `BiMap`, `IdMap`, `LruCache`, etc. - Specialized data structures

---

## Implementation Strategy

### Phase 1: Low-Risk Standardization (Minimal Code Changes)
1. **Alias Maybe to std::optional** (if possible)
   - Create type alias: `template<typename T> using Maybe = std::optional<T>;`
   - Add custom methods as free functions
   - This allows gradual migration

2. **Remove unique_ptr wrappers from mutex types**
   - Simple one-line changes
   - No API changes needed

### Phase 2: RAII Lock Guard Replacement
1. **Replace MutexLocker with std::lock_guard/unique_lock**
   - High occurrence count (~531 uses)
   - Mostly mechanical replacement
   - May need `std::unique_lock` where manual control is needed

2. **Replace ReadLocker/WriteLocker with std::shared_lock**
   - Medium occurrence count (~118 uses)
   - Straightforward replacement

### Phase 3: Advanced Replacements
1. **Replace Variant with std::variant**
2. **Evaluate Either vs std::expected**
3. **Consider ranges-based refactoring for containers**

---

## Code Examples

### Maybe → std::optional

```cpp
// Before
Maybe<std::string> getName() {
  if (condition)
    return "Alice";
  return {};
}

auto name = getName();
if (name.isValid()) {
  std::cout << name.value() << std::endl;
}

// After
std::optional<std::string> getName() {
  if (condition)
    return "Alice";
  return std::nullopt;
}

auto name = getName();
if (name.has_value()) {  // or just: if (name)
  std::cout << *name << std::endl;
}
```

### MutexLocker → std::lock_guard

```cpp
// Before
void criticalSection() {
  MutexLocker lock(m_mutex);
  // critical section
}

// After
void criticalSection() {
  std::lock_guard<std::mutex> lock(m_mutex);
  // critical section
}

// Or if manual control needed:
void complexSection() {
  std::unique_lock<std::mutex> lock(m_mutex);
  // work...
  lock.unlock();
  // non-critical work
  lock.lock();
  // more critical work
}
```

### ReadLocker → std::shared_lock

```cpp
// Before
void readData() {
  ReadLocker lock(m_rwMutex);
  // read data
}

void writeData() {
  WriteLocker lock(m_rwMutex);
  // write data
}

// After
void readData() {
  std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
  // read data
}

void writeData() {
  std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
  // write data
}
```

---

## Migration Risks

### Low Risk
- Direct type replacements (mutex wrappers, condition variables)
- RAII lock guard replacements (std::lock_guard is equivalent)

### Medium Risk
- Maybe → std::optional (need to update method calls: isValid() → has_value())
- Variant → std::variant (API differences in access patterns)

### High Risk
- ThreadFunction changes (API redesign)
- Container mixin replacements with ranges (large refactor)

---

## Compiler Support Requirements

| Feature | C++ Version | GCC | Clang | MSVC |
|---------|-------------|-----|-------|------|
| `std::optional` | C++17 | ✅ 7+ | ✅ 4+ | ✅ 19.10+ |
| `std::variant` | C++17 | ✅ 7+ | ✅ 4+ | ✅ 19.10+ |
| `std::shared_mutex` | C++17 | ✅ 6+ | ✅ 3.9+ | ✅ 19.00+ |
| `std::shared_lock` | C++14 | ✅ 5+ | ✅ 3.4+ | ✅ 19.00+ |
| `std::scoped_lock` | C++17 | ✅ 7+ | ✅ 5+ | ✅ 19.11+ |
| `std::jthread` | C++20 | ✅ 10+ | ✅ 13+ | ✅ 19.28+ |
| `std::expected` | C++23 | ✅ 12+ | ✅ 16+ | ✅ 19.33+ |
| `std::atomic<shared_ptr>` | C++20 | ✅ 10+ | ✅ 15+ | ✅ 19.28+ |

**Current Project**: C++26 mode, so all C++17/20/23 features should be available.

---

## Conclusion

OpenStarbound has significant opportunities for modernization by removing redundant wrappers that duplicate standard library functionality. The highest-value changes are:

1. **Replacing `Maybe<T>` with `std::optional<T>`** - Eliminates wrapper overhead
2. **Replacing custom lock guards with standard RAII types** - Reduces maintenance burden
3. **Simplifying mutex wrappers** - Removes unnecessary indirection

These changes will:
- ✅ Reduce code maintenance burden
- ✅ Improve code clarity and portability
- ✅ Leverage optimized standard library implementations
- ✅ Make the codebase more accessible to new contributors familiar with standard C++

**Estimated Total Impact**: 
- ~1000+ lines of wrapper code could be eliminated
- ~1000+ call sites could be simplified
- Maintenance effort reduced by removing custom implementations
