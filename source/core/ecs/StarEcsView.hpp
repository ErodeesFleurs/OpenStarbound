#pragma once

#include "StarEcsTypes.hpp"
#include "StarEcsComponent.hpp"

#include <tuple>

namespace Star {
namespace ECS {

// Forward declaration
class World;

// View allows iteration over entities with specific components
template<typename... Components>
class View {
public:
  View(World* world);
  
  // Iterator for the view
  class Iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::tuple<Entity, Components&...>;
    using difference_type = std::ptrdiff_t;
    
    Iterator(World* world, List<Entity> const* entities, size_t index)
      : m_world(world), m_entities(entities), m_index(index) {
      // Find first valid entity
      while (m_index < m_entities->size() && !isValid()) {
        ++m_index;
      }
    }
    
    std::tuple<Entity, Components&...> operator*();
    
    Iterator& operator++() {
      do {
        ++m_index;
      } while (m_index < m_entities->size() && !isValid());
      return *this;
    }
    
    bool operator!=(Iterator const& other) const {
      return m_index != other.m_index;
    }
    
    bool operator==(Iterator const& other) const {
      return m_index == other.m_index;
    }
    
  private:
    bool isValid() const;
    
    World* m_world;
    List<Entity> const* m_entities;
    size_t m_index;
  };
  
  Iterator begin();
  Iterator end();
  
  // Count entities matching this view
  size_t count() const;
  
  // Check if view is empty
  bool empty() const { return count() == 0; }
  
  // Execute a function for each matching entity
  template<typename Func>
  void each(Func&& func);

private:
  World* m_world;
  List<Entity> m_entities;
};

// Single component view - simpler and more efficient
template<typename T>
class SingleView {
public:
  SingleView(ComponentArray<T>* array) : m_array(array) {}
  
  auto begin() { return m_array->begin(); }
  auto end() { return m_array->end(); }
  
  size_t size() const { return m_array->size(); }
  bool empty() const { return m_array->size() == 0; }
  
private:
  ComponentArray<T>* m_array;
};

} // namespace ECS
} // namespace Star
