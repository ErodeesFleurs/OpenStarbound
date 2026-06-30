#pragma once

#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "StarThread.hpp"
#include "StarLogging.hpp"
#include "StarTime.hpp"

namespace Star {

class ServiceContainer {
public:
  ServiceContainer() = default;

  // Register a pre-built service instance (for testing or eager-loading).
  // Takes ownership via shared_ptr.
  template <typename Interface>
  void registerService(shared_ptr<Interface> service) {
    MutexLocker locker(m_mutex);
    m_services[typeid(Interface)] = service;
  }

  // Get or lazily-construct a service. The factory is only called once.
  template <typename Interface, typename Factory>
  shared_ptr<Interface> get(Factory&& factory) {
    std::type_index idx = typeid(Interface);
    {
      MutexLocker locker(m_mutex);
      auto it = m_services.find(idx);
      if (it != m_services.end()) {
        auto ptr = std::static_pointer_cast<Interface>(it->second);
        if (ptr)
          return ptr;
      }
    }
    auto service = std::static_pointer_cast<void>(std::shared_ptr<Interface>(factory()));
    MutexLocker locker(m_mutex);
    auto& entry = m_services[idx];
    if (!entry)
      entry = service;
    return std::static_pointer_cast<Interface>(entry);
  }

  // Get a previously registered or constructed service.
  template <typename Interface>
  shared_ptr<Interface> get() {
    std::type_index idx = typeid(Interface);
    MutexLocker locker(m_mutex);
    auto it = m_services.find(idx);
    if (it != m_services.end())
      return std::static_pointer_cast<Interface>(it->second);
    return {};
  }

  // Release all services, calling destructors in reverse registration order
  // (approximation of dependency ordering).
  void reset() {
    MutexLocker locker(m_mutex);
    m_services.clear();
  }

private:
  Mutex m_mutex;
  std::unordered_map<std::type_index, shared_ptr<void>> m_services;
};

}

