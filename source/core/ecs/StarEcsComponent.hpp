#pragma once

#include "StarEcsTypes.hpp"
#include "StarList.hpp"

#include <vector>
#include <memory>
#include <functional>
#include <limits>

namespace Star {
namespace ECS {

// Forward declarations
class World;

// Base class for type-erased component storage
class IComponentArray {
public:
  virtual ~IComponentArray() = default;
  virtual void entityDestroyed(Entity entity) = 0;
  virtual bool has(Entity entity) const = 0;
  virtual void* getRaw(Entity entity) = 0;
  virtual void const* getRaw(Entity entity) const = 0;
};

// Sparse set implementation for efficient component storage
// Provides O(1) lookup, insertion, and removal
template<typename T>
class ComponentArray : public IComponentArray {
public:
  ComponentArray() = default;
  
  // Insert a component for an entity
  void insert(Entity entity, T component) {
    auto version = unpackEntity(entity);
    uint32_t index = version.index;
    
    // Grow sparse array if needed
    if (index >= m_sparse.size()) {
      m_sparse.resize(index + 1, InvalidIndex);
    }
    
    // Check if entity already has this component
    if (m_sparse[index] != InvalidIndex) {
      // Update existing component
      m_dense[m_sparse[index]].second = std::move(component);
      return;
    }
    
    // Add new component
    m_sparse[index] = static_cast<uint32_t>(m_dense.size());
    m_dense.push_back({entity, std::move(component)});
  }
  
  // Remove a component from an entity
  void remove(Entity entity) {
    auto version = unpackEntity(entity);
    uint32_t index = version.index;
    
    if (index >= m_sparse.size() || m_sparse[index] == InvalidIndex) {
      return; // Entity doesn't have this component
    }
    
    uint32_t denseIndex = m_sparse[index];
    
    // Swap with last element to maintain dense packing
    if (denseIndex < m_dense.size() - 1) {
      Entity lastEntity = m_dense.back().first;
      uint32_t lastIndex = unpackEntity(lastEntity).index;
      
      m_dense[denseIndex] = std::move(m_dense.back());
      m_sparse[lastIndex] = denseIndex;
    }
    
    m_dense.pop_back();
    m_sparse[index] = InvalidIndex;
  }
  
  // Get component for entity (returns nullptr if not found)
  T* get(Entity entity) {
    auto version = unpackEntity(entity);
    uint32_t index = version.index;
    
    if (index >= m_sparse.size() || m_sparse[index] == InvalidIndex) {
      return nullptr;
    }
    
    return &m_dense[m_sparse[index]].second;
  }
  
  T const* get(Entity entity) const {
    auto version = unpackEntity(entity);
    uint32_t index = version.index;
    
    if (index >= m_sparse.size() || m_sparse[index] == InvalidIndex) {
      return nullptr;
    }
    
    return &m_dense[m_sparse[index]].second;
  }
  
  // Check if entity has this component
  bool has(Entity entity) const override {
    auto version = unpackEntity(entity);
    uint32_t index = version.index;
    return index < m_sparse.size() && m_sparse[index] != InvalidIndex;
  }
  
  // IComponentArray interface
  void entityDestroyed(Entity entity) override {
    remove(entity);
  }
  
  void* getRaw(Entity entity) override {
    return get(entity);
  }
  
  void const* getRaw(Entity entity) const override {
    return get(entity);
  }
  
  // Get total number of components
  size_t size() const {
    return m_dense.size();
  }
  
  // Iteration support
  class Iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<Entity, T&>;
    using difference_type = std::ptrdiff_t;
    
    Iterator(typename std::vector<std::pair<Entity, T>>::iterator it)
      : m_it(it) {}
    
    std::pair<Entity, T&> operator*() {
      return {m_it->first, m_it->second};
    }
    
    Iterator& operator++() {
      ++m_it;
      return *this;
    }
    
    bool operator!=(Iterator const& other) const {
      return m_it != other.m_it;
    }
    
    bool operator==(Iterator const& other) const {
      return m_it == other.m_it;
    }
    
  private:
    typename std::vector<std::pair<Entity, T>>::iterator m_it;
  };
  
  class ConstIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<Entity, T const&>;
    using difference_type = std::ptrdiff_t;
    
    ConstIterator(typename std::vector<std::pair<Entity, T>>::const_iterator it)
      : m_it(it) {}
    
    std::pair<Entity, T const&> operator*() const {
      return {m_it->first, m_it->second};
    }
    
    ConstIterator& operator++() {
      ++m_it;
      return *this;
    }
    
    bool operator!=(ConstIterator const& other) const {
      return m_it != other.m_it;
    }
    
    bool operator==(ConstIterator const& other) const {
      return m_it == other.m_it;
    }
    
  private:
    typename std::vector<std::pair<Entity, T>>::const_iterator m_it;
  };
  
  Iterator begin() { return Iterator(m_dense.begin()); }
  Iterator end() { return Iterator(m_dense.end()); }
  ConstIterator begin() const { return ConstIterator(m_dense.begin()); }
  ConstIterator end() const { return ConstIterator(m_dense.end()); }
  
  // Get all entities with this component
  List<Entity> entities() const {
    List<Entity> result;
    result.reserve(m_dense.size());
    for (auto const& pair : m_dense) {
      result.append(pair.first);
    }
    return result;
  }

private:
  // InvalidIndex is set to max uint32_t value to represent an unallocated slot
  // in the sparse array. This allows using 0 as a valid index while ensuring
  // the sentinel value is unlikely to conflict with actual indices.
  static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();
  
  // Sparse array: maps entity index to dense array index
  std::vector<uint32_t> m_sparse;
  
  // Dense array: stores (entity, component) pairs
  std::vector<std::pair<Entity, T>> m_dense;
};

} // namespace ECS
} // namespace Star
