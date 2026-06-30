#pragma once

#include "StarPaneManager.hpp"

namespace Star {

template <typename KeyT>
class RegisteredPaneManager : public PaneManager {
public:
  using Key = KeyT;
  using PaneManager::PaneManager;

  void registerPane(KeyT paneId, PaneLayer paneLayer, UniquePtr<Pane> pane, DismissCallback onDismiss = {});
  [[nodiscard]] PanePtr deregisterPane(KeyT const& paneId);
  void deregisterAllPanes();

  template <typename T = Pane>
  [[nodiscard]] observer_ptr<T> registeredPane(KeyT const& paneId) const;
  template <typename T = Pane>
  [[nodiscard]] observer_ptr<T> maybeRegisteredPane(KeyT const& paneId) const;

  [[nodiscard]] bool displayRegisteredPane(KeyT const& paneId);
  [[nodiscard]] bool registeredPaneIsDisplayed(KeyT const& paneId) const;

  [[nodiscard]] bool dismissRegisteredPane(KeyT const& paneId);

  [[nodiscard]] bool toggleRegisteredPane(KeyT const& paneId);

private:
  struct PaneInfo {
    PaneLayer layer;
    SharedPtr<Pane> pane;
    DismissCallback dismissCallback;
  };

  [[nodiscard]] PaneInfo const& getRegisteredPaneInfo(KeyT const& paneId) const;

  HashMap<KeyT, PaneInfo> m_registeredPanes;
};

template <typename KeyT>
template <typename T>
observer_ptr<T> RegisteredPaneManager<KeyT>::registeredPane(KeyT const& paneId) const {
  if (auto v = m_registeredPanes.ptr(paneId))
    return observer_ptr<T>(dynamic_cast<T*>(v->pane.get()));
  throw GuiException(strf("No pane named '{}' found in RegisteredPaneManager", outputAny(paneId)));
}

template <typename KeyT>
template <typename T>
observer_ptr<T> RegisteredPaneManager<KeyT>::maybeRegisteredPane(KeyT const& paneId) const {
  if (auto v = m_registeredPanes.ptr(paneId))
    return observer_ptr<T>(dynamic_cast<T*>(v->pane.get()));
  return {};
}

template <typename KeyT>
void RegisteredPaneManager<KeyT>::registerPane(
    KeyT paneId, PaneLayer paneLayer, UniquePtr<Pane> pane, DismissCallback onDismiss) {
  auto sharedPane = SharedPtr<Pane>(std::move(pane));
  auto [paneIt, inserted] = m_registeredPanes.insert(std::move(paneId),
      {std::move(paneLayer), std::move(sharedPane), std::move(onDismiss)});
  if (!inserted)
    throw GuiException(
        strf("Registered pane with name '{}' registered a second time in RegisteredPaneManager::registerPane",
            outputAny(paneId)));
}

template <typename KeyT>
PanePtr RegisteredPaneManager<KeyT>::deregisterPane(KeyT const& paneId) {
  if (auto v = m_registeredPanes.maybeTake(paneId)) {
    if (isDisplayed(observer_ptr<Pane>(v->pane.get())))
      dismissPane(observer_ptr<Pane>(v->pane.get()));
    return std::move(v->pane);
  }
  throw GuiException(strf("No pane named '{}' found in RegisteredPaneManager::deregisterPane", outputAny(paneId)));
}

template <typename KeyT>
void RegisteredPaneManager<KeyT>::deregisterAllPanes() {
  for (auto const& paneId : m_registeredPanes.keys())
    deregisterPane(paneId);
}

template <typename KeyT>
bool RegisteredPaneManager<KeyT>::displayRegisteredPane(KeyT const& paneId) {
  auto const& paneInfo = getRegisteredPaneInfo(paneId);
  if (!isDisplayed(observer_ptr<Pane>(paneInfo.pane.get()))) {
    displayPane(paneInfo.layer, *paneInfo.pane, paneInfo.dismissCallback);
    return true;
  }
  return false;
}

template <typename KeyT>
bool RegisteredPaneManager<KeyT>::registeredPaneIsDisplayed(KeyT const& paneId) const {
  auto const& paneInfo = getRegisteredPaneInfo(paneId);
  return isDisplayed(observer_ptr<Pane>(paneInfo.pane.get()));
}

template <typename KeyT>
bool RegisteredPaneManager<KeyT>::dismissRegisteredPane(KeyT const& paneId) {
  auto const& paneInfo = getRegisteredPaneInfo(paneId);
  if (isDisplayed(observer_ptr<Pane>(paneInfo.pane.get()))) {
    dismissPane(observer_ptr<Pane>(paneInfo.pane.get()));
    return true;
  }
  return false;
}

template <typename KeyT>
bool RegisteredPaneManager<KeyT>::toggleRegisteredPane(KeyT const& paneId) {
  if (registeredPaneIsDisplayed(paneId)) {
    dismissRegisteredPane(paneId);
    return false;
  } else {
    displayRegisteredPane(paneId);
    return true;
  }
}

template <typename KeyT>
typename RegisteredPaneManager<KeyT>::PaneInfo const& RegisteredPaneManager<KeyT>::getRegisteredPaneInfo(
    KeyT const& paneId) const {
  if (auto paneInfo = m_registeredPanes.ptr(paneId))
    return *paneInfo;
  throw GuiException(strf("No registered pane with name '{}' found in  RegisteredPaneManager", outputAny(paneId)));
}

}

